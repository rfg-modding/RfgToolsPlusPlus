#include "common/Typedefs.h"
#include "Common/timing/Timer.h"
#include "Common/filesystem/Path.h"
#include "Common/filesystem/File.h"
#include "RfgTools++/formats/packfiles/Packfile3.h"
#include <iostream>
#include <filesystem>
#include <RfgTools++\formats\meshes\MeshDataBlock.h>
#include <BinaryTools\BinaryWriter.h>

int main()
{
    string packfilePath = "G:\\GOG\\Games\\Red Faction Guerrilla Re-Mars-tered\\data\\aaaaaa_terr01_l0.vpp_pc";
    Packfile3 packfile(packfilePath);
    packfile.ReadMetadata();
    packfile.ExtractSubfiles("G:\\GOG\\Games\\Red Faction Guerrilla Re-Mars-tered\\data\\out\\");

    return 0;

    /*Extract contents of all packfiles*/
    //auto files = std::filesystem::directory_iterator("G:\\_RFGR_Unpack\\data\\");
    //for (auto& file : files)
    //{
    //    string ext = Path::GetExtension(file);
    //    if (file.is_directory() || !(ext == ".vpp_pc" || ext == ".str2_pc"))
    //        continue;

    //    string packfilePath = file.path().string();
    //    string outputPath = "G:\\_RFGR_Unpack\\data\\unpack\\" + Path::GetFileName(file) + "\\";
    //    Path::CreatePath(outputPath);
    //    std::cout << "Extracting " << Path::GetFileName(file) << "...";
    //    Packfile3 packfile(packfilePath);
    //    packfile.ReadMetadata();
    //    packfile.ExtractSubfiles(outputPath);
    //    std::cout << " Done!\n";
    //}

    /*Extract contents of all str2_pc files from previous step*/
    auto subfiles = std::filesystem::recursive_directory_iterator("G:\\_RFGR_Unpack\\data\\unpack\\");
    for (auto& file : subfiles)
    {
        string ext = Path::GetExtension(file);
        if (file.is_directory() || ext != ".str2_pc")
            continue;

        string packfilePath = file.path().string();
        string outputPath = Path::GetParentDirectory(file) + "\\subfiles\\" + Path::GetFileName(file) + "\\";
        Path::CreatePath(outputPath);
        std::cout << "Extracting " << Path::GetFileName(file) << "...";
        Packfile3 packfile(packfilePath);
        packfile.ReadMetadata();
        packfile.ExtractSubfiles(outputPath);
        std::cout << " Done!\n";
    }

    return 0;








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

    //string cpuFilePath = "G:/RFG Unpack/data/Unpack/terr01_l1.vpp_pc/Unpack/0h_c2337.str2_pc/0202tower_guard.cchk_pc";
    //string gpuFilePath = "G:/RFG Unpack/data/Unpack/terr01_l1.vpp_pc/Unpack/0h_c2337.str2_pc/0202tower_guard.gchk_pc";
    string cpuFilePath = "G:/RFG Unpack/data/Unpack/terr01_l0.vpp_pc/Unpack/0h_c2016.str2_pc/0101landing_pad_small_a.cchk_pc";
    string gpuFilePath = "G:/RFG Unpack/data/Unpack/terr01_l0.vpp_pc/Unpack/0h_c2016.str2_pc/0101landing_pad_small_a.gchk_pc";

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

    std::cout << "Position at start of material block: " << cpuFile.Position() << "\n";
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

    //TODO: IMPORTANT
    //TODO: READ GENERAL OBJECTS HERE IF THE CHUNK HAS ANY. NEED TO READ VALUE FROM HEADER FIRST, CURRENTLY NOT

    u32 numDestroyables = cpuFile.ReadUint32();
    //Seemingly random data skip that the game does. Maybe empty space in case it's needed eventually
    cpuFile.Skip((numDestroyables * 8) + 4);
    cpuFile.Align(16);

    std::cout << "Position at expected start of first destroyable: " << cpuFile.Position() << "\n";

    enum material_shape_type
    {
        INVALID_MATERIAL_EVENT_SHAPE_TYPE = 0xFFFFFFFF,
        MATERIAL_EVENT_SHAPE_SOLID = 0x0,
        MATERIAL_EVENT_SHAPE_SHEET = 0x1,
        MATERIAL_EVENT_SHAPE_POLE = 0x2,
        MATERIAL_EVENT_SHAPE_FENCE = 0x3,
        MATERIAL_EVENT_SHAPE_CATWALK = 0x4,
        NUM_MATERIAL_EVENT_SHAPE_TYPES = 0x5,
    };
    struct rl_color_float
    {
        float red;
        float green;
        float blue;
        float alpha;
    };
    struct base_event_candidate
    {
        void* vfptr;//base_event_candidateVtbl* vfptr;
        float event_size[3];
    };
    struct __declspec(align(4)) gust_effect_info
    {
        unsigned int effect;
        float fade_time;
        float percent_chance;
        bool near_far;
    };
    struct rfg_effect_material
    {
        char name[32];
        material_shape_type default_shape_type;
        rl_color_float effect_tint;
        bool effect_tint_enabled;
        base_event_candidate* effect_candidate[13];
        int blast_decal;
        unsigned int trail_effects[2];
        int num_gust_effects;
        gust_effect_info gust_effects[10];
        bool driving_do_tracks;
        unsigned int driving_front_effect;
        unsigned int driving_rear_effect;
    };
    struct rfg_physical_material
    {
        char name[32];
        unsigned int name_checksum;
        char suffix[3];
        unsigned int suffix_checksum;
        u32 base_props_name_ptr; //char* base_props_name;
        float density;
        float brittleness;
        float restitution;
        float friction;
        unsigned int link_strength;
        u32 effect_material_ptr; //rfg_effect_material* effect_material;
        unsigned int flags;
    };
    struct vector
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct matrix
    {
        vector rvec;
        vector uvec;
        vector fvec;
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
    struct link_base
    {
        i32 yield_max;
        f32 area;
        i16 obj[2];
        u8 flags;
        u8 padding[3];
    };
    struct dlod_base
    {
        u32 name_hash;
        vector pos;
        matrix orient;
        u16 render_subpiece;
        u16 first_piece;
        u8 max_pieces;
        u8 padding[3];
    };
    struct fp_aabb
    {
        __int16 min_x;
        __int16 min_y;
        __int16 min_z;
        __int16 max_x;
        __int16 max_y;
        __int16 max_z;
    };
    struct rfg_rbb_node
    {
        void Read(BinaryReader& reader)
        {
            reader.ReadToMemory(this, sizeof(rfg_rbb_node));
        }
        int num_objects;
        fp_aabb aabb;
        u32 node_data_offset; //et_ptr_offset<unsigned char, 0> node_data;
    };
    struct destroyable_instance_data
    {
        unsigned int objects_offset;
        unsigned int links_offset;
        unsigned int dlods_offset;
        unsigned int data_size;
        u32 buffer_offset; //et_ptr_offset<unsigned char, 0> buffer;
    };

    struct destroyable
    {
        destroyable_base base;
        std::vector<subpiece_base> subpieces;
        std::vector<subpiece_base_data> subpieces_data;
        std::vector<link_base> links;
        std::vector<dlod_base> dlods;
        std::vector<rfg_rbb_node> rbb_nodes;
        destroyable_instance_data instance_data;
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

        //for (u32 j = 0; j < destroyable.base.num_objects; j++)
        //{
        //    std::cout << destroyable.subpieces_data[j].render_subpiece << "\n";
        //}
        
        //Todo: Figure out what this data is meant to be. Game has some physical material code here. Maybe link material
        auto b = cpuFile.Position();
        for (auto& subpiece : destroyable.subpieces)
            cpuFile.Skip(subpiece.num_links * 2);

        cpuFile.Align(4);
        auto d = cpuFile.Position();

        //Read links
        for (u32 j = 0; j < destroyable.base.num_links; j++)
        {
            auto& link = destroyable.links.emplace_back();
            cpuFile.ReadToMemory(&link, sizeof(link_base));
        }
        auto e = cpuFile.Position();
        cpuFile.Align(4);
        auto f = cpuFile.Position();


        //Read dlods
        for (u32 j = 0; j < destroyable.base.num_dlods; j++)
        {
            auto& dlod = destroyable.dlods.emplace_back();
            cpuFile.ReadToMemory(&dlod, sizeof(dlod_base));
        }
        auto g = cpuFile.Position();
        cpuFile.Align(4);
        auto h = cpuFile.Position();


        //Read rbb nodes
        destroyable.rbb_nodes.push_back(rfg_rbb_node());
        rfg_rbb_node* curNode = &destroyable.rbb_nodes.back();
        curNode->Read(cpuFile);
        u32 num_objects = curNode->num_objects;
        while (num_objects > 0)
        {
            //Todo: Implement this loop. Not yet added since the test file doesn't use it
            throw std::exception("Support for multiple rbb nodes not yet added!");
        }
        auto k = cpuFile.Position();
        cpuFile.Align(4);
        auto l = cpuFile.Position();

        //Read instance data. Seems to be junk in the file and set at runtime. Unless it's offset from the packfile start or something painful like that
        cpuFile.ReadToMemory(&destroyable.instance_data, sizeof(destroyable_instance_data));
        static_assert(sizeof(destroyable_instance_data) == 20, "destroyable_instance_data size check failed!");

        //Todo: There may be some data we have to read/skip here. The code at this point was confusing. Something something aabb instances

        auto m = cpuFile.Position();
        cpuFile.Align(16);
        auto n = cpuFile.Position();

        u32 maybeTransformBufferSize = cpuFile.ReadUint32();
        cpuFile.Skip(destroyable.subpieces.size() * 36);
        auto o = cpuFile.Position();
        cpuFile.Align(16);
        auto p = cpuFile.Position();
        

        auto c = 2;
    }

    //Read vertex and index data


    //Output submeshes to an obj file
    for (u32 i = 0; i < meshData.NumSubmeshes; i++)
    {
        SubmeshData& submesh = meshData.Submeshes[i];

    }

    BinaryWriter writer("G:\\Rfg Mesh Hack extraction folder\\0101landing_pad_small_a__object_positions.bin");
    for (auto& destroyable : destroyables)
    {
        for (auto& subpiece : destroyable.subpieces)
        {
            writer.WriteFloat(subpiece.pos.x);
            writer.WriteFloat(subpiece.pos.y);
            writer.WriteFloat(subpiece.pos.z);
        }
    }

    auto d = 2;

    return 0;
}