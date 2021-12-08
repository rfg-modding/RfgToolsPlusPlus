#include "common/Typedefs.h"
#include "ChunkTypes.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <vector>

//tiny gltf and its dependencies
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

/*
This file contains an experimental RFG chunk file reader (cchk_pc, gchk_pc). It's only intended as a testing tool to reverse chunk files so the code is a mess.
Once the format is fully reversed this tool will be discarded and cchk reading/writing will be implemented properly into RfgTools++.
*/

template <class T>
std::vector<unsigned char> ToByteVector(std::vector<T>& source)
{
    //Create output vector
    size_t dataSize = source.size() * sizeof(T);
    std::vector<unsigned char> out;
    out.reserve(dataSize);

    //Copy source to output
    unsigned char* buffer = new unsigned char[dataSize];
    memcpy(buffer, source.data(), dataSize);
    for (u32 i = 0; i < dataSize; i++)
    {
        out.push_back(buffer[i]);
    }
    delete[] buffer;

    //Return output
    return out;
}

class ConsoleArgs
{
public:
    ConsoleArgs(int argc, char* argv[])
    {
        _programPath = argv[0];
        if (argc > 1)
        {
            for (u32 i = 1; i < argc; i++)
                _args.push_back(argv[i]);
        }
    }

    //Returns true if an option was passed. E.g. Option("Names") returns true if one of the options passed to the exe is --Names
    bool Option(const string& argName)
    {
        string argString = "--" + argName;
        auto find = std::ranges::find_if(_args, [argString](const string& arg) -> bool { return arg == argString; });
        return find != _args.end();
    }

    std::optional<string> GetArg(size_t index)
    {
        if (index >= _args.size())
            return {};
        else
            return _args[index];
    }

    string ProgramPath()
    {
        return _programPath;
    }

    size_t Count()
    {
        return _args.size();
    }

private:
    string _programPath;
    std::vector<std::string> _args = {};
};

int main(int argc, char* argv[])
{
    //Parse console arguments
    printf("count: %d, argv[0]: %s\n", argc, argv[0]);
    ConsoleArgs args(argc, argv);
    std::optional<string> cpuFilePath = args.GetArg(0);
    std::optional<string> gpuFilePath = cpuFilePath.has_value() ?
                                        Path::GetParentDirectory(cpuFilePath.value()) + Path::GetFileNameNoExtension(cpuFilePath.value()) + ".gchk_pc" :
                                        std::optional<string>();
    bool showTextures = args.Option("textures");
    bool debug = args.Option("debug");
    bool verbose = args.Option("verbose");

    //If file is packfile, extract it. Otherwise go onto normal cchk extractor
    //Stop gap measure until manual packfile extraction is added to Nanoforge. Necessary since existing console based packfile extractors have bugs
    if (cpuFilePath && (Path::GetExtension(cpuFilePath.value()) == ".vpp_pc" || Path::GetExtension(cpuFilePath.value()) == ".str2_pc"))
    {
        printf("Passed a packfile, attempting extract %s...\n", Path::GetExtension(cpuFilePath.value()).c_str());
        string outputPath = Path::GetParentDirectory(cpuFilePath.value()) + "Unpack/" + Path::GetFileNameNoExtension(cpuFilePath.value()) + "/";
        Path::CreatePath(outputPath);

        Packfile3 packfile(cpuFilePath.value());
        packfile.ReadMetadata();
        packfile.ExtractSubfiles(outputPath);

        //Unpack str2_pc files if passed --recursive
        if (args.Option("recursive"))
        {
            printf("Recursively extracting packfiles...");
            for (auto& entry : std::filesystem::directory_iterator(outputPath))
            {
                string ext = Path::GetExtension(entry.path());
                if (ext == ".str2_pc" || ext == ".vpp_pc")
                {
                    if (verbose)
                        printf("Extracting %s...\n", Path::GetFileName(entry.path().string()).c_str());

                    string containerOutputPath = outputPath + "Unpack/" + Path::GetFileNameNoExtension(entry.path().string()) + "/";
                    Path::CreatePath(containerOutputPath);

                    Packfile3 container(entry.path().string());
                    container.ReadMetadata();
                    container.ExtractSubfiles(containerOutputPath);
                }
            }
        }

        printf("Done!\n");
        return 0;
    }

    if (args.Count() < 1 || !cpuFilePath || !gpuFilePath)
    {
        printf("Error. Invalid input. Expected '%s input.cchk_pc input.gchk_pc --options'\n", args.ProgramPath().c_str());
        return 0;
    }
    if (!std::filesystem::exists(cpuFilePath.value()))
    {
        printf("Input file '%s' doesn't exist!", cpuFilePath.value().c_str());
        return 0;
    }
    if (!std::filesystem::exists(gpuFilePath.value()))
    {
        printf("Input file '%s' doesn't exist!", gpuFilePath.value().c_str());
        return 0;
    }

    BinaryReader cpuFile(cpuFilePath.value());
    BinaryReader gpuFile(gpuFilePath.value());

    ChunkHeader header;
    cpuFile.ReadToMemory((void*)&header, sizeof(ChunkHeader));
    if (debug)
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

    if (debug)
        std::cout << "Position at start of chunk render data block: " << cpuFile.Position() << "\n";

    //Read the mesh data block that should be at the start of the render data offset
    MeshDataBlock meshData;
    meshData.Read(cpuFile);

    if (debug)
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
        //Read null terminated string
        textureNames.push_back(cpuFile.ReadNullTerminatedString());

        //Skip any additional null terminators. RFG likes to have random amounts of these
        while (cpuFile.Position() - textureNamesStartPos < textureNamesBlockSize)
        {
            if (cpuFile.PeekChar() == '\0')
                cpuFile.Skip(1);
            else
                break;
        }
    }
    cpuFile.Align(16);

    if (showTextures)
    {
        printf("Texture names:\n");
        for (const string& name : textureNames)
            printf("\t%s\n", name.c_str());
    }

    if (debug)
        std::cout << "Position at end of texture names block: " << cpuFile.Position() << "\n";

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

    //std::cout << "Position at start of material block: " << cpuFile.Position() << "\n";
    //std::vector<material_data_block> materials;

    //for (u32 i = 0; i < numMaterials; i++)
    //{
    //    cpuFile.Align(16);
    //    auto& mat = materials.emplace_back();
    //    mat.Read(cpuFile);
    //}

    //std::cout << "Position at end of material data block 0: " << cpuFile.Position() << "\n";
    ////Todo: Figure out if supposed to Align(64) or Skip(64) here. Game seems to Align(64) but Skip(64) is what gets us to the next data section in the test file
    ////cpuFile.Align(64);
    //cpuFile.Skip(64);
    //std::cout << "Position at expected start of destruction data block: " << cpuFile.Position() << "\n";

    //cpuFile.Align(128); //Todo: Just guessing at this point...
    ////Todo: Re-enable check instead of skipping around
    //if (cpuFile.Position() != header.destruction_offset)
    //    throw std::exception("Error! Current location does not equal the destruction data offset as it is expected to at this point!");


    cpuFile.SeekBeg(header.destruction_offset);
    if (debug)
        std::cout << "Position at expected start of destroyables data block: " << cpuFile.Position() << "\n";

    //TODO: general_object instances might exist here in some files
    
    cpuFile.Align(128);

    u32 numDestroyables = cpuFile.ReadUint32();
    if (numDestroyables != 1)
    {
        printf("Error! '%s' has %d destroyables. Only files with 1 destroyable are supported at the moment!\n", Path::GetFileName(cpuFilePath.value()).c_str(), numDestroyables);
        return 0;
    }

    //Seemingly random data skip that the game does. Maybe empty space in case it's needed eventually
    cpuFile.Skip((numDestroyables * 8) + 4);
    cpuFile.Align(16);

    if (debug)
        std::cout << "Position at expected start of first destroyable: " << cpuFile.Position() << "\n";


    std::vector<destroyable> destroyables;
    for (u32 i = 0; i < numDestroyables; i++)
    {
        //Create new destroyable instance
        auto& destroyable = destroyables.emplace_back();
        size_t destroyableStartPos = cpuFile.Position();

        //Read base data and align to next piece of data
        cpuFile.ReadToMemory(&destroyable.base, sizeof(destroyable_base));
        cpuFile.Align(128);

        //Todo: Could allocate a buffer and read all this data at once then access with a span
        //Read base object data
        for (u32 j = 0; j < destroyable.base.num_objects; j++)
        {
            auto& subpiece = destroyable.subpieces.emplace_back();
            cpuFile.ReadToMemory(&subpiece, sizeof(subpiece_base)); //size = 64
        }
        for (u32 j = 0; j < destroyable.base.num_objects; j++)
        {
            auto& subpiece_data = destroyable.subpieces_data.emplace_back();
            cpuFile.ReadToMemory(&subpiece_data, sizeof(subpiece_base_data)); //size = 12
        }

        //Todo: Figure out what this data is meant to be. Game has some physical material code here. Maybe link material
        for (auto& subpiece : destroyable.subpieces)
            cpuFile.Skip(subpiece.num_links * 2);

        cpuFile.Align(4);

        //Read links
        for (u32 j = 0; j < destroyable.base.num_links; j++)
        {
            auto& link = destroyable.links.emplace_back();
            cpuFile.ReadToMemory(&link, sizeof(link_base)); //size = 16
        }
        cpuFile.Align(4);

        //Read dlods
        for (u32 j = 0; j < destroyable.base.num_dlods; j++)
        {
            auto& dlod = destroyable.dlods.emplace_back();
            cpuFile.ReadToMemory(&dlod, sizeof(dlod_base)); //size = 60
        }
        cpuFile.Align(4);

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
        cpuFile.Align(4);

        //Read instance data. Seems to be junk in the file and set at runtime. Unless it's offset from the packfile start or something painful like that
        if (destroyable.base.inst_data_offset != 0xFFFFFFFF)
        {
            cpuFile.ReadToMemory(&destroyable.instance_data, sizeof(destroyable_instance_data));
        }
        static_assert(sizeof(destroyable_instance_data) == 20, "destroyable_instance_data size check failed!");

        //Seek to end of this destroyable
        //cpuFile.SeekBeg(destroyableStartPos + destroyable.instance_data.data_size);

        //cpuFile.SeekBeg(destroyableStartPos + destroyable.base.transform_buffer_offset);

        //Todo: There may be some data we have to read/skip here. The code at this point was confusing. Something something aabb instances

        cpuFile.Align(16);
        u32 maybeTransformBufferSize = cpuFile.ReadUint32();
        cpuFile.Skip(destroyable.subpieces.size() * 36);
        cpuFile.Align(16);
    }



    std::vector<ChunkVertex> vertices;
    std::vector<uint32_t> indices;

    //Read indices
    gpuFile.SeekBeg(16);
    for (u32 i = 0; i < meshData.NumIndices; i++)
    {
        if (meshData.IndexSize == 2)
            indices.push_back(gpuFile.ReadUint16());
        else if (meshData.IndexSize == 4)
            indices.push_back(gpuFile.ReadUint32());
        else
            throw std::runtime_error("Unsupported mesh index size!");
    }

    //Read vertices
    u32 indicesEnd = 16 + (meshData.IndexSize * meshData.NumIndices);
    u32 verticesStart = indicesEnd + BinaryWriter::CalcAlign(indicesEnd, 16); //Todo: Is align needed?
    gpuFile.SeekBeg(verticesStart);
    for (u32 i = 0; i < meshData.NumVertices; i++)
    {
        ChunkVertex& vert = vertices.emplace_back();

        if (meshData.VertFormat == VertexFormat::Pixlit1UvNmap)
        {
            //Stride: 24 bytes
            gpuFile.ReadToMemory(&vert.Position, sizeof(Vec3));
            gpuFile.ReadToMemory(&vert.Normal, sizeof(ByteVec4));
            gpuFile.ReadToMemory(&vert.Tangent, sizeof(ByteVec4));
            gpuFile.ReadToMemory(&vert.Uv0, sizeof(ShortVec2));
        }
        else if (meshData.VertFormat == VertexFormat::Pixlit2UvNmap)
        {
            //Stride: 28 bytes
            gpuFile.ReadToMemory(&vert.Position, sizeof(Vec3));
            gpuFile.ReadToMemory(&vert.Normal, sizeof(ByteVec4));
            gpuFile.ReadToMemory(&vert.Tangent, sizeof(ByteVec4));
            gpuFile.ReadToMemory(&vert.Uv0, sizeof(ShortVec2));
            gpuFile.ReadToMemory(&vert.Uv1, sizeof(ShortVec2));
        }
        else if (meshData.VertFormat == VertexFormat::Pixlit3UvNmap)
        {
            //Stride: 32 bytes
            gpuFile.ReadToMemory(&vert.Position, sizeof(Vec3));
            gpuFile.ReadToMemory(&vert.Normal, sizeof(ByteVec4));
            gpuFile.ReadToMemory(&vert.Tangent, sizeof(ByteVec4));
            gpuFile.ReadToMemory(&vert.Uv0, sizeof(ShortVec2));
            gpuFile.ReadToMemory(&vert.Uv1, sizeof(ShortVec2));
            gpuFile.ReadToMemory(&vert.Uv2, sizeof(ShortVec2));
        }
        else
        {
            throw std::runtime_error("Unsupported vertex format!");
        }
    }

    //Also make simpler general purpose vertex buffer
    struct SimpleVertex
    {
        Vec3 Pos;
        Vec2 UV0;
        Vec3 Normal;
    };
    std::vector<SimpleVertex> vertices2;
    for (auto& vert : vertices)
    {
        Vec3 normal = { vert.Normal.x / 255.0f, vert.Normal.y / 255.0f, vert.Normal.z / 255.0f };
        normal.x *= 2.0f;
        normal.x -= 1.0f;
        normal.y *= 2.0f;
        normal.y -= 1.0f;
        normal.z *= 2.0f;
        normal.z -= 1.0f;
        vertices2.push_back(SimpleVertex{ .Pos = vert.Position, .UV0 = {(f32)vert.Uv0.x, (f32)vert.Uv0.y}, .Normal = normal });
    }

    //Output gltf file with submesh data
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Asset asset;
    u32 gltfModel_BufferIndex = 0;
    u32 gltfModel_BufferViewIndex = 0;
    u32 gltfModel_MeshIndex = 0;
    u32 gltfModel_NodeIndex = 0;
    u32 gltfModel_AccessorIndex = 0;

    asset.version = "2.0";
    asset.generator = "tinygltf";

    //Output subpiece meshes
    for (u32 destroyableIndex = 0; destroyableIndex < destroyables.size(); destroyableIndex++)
    {
        auto& destroyable = destroyables[destroyableIndex];
        u32 dlodIndex = 0;
        for (auto& dlod : destroyable.dlods)
        {
            for (u32 subpieceIndex = dlod.first_piece; subpieceIndex < dlod.first_piece + dlod.max_pieces; subpieceIndex++)
            {
                auto& subpiece = destroyable.subpieces[subpieceIndex];
                auto& subpieceData = destroyable.subpieces_data[subpieceIndex];
                if (subpieceData.render_subpiece >= meshData.Submeshes.size())
                {
                    printf("Skipping subpiece %d in destroyable %d. render_subpiece > MaxSubmeshIndex.\n", subpieceIndex, destroyableIndex);
                    continue;
                }

                u32 submeshIndex = subpieceData.render_subpiece;
                SubmeshData& submesh = meshData.Submeshes[submeshIndex];
                for (u32 j = 0; j < submesh.NumRenderBlocks; j++)
                {
                    RenderBlock& block = meshData.RenderBlocks[submesh.RenderBlocksOffset + j];

                    //Gltf mesh data
                    tinygltf::Mesh mesh;
                    tinygltf::Primitive primitive;
                    tinygltf::Node node;
                    tinygltf::Buffer vertexBuffer;
                    tinygltf::Buffer indexBuffer;
                    tinygltf::BufferView vertexBufferView;
                    tinygltf::BufferView indexBufferView;
                    tinygltf::Accessor positionAccessor;
                    tinygltf::Accessor Uv0Accessor;
                    tinygltf::Accessor normalAccessor;
                    tinygltf::Accessor indexAccessor;

                    //Generate gltf vertex and index buffers for this render block
                    std::vector<SimpleVertex> gltfVertices;
                    std::vector<u32> gltfIndices;

                    //Generate simplified vertex buffer that discards all attributes except position. Others aren't need yet
                    u32 curIndex = 0;
                    for (u32 k = block.StartIndex; k < block.StartIndex + block.NumIndices; k++)
                    {
                        u32 index = indices[k];
                        SimpleVertex& vertex = vertices2[index];

                        Vec3 pos = vertex.Pos;

                        pos = dlod.orient.rotate_point(pos);

                        pos.x += dlod.pos.x;
                        pos.y += dlod.pos.y;
                        pos.z += dlod.pos.z;

                        gltfVertices.push_back(
                        {
                            .Pos = pos,
                            .UV0 = Vec2{vertex.UV0.x / 1024.0f, vertex.UV0.y / 1024.0f},
                            .Normal = vertex.Normal
                        });
                        gltfIndices.push_back(curIndex++);
                    }

                    //Setup vertex buffer + views
                    vertexBuffer.data = ToByteVector<SimpleVertex>(gltfVertices);
                    u32 vertexBufferIndex = gltfModel_BufferIndex++;

                    vertexBufferView.buffer = vertexBufferIndex;
                    vertexBufferView.byteOffset = 0;
                    vertexBufferView.byteLength = vertexBuffer.data.size();
                    vertexBufferView.byteStride = sizeof(SimpleVertex);
                    vertexBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                    //Vertex accessors
                    u32 positionAccessorIndex = gltfModel_AccessorIndex++;
                    positionAccessor.bufferView = vertexBufferIndex;
                    positionAccessor.byteOffset = 0;
                    positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    positionAccessor.type = TINYGLTF_TYPE_VEC3;
                    positionAccessor.count = gltfVertices.size();

                    u32 uv0AccessorIndex = gltfModel_AccessorIndex++;
                    Uv0Accessor.bufferView = vertexBufferIndex;
                    Uv0Accessor.byteOffset = 12;
                    Uv0Accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    Uv0Accessor.type = TINYGLTF_TYPE_VEC2;
                    Uv0Accessor.count = gltfVertices.size();

                    u32 normalAccessorIndex = gltfModel_AccessorIndex++;
                    normalAccessor.bufferView = vertexBufferIndex;
                    normalAccessor.byteOffset = 20;
                    normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    normalAccessor.type = TINYGLTF_TYPE_VEC3;
                    normalAccessor.count = gltfVertices.size();

                    model.buffers.push_back(vertexBuffer);
                    model.bufferViews.push_back(vertexBufferView);
                    model.accessors.push_back(positionAccessor);
                    model.accessors.push_back(Uv0Accessor);
                    model.accessors.push_back(normalAccessor);

                    //Setup index buffer + views
                    indexBuffer.data = ToByteVector<u32>(gltfIndices);
                    u32 indexBufferIndex = gltfModel_BufferIndex++;

                    indexBufferView.buffer = indexBufferIndex;
                    indexBufferView.byteOffset = 0;
                    indexBufferView.byteLength = indexBuffer.data.size();
                    indexBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

                    //Index accessors
                    u32 indexAccessorIndex = gltfModel_AccessorIndex++;
                    indexAccessor.bufferView = indexBufferIndex;
                    indexAccessor.byteOffset = 0;
                    indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                    indexAccessor.count = gltfIndices.size();
                    indexAccessor.type = TINYGLTF_TYPE_SCALAR;

                    model.buffers.push_back(indexBuffer);
                    model.bufferViews.push_back(indexBufferView);
                    model.accessors.push_back(indexAccessor);

                    //Setup gltf mesh primitive
                    primitive.indices = indexAccessorIndex;
                    primitive.attributes["POSITION"] = positionAccessorIndex;
                    primitive.attributes["TEXCOORD_0"] = uv0AccessorIndex;
                    primitive.attributes["NORMAL"] = normalAccessorIndex;
                    primitive.material = 0;
                    primitive.mode = TINYGLTF_MODE_TRIANGLE_STRIP;
                    mesh.primitives.push_back(primitive);

                    //Scene node for mesh
                    node.mesh = gltfModel_MeshIndex++;
                    scene.nodes.push_back(gltfModel_NodeIndex++);

                    model.meshes.push_back(mesh);
                    model.nodes.push_back(node);
                }
            }
            dlodIndex++;
        }
    }

    //Add scene with all the mesh nodes to model
    model.asset = asset;
    model.scenes.push_back(scene);

    //Basic material
    tinygltf::Material mat;
    mat.pbrMetallicRoughness.baseColorFactor = { 1.0f, 0.9f, 0.9f, 1.0f };
    mat.doubleSided = true;
    model.materials.push_back(mat);

    //Save mesh to gltf file
    std::string gltfOutPath = Path::GetParentDirectory(cpuFilePath.value()) + Path::GetFileNameNoExtension(cpuFilePath.value()) + "_cchk_full_output.gltf";
    printf("Outputting file at '%s'\n", gltfOutPath.c_str());
    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&model, gltfOutPath,
        true, // embedImages
        true, // embedBuffers
        true, // pretty print
        false); // write binary

    printf("Done!\n");

    return 0;
}