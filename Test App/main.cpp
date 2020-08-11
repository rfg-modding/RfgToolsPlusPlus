#include "common/Typedefs.h"
#include "Common/timing/Timer.h"
#include "Common/filesystem/Path.h"
#include "Common/filesystem/File.h"
#include "RfgTools++/formats/packfiles/Packfile3.h"
#include <iostream>
#include <RfgTools++\formats\meshes\MeshDataBlock.h>

int main()
{
    //string inputPath = "G:/RFG Unpack/data/misc.vpp_pc";
    //string outputPath = "G:/RFG Unpack 2/CppToolOutput/Unpack/";

    //std::cout << "inputPath: \"" << inputPath << "\"\n";
    //std::cout << "outputPath: \"" << outputPath << "\"\n";

    //u32 numRuns = 1; //Should use higher number for more precise results. Need speed for testing though
    //std::vector<u64> times;
    //times.reserve(numRuns);

    //printf("Extracting packfile %d times...\n", numRuns);
    //Timer timer(true);

    //for (u32 i = 0; i < numRuns; i++)
    //{
    //    Packfile3 packfile(inputPath);
    //    packfile.ReadMetadata();
    //    packfile.ExtractSubfiles(outputPath + Path::GetFileNameNoExtension(inputPath) + "/");
    //    //auto weaponsXtbl = packfile.ExtractSingleFile("weapons.xtbl");
    //    //File::WriteToFile("G:\\RFG Unpack\\boop.xml", weaponsXtbl.value());
    //    //delete[] weaponsXtbl.value().data();
    //    times.push_back(timer.ElapsedMicroseconds());
    //    timer.Reset();
    //}

    //f32 sum = 0.0f;
    //for (auto& val : times)
    //    sum += (f32)val;

    //std::cout << "Elapsed time (average over " << numRuns << " runs): " << sum / (f32)numRuns << "us | " << sum / (f32)numRuns / 1000.0f << "ms | " << sum / (f32)numRuns / 1000000.0f << "s\n";

    struct ChunkHeader
    {
        unsigned int signature = 0;
        unsigned int version = 0;
        unsigned int source_rfgchunkx_version = 0;
        unsigned int render_data_checksum = 0;
        unsigned int render_cpu_data_offset = 0;
        unsigned int render_cpu_data_size = 0;
        unsigned int collision_model_checksum = 0;
        unsigned int collision_model_data_offset = 0;
        unsigned int collision_model_data_size = 0;
        unsigned int destruction_checksum = 0;
        unsigned int destruction_offset = 0;
        unsigned int destruction_datasize = 0;
    };

    string cpuFilePath = "G:/RFG Unpack/data/Unpack/terr01_l1.vpp_pc/Unpack/0h_c2337.str2_pc/0202tower_guard.cchk_pc";
    string gpuFilePath = "G:/RFG Unpack/data/Unpack/terr01_l1.vpp_pc/Unpack/0h_c2337.str2_pc/0202tower_guard.gchk_pc";

    BinaryReader cpuFile(cpuFilePath);
    BinaryReader gpuFile(gpuFilePath);

    ChunkHeader header;
    cpuFile.ReadToMemory((void*)&header, sizeof(ChunkHeader));
    std::cout << "Position in cpu file after reading header: " << cpuFile.Position() << "\n";

    cpuFile.Skip(400); //Skip 400 unknown / possibly padding bytes
    u32 unkValue0 = cpuFile.ReadUint32();
    cpuFile.Skip(28);
    u32 unkValue1 = cpuFile.ReadUint32();
    cpuFile.Skip(32);
    u32 unkValue2 = cpuFile.ReadUint32();
    cpuFile.Skip(184);

    //Should be at the render data offset
    if (cpuFile.Position() != header.render_cpu_data_offset)
        throw std::exception("Error! Expected chunk render data offset does not match expected value!");

    std::cout << "Position at start of chunk render data block: " << cpuFile.Position() << "\n";
    
    //Read the mesh data block that should be at the start of the render data offset
    MeshDataBlock meshData;
    meshData.Read(cpuFile);

    std::cout << "Position at end of chunk render data MeshDataBlock: " << cpuFile.Position() << "\n";

    //Align to 16 bytes and read size of texture names block
    cpuFile.Align(16);
    auto a = cpuFile.Position();
    u32 textureNamesBlockSize = cpuFile.ReadUint32();

    //Read texture names block. These are double null terminated
    std::vector<string> textureNames;
    u64 textureNamesStartPos = cpuFile.Position();
    //Read strings until we reach the end of the string block
    while (cpuFile.Position() - textureNamesStartPos < textureNamesBlockSize)
    {
        auto pos0 = cpuFile.Position();
        //Read null terminated string
        textureNames.push_back(cpuFile.ReadNullTerminatedString());
        auto pos1 = cpuFile.Position();
        //Skip any additional null terminators. RFG likes to have random amounts of these
        while (cpuFile.Position() - textureNamesStartPos < textureNamesBlockSize)
        {
            auto pos2 = cpuFile.Position();
            if (cpuFile.PeekChar() == '\0')
                cpuFile.Skip(1);
            else
                break;
        }
    }
    cpuFile.Align(16);

    std::cout << "Position at end of texture names block: " << cpuFile.Position() << "\n";

    //Todo: Update this comment if I figure this trash out
    //Some kind of material data. Haven't found a generic material data structure used across mesh types yet so this may be tricky
    u32 materialOffset = cpuFile.ReadUint32();
    u32 numMaterials = cpuFile.ReadUint32();
    std::vector<u32> materialIds; //Todo: Figure out what these really are
    for (u32 i = 0; i < numMaterials; i++)
        materialIds.push_back(cpuFile.ReadUint32());

    //I dunno what this even does but the game seems to do something roughly analogous to this
    cpuFile.Skip(materialOffset); //Todo: Figure out what the hell is going on with this
    cpuFile.Skip(numMaterials * 8); //The game does this for some reason
    cpuFile.Align(16); //Todo: Make sure this reliably gets us to the next chunk of data



    struct rl_material_data
    {
        unsigned int shader_handle;
        unsigned int name_checksum;
        unsigned int mat_flags;
        unsigned __int16 num_textures;
        char num_constants;
        char max_constants;
        u32 textures_offset;
        u32 constant_name_checksum_offset;
        u32 constant_block_offset;
    };
    struct texture_desc
    {
        u32 name_offset;
        u32 name_checksum;
        u32 texture_index;
    };
    struct material_constant
    {
        float constants[4];
    };
    struct material_data_block
    {
        rl_material_data material_data;
        std::vector<texture_desc> texture_descs;
        std::vector<u32> constant_name_checksums;
        std::vector<material_constant> constants;

        void Read(BinaryReader& reader)
        {
            u64 startPos = reader.Position();
            u32 materialDataSize = reader.ReadUint32();

            reader.ReadToMemory(&material_data, sizeof(rl_material_data));
            for (u32 i = 0; i < material_data.num_textures; i++)
            {
                auto& desc = texture_descs.emplace_back();
                reader.ReadToMemory(&desc, sizeof(texture_desc));
            }
            for (u32 i = 0; i < material_data.num_constants; i++)
            {
                constant_name_checksums.push_back(reader.ReadUint32());
            }
            reader.Align(16);
            for (u32 i = 0; i < material_data.max_constants; i++)
            {
                auto& constantData = constants.emplace_back();
                reader.ReadToMemory(&constantData, sizeof(material_constant));
            }

            if (reader.Position() - startPos != materialDataSize)
                throw std::exception("Error! Failed to read material data block. Length of read data doesn't equal expected data size.");
        }
    };

    std::vector<material_data_block> materials;

    for (u32 i = 0; i < numMaterials; i++)
    {
        cpuFile.Align(16);
        auto& mat = materials.emplace_back();
        mat.Read(cpuFile);
    }

    std::cout << "Position at end of material data block 0: " << cpuFile.Position() << "\n";
    //Todo: Figure out if supposed to Align(64) or Skip(64) here. Game seems to Align(64) but Skip(64) is what gets us to the next data section in the test file
    //cpuFile.Align(64);
    cpuFile.Skip(64);
    std::cout << "Position at expected start of destruction data block: " << cpuFile.Position() << "\n";

    if (cpuFile.Position() != header.destruction_offset)
        throw std::exception("Error! Current location does not equal the destruction data offset as it is expected to at this point!");

    cpuFile.Align(128); //Todo: Just guessing at this point...

    std::cout << "Position at expected start of destroyables data block: " << cpuFile.Position() << "\n";

    u32 numDestroyables = cpuFile.ReadUint32();
    //Seemingly random data skip that the game does. Maybe empty space in case it's needed eventually
    cpuFile.Skip((numDestroyables * 8) + 4);
    cpuFile.Align(16);

    std::cout << "Position at expected start of first destroyable: " << cpuFile.Position() << "\n";

    struct vector
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct destroyable_base
    {
        u32 aabb_tree_offset; //rfg_rbb_node
        u32 base_objects_offset; //rfg_subpiece_base
        u32 base_extra_data_offset; //rfg_subpiece_base_extra_data
        int num_objects;
        u32 base_links_offset; //base_links
        int num_links;
        u32 base_dlods_offset; //base_dlods
        int num_dlods;
        u32 inst_data_offset; //rfg_destroyable_base_instance_data
        u32 transform_buffer_offset; //unsigned char
        float base_structural_mass;
    };
    struct subpiece_base
    {
        vector prel_bmin;
        vector prel_bmax;
        vector pos;
        vector center_of_mass;
        f32 mass;
        u32 dlod_key;
        u32 links_offset; //ushort
        u8 physical_material_index;
        u8 shape_type;
        u8 num_links;
        u8 flags;
    };
    struct subpiece_base_data
    {
        u32 shape_offset; //hkpShape
        u16 collision_model;
        u16 render_subpiece;
        u32 h;
    };
    struct destroyable
    {
        destroyable_base base;
        std::vector<subpiece_base> subpieces;
        std::vector<subpiece_base_data> subpieces_data;
    };
    std::vector<destroyable> destroyables;
    for (u32 i = 0; i < numDestroyables; i++)
    {
        //Create new destroyable instance
        auto& destroyable = destroyables.emplace_back();

        //Read base data and align to next piece of data
        cpuFile.ReadToMemory(&destroyable.base, sizeof(destroyable_base));
        cpuFile.Align(128);

        auto a = cpuFile.Position();
        
        //Todo: Could allocate a buffer and read all this data at once then access with a span
        //Read base object data
        for (u32 j = 0; j < destroyable.base.num_objects; j++)
        {
            auto& subpiece = destroyable.subpieces.emplace_back();
            cpuFile.ReadToMemory(&subpiece, sizeof(subpiece_base));
        }
        for (u32 j = 0; j < destroyable.base.num_objects; j++)
        {
            auto& subpiece_data = destroyable.subpieces_data.emplace_back();
            cpuFile.ReadToMemory(&subpiece_data, sizeof(subpiece_base_data));
        }



        auto b = cpuFile.Position();
        auto c = 2;
    }

    return 0;
}