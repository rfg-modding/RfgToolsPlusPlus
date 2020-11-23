#include "StaticMesh.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "hashes/Hash.h"
#include "MeshHelpers.h"
#include "MeshDataBlock.h"
#include <filesystem>

void StaticMesh::Read(BinaryReader& cpuFile, const string& name, u32 signature, u32 version, bool isChunkFile)
{
    Name = name;

    if (isChunkFile)
    {
        //Todo: Use c_chunk_base here instead. Game directly maps that struct to start of the file. Rest of bytes to reach 704 (mesh data offset) is an align(64)
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

        ChunkHeader chunkHeader;
        cpuFile.ReadToMemory(&chunkHeader, sizeof(ChunkHeader));
        cpuFile.SeekBeg(chunkHeader.render_cpu_data_offset);
        Header.Signature = chunkHeader.signature;
        Header.Version = chunkHeader.version;

        //Read submesh metadata
        MeshDataBlock meshData;
        meshData.Read(cpuFile);

        MeshVersion = meshData.Version;
        CpuDataSize = meshData.CpuDataSize;
        GpuDataSize = meshData.GpuDataSize;
        NumSubmeshes = meshData.NumSubmeshes;
        SubmeshesOffset = meshData.SubmeshesOffset;
        memcpy(&VertexBufferConfig, &meshData.NumVertices, sizeof(VertexBufferConfig));
        memcpy(&IndexBufferConfig, &meshData.NumIndices, sizeof(IndexBufferConfig));
        SubMeshes = meshData.Submeshes;
        RenderBlocks = meshData.RenderBlocks;
        cpuFile.Align(16);

        //Read texture names
        u32 textureNamesBlockSize = cpuFile.ReadUint32();
        u64 textureNamesStartPos = cpuFile.Position();
        //Read strings until we reach the end of the string block
        while (cpuFile.Position() - textureNamesStartPos < textureNamesBlockSize)
        {
            //Read null terminated string
            TextureNames.push_back(cpuFile.ReadNullTerminatedString());
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
        printf("Pos after reading texture names: %d\n", cpuFile.Position());

        //Todo: Try to merge this and the code of MaterialBlock. They're very similar with slight differences. Unsure if that means they're really different or if I'm just not seeing the common thread
        u32 materialAlign = cpuFile.ReadUint32(); //16 in all files I've seen so far
        Header.NumMaterials = cpuFile.ReadUint32();
        for (u32 i = 0; i < Header.NumMaterials; i++)
            MaterialIds.push_back(cpuFile.ReadUint32());

        cpuFile.Align(materialAlign);
        cpuFile.Skip(Header.NumMaterials * 8);
        cpuFile.Align(materialAlign);

        printf("Pos of first material: %d\n", cpuFile.Position());
        for (u32 i = 0; i < Header.NumMaterials; i++)
        {
            RfgMaterial& material = ChunkMaterials.emplace_back();
            material.Read(cpuFile);
        }

        printf("Position at end of material data block 0: %d\n", cpuFile.Position());
        //Todo: Figure out if supposed to Align(64) or Skip(64) here. Game seems to Align(64) but Skip(64) is what gets us to the next data section in the test file
        cpuFile.Align(64);
        //cpuFile.Skip(64);
        printf("Position at expected start of destruction data block: %d\n", cpuFile.Position());

        //Todo: If the file has any general objects they'd be here. So far all I've seen have 0. This is a value in the extended header, c_chunk_base that isn't used yet

        //Todo: Some other data after this. Ignoring for now since don't need it yet. Will need eventually for editing buildings.

        //Jump straight to destruction offset since we don't need the other data before it currently
        cpuFile.SeekBeg(chunkHeader.destruction_offset);
        if (cpuFile.Position() != chunkHeader.destruction_offset)
            throw std::exception("Error! Current location does not equal the destruction data offset as it is expected to at this point!");

        cpuFile.Align(128); //Todo: Just guessing at this point...
        printf("Position at expected start of destroyables data block: %d\n", cpuFile.Position());

        u32 numDestroyables = cpuFile.ReadUint32();
        //Seemingly random data skip that the game does. Maybe empty space in case it's needed eventually
        cpuFile.Skip((numDestroyables * 8) + 4);
        //cpuFile.Align(16);

        printf("Position at expected start of first destroyable: %d\n", cpuFile.Position());

        //TODO: SUPPORT numDestroyables > 1  and switch to i < numDestroyables here
        for (u32 i = 0; i < numDestroyables; i++)
        {
            cpuFile.Align(16);
            printf("Position at start of destroyable[%d]: %d\n", i, cpuFile.Position());

            //Create new destroyable instance
            auto& destroyable = Destroyables.emplace_back();

            //Read base data and align to next piece of data
            cpuFile.ReadToMemory(&destroyable.base, sizeof(destroyable_base)/*44*/);
            cpuFile.Align(128);

            auto a = cpuFile.Position();

            //Todo: Could allocate a buffer and read all this data at once then access with a span
            //Read base object data
            for (u32 j = 0; j < destroyable.base.num_objects; j++)
            {
                auto& subpiece = destroyable.subpieces.emplace_back();
                cpuFile.ReadToMemory(&subpiece, sizeof(subpiece_base)/*64*/);
            }
            for (u32 j = 0; j < destroyable.base.num_objects; j++)
            {
                auto& subpiece_data = destroyable.subpieces_data.emplace_back();
                cpuFile.ReadToMemory(&subpiece_data, sizeof(subpiece_base_data)/*12*/);
            }

            //for (u32 j = 0; j < destroyable.base.num_objects; j++)
            //{
            //    std::cout << destroyable.subpieces_data[j].render_subpiece << "\n";
            //}

            //Todo: LABEL: Big weird physical material do-while loop
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
                cpuFile.ReadToMemory(&link, sizeof(link_base)/*16*/);
            }
            auto e = cpuFile.Position();
            cpuFile.Align(4);
            auto f = cpuFile.Position();
            printf("Position after reading links: %d\n", cpuFile.Position());


            //Read dlods
            for (u32 j = 0; j < destroyable.base.num_dlods; j++)
            {
                auto& dlod = destroyable.dlods.emplace_back();
                cpuFile.ReadToMemory(&dlod, sizeof(dlod_base)/*60*/);
            }
            auto g = cpuFile.Position();
            cpuFile.Align(4);
            auto h = cpuFile.Position();
            printf("Position after reading dlods: %d\n", cpuFile.Position());


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
            printf("Position after reading rbb nodes: %d\n", cpuFile.Position());


            //Read instance data. Seems to be junk in the file and set at runtime. Unless it's offset from the packfile start or something painful like that
            cpuFile.ReadToMemory(&destroyable.instance_data, sizeof(destroyable_instance_data)/*20*/);
            static_assert(sizeof(destroyable_instance_data) == 20, "destroyable_instance_data size check failed!");
            printf("Position after reading destroyable instance data: %d\n", cpuFile.Position());

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

        printf("Position at end of destroyables: %d\n", cpuFile.Position());
        printf("Num destroyables: %d\n", numDestroyables);
        printf("\n");
        //Todo: Read whatever data comes after this
        //Todo: Read collision models at collision model offset. This is havok data 
    }
    else
    {
        Header.Read(cpuFile, signature, version);

        //Static mesh specific header data
        NumLods = cpuFile.ReadUint32();
        cpuFile.Skip(4);
        LodSubmeshIdOffset = cpuFile.ReadUint32();
        cpuFile.Skip(4);
        MeshTagsOffset = cpuFile.ReadUint32();
        cpuFile.Skip(4);

        MeshTagsNumTags = cpuFile.ReadUint32();
        cpuFile.Skip(4);
        MeshTagsInternalOffset = cpuFile.ReadUint32();
        cpuFile.Skip(4);

        CmIndex = cpuFile.ReadUint32();
        cpuFile.Skip(4);

        //Seek to mesh data offset and read mesh data
        cpuFile.SeekBeg(Header.MeshOffset);
        MeshVersion = cpuFile.ReadUint32();
        MeshSimpleCrc = cpuFile.ReadUint32();
        CpuDataSize = cpuFile.ReadUint32();
        GpuDataSize = cpuFile.ReadUint32();
        NumSubmeshes = cpuFile.ReadUint32();
        SubmeshesOffset = cpuFile.ReadUint32();
        VertexBufferConfig.Read(cpuFile);
        IndexBufferConfig.Read(cpuFile);

        //Read submesh data. Usually only one submesh in static meshes
        for (int i = 0; i < NumSubmeshes; i++)
        {
            SubmeshData& submesh = SubMeshes.emplace_back();
            submesh.Read(cpuFile);
        }
        for (int i = 0; i < IndexBufferConfig.NumBlocks; i++)
        {
            RenderBlock& renderBlock = RenderBlocks.emplace_back();
            renderBlock.Read(cpuFile);
        }

        //Todo: Compare with previous crc and report error if they don't match
        u32 MeshSimpleCrc2 = cpuFile.ReadUint32();

        //Read material data block
        cpuFile.SeekBeg(Header.MaterialMapOffset);
        MaterialBlock.Read(cpuFile, Header.MaterialsOffset); //Handles reading material map and materials

        //Read texture names
        cpuFile.SeekBeg(Header.TextureNamesOffset);
        for (auto& material : MaterialBlock.Materials)
        {
            for (auto& textureDesc : material.TextureDescs)
            {
                cpuFile.SeekBeg(Header.TextureNamesOffset + textureDesc.NameOffset);
                TextureNames.push_back(cpuFile.ReadNullTerminatedString());
            }
        }

        //Todo: Read mesh_tag list after texture name list
        //Todo: Read havok data that is sometimes present here (see tharsis_gun_weapon.csmesh_pc). Has MCKH signature which is always with havok stuff
    }

    readHeader_ = true;
}

std::optional<MeshInstanceData> StaticMesh::ReadSubmeshData(BinaryReader& gpuFile, u32 index, bool isChunkFile)
{
    if (!readHeader_ || index >= SubMeshes.size())
        return {};

    if (isChunkFile)
    {
        if (index == 0)
        {
        //    u64 vertexDataOffset = 0;
        //    u8* indexBuffer0;
        //    u32 indexBufferSize0 = 0;
        //    u8* indexBuffer1;
        //    u32 indexBufferSize1 = 0;
        //    {
        //        //Calculate positions of index and vertex data
        //        u32 firstRenderBlockIndex = 0;
        //        u32 startIndex = RenderBlocks[firstRenderBlockIndex].StartIndex;
        //        u32 numIndices = 0;
        //        for (u32 i = firstRenderBlockIndex; i < firstRenderBlockIndex + SubMeshes[0].NumRenderBlocks; i++)
        //        {
        //            numIndices += RenderBlocks[i].NumIndices;
        //        }

        //        u64 indexDataOffset = 16; //Start of index data
        //        u64 indexDataEnd = indexDataOffset + (IndexBufferConfig.NumIndices * IndexBufferConfig.IndexSize); //End of index data
        //        u64 firstIndexOffset = indexDataOffset + (startIndex * IndexBufferConfig.IndexSize); //Offset of first index for this submesh
        //        vertexDataOffset = indexDataEnd + BinaryWriter::CalcAlign(indexDataEnd, 16); //Start of vertex data

        //        //Read index buffer
        //        gpuFile.SeekBeg(firstIndexOffset);
        //        indexBufferSize0 = numIndices * IndexBufferConfig.IndexSize;
        //        indexBuffer0 = new u8[indexBufferSize0];
        //        gpuFile.ReadToMemory(indexBuffer0, indexBufferSize0);
        //    }
        //    {
        //        //Calculate positions of index and vertex data
        //        u32 firstRenderBlockIndex = 1;
        //        u32 startIndex = RenderBlocks[firstRenderBlockIndex].StartIndex;
        //        u32 numIndices = 0;
        //        for (u32 i = firstRenderBlockIndex; i < firstRenderBlockIndex + SubMeshes[0].NumRenderBlocks; i++)
        //        {
        //            numIndices += RenderBlocks[i].NumIndices;
        //        }

        //        u64 indexDataOffset = 16; //Start of index data
        //        u64 indexDataEnd = indexDataOffset + (IndexBufferConfig.NumIndices * IndexBufferConfig.IndexSize); //End of index data
        //        u64 firstIndexOffset = indexDataOffset + (startIndex * IndexBufferConfig.IndexSize); //Offset of first index for this submesh
        //        vertexDataOffset = indexDataEnd + BinaryWriter::CalcAlign(indexDataEnd, 16); //Start of vertex data

        //        //Read index buffer
        //        gpuFile.SeekBeg(firstIndexOffset);
        //        indexBufferSize1 = numIndices * IndexBufferConfig.IndexSize;
        //        indexBuffer1 = new u8[indexBufferSize1];
        //        gpuFile.ReadToMemory(indexBuffer1, indexBufferSize1);                
        //    }

        //    u32 indexBufferSize = indexBufferSize0 + indexBufferSize1;
        //    u8* indexBuffer = new u8[indexBufferSize];
        //    memcpy(indexBuffer, indexBuffer0, indexBufferSize0);
        //    memcpy(indexBuffer + indexBufferSize0, indexBuffer1, indexBufferSize1);

        //    gpuFile.SeekBeg(vertexDataOffset);
        //    u32 vertexBufferSize = VertexBufferConfig.NumVerts * VertexBufferConfig.VertexStride0;
        //    u8* vertexBuffer = new u8[vertexBufferSize];
        //    gpuFile.ReadToMemory(vertexBuffer, vertexBufferSize);

        //    //Return submesh data buffers
        //    return MeshInstanceData
        //    {
        //        .VertexBuffer = std::span<u8>(vertexBuffer, vertexBufferSize),
        //        .IndexBuffer = std::span<u8>(indexBuffer, indexBufferSize)
        //    };
        }

        //Get submesh data
        SubmeshData& submesh = SubMeshes[index];

        //Calc number of render blocks and indices
        u32 firstRenderBlockIndex = 0;
        for (u32 i = 0; i < index; i++)
        {
            firstRenderBlockIndex += SubMeshes[i].NumRenderBlocks;
        }
        u32 startIndex = RenderBlocks[firstRenderBlockIndex].StartIndex;
        u32 numIndices = 0;
        for (u32 i = firstRenderBlockIndex; i < firstRenderBlockIndex + submesh.NumRenderBlocks; i++)
        {
            numIndices += RenderBlocks[i].NumIndices;
        }

        //Calculate positions of index and vertex data
        u64 indexDataOffset = 16; //Start of index data
        u64 indexDataEnd = indexDataOffset + (IndexBufferConfig.NumIndices * IndexBufferConfig.IndexSize); //End of index data
        u64 firstIndexOffset = indexDataOffset + (startIndex * IndexBufferConfig.IndexSize); //Offset of first index for this submesh
        u64 vertexDataOffset = indexDataEnd + BinaryWriter::CalcAlign(indexDataEnd, 16); //Start of vertex data
        u64 firstVertexOffset = vertexDataOffset + (startIndex * VertexBufferConfig.VertexStride0); //Offset of first vertex for this submesh

        //Read index buffer
        //gpuFile.SeekBeg(firstIndexOffset);
        //u32 indexBufferSize = numIndices * IndexBufferConfig.IndexSize;
        //u8* indexBuffer = new u8[indexBufferSize];
        //gpuFile.ReadToMemory(indexBuffer, indexBufferSize);

        u32 indexBufferSize = numIndices * IndexBufferConfig.IndexSize;
        u8* indexBuffer = new u8[indexBufferSize];
        u32 indexBufferOffset = 0;
        for (u32 i = 0; i < submesh.NumRenderBlocks; i++)
        {
            RenderBlock& block = RenderBlocks[(u64)(firstRenderBlockIndex + i)];
            u64 blockFirstIndexOffset = 16 + (block.StartIndex * (u32)IndexBufferConfig.IndexSize); //Offset of first index for this block
            u64 blockIndicesSize = block.NumIndices * (u32)IndexBufferConfig.IndexSize;

            gpuFile.SeekBeg(blockFirstIndexOffset);
            gpuFile.ReadToMemory((indexBuffer + indexBufferOffset), blockIndicesSize);
            indexBufferOffset += blockIndicesSize;
        }

        //Read vertex buffer
        gpuFile.SeekBeg(vertexDataOffset);
        u32 vertexBufferSize = VertexBufferConfig.NumVerts * VertexBufferConfig.VertexStride0;
        u8* vertexBuffer = new u8[vertexBufferSize];
        gpuFile.ReadToMemory(vertexBuffer, vertexBufferSize);

        //Return submesh data buffers
        return MeshInstanceData
        {
            .VertexBuffer = std::span<u8>(vertexBuffer, vertexBufferSize),
            .IndexBuffer = std::span<u8>(indexBuffer, indexBufferSize)
        };
    }
    else
    {
        //Get submesh data
        SubmeshData& submesh = SubMeshes[index];

        //Calc number of render blocks and indices
        u32 firstRenderBlockIndex = 0;
        for (u32 i = 0; i < index; i++)
        {
            firstRenderBlockIndex += SubMeshes[i].NumRenderBlocks;
        }
        u32 startIndex = RenderBlocks[firstRenderBlockIndex].StartIndex;
        u32 numIndices = 0;
        for (u32 i = firstRenderBlockIndex; i < firstRenderBlockIndex + submesh.NumRenderBlocks; i++)
        {
            numIndices += RenderBlocks[i].NumIndices;
        }

        //Calculate positions of index and vertex data
        u64 indexDataOffset = 16; //Start of index data
        u64 indexDataEnd = indexDataOffset + (IndexBufferConfig.NumIndices * IndexBufferConfig.IndexSize); //End of index data
        u64 firstIndexOffset = indexDataOffset + (startIndex * IndexBufferConfig.IndexSize); //Offset of first index for this submesh
        u64 vertexDataOffset = indexDataEnd + BinaryWriter::CalcAlign(indexDataEnd, 16); //Start of vertex data
        u64 firstVertexOffset = vertexDataOffset + (startIndex * VertexBufferConfig.VertexStride0); //Offset of first vertex for this submesh

        //Read index buffer
        gpuFile.SeekBeg(firstIndexOffset);
        u32 indexBufferSize = numIndices * IndexBufferConfig.IndexSize;
        u8* indexBuffer = new u8[indexBufferSize];
        gpuFile.ReadToMemory(indexBuffer, indexBufferSize);

        //Read vertex buffer
        gpuFile.SeekBeg(firstVertexOffset);
        u32 vertexBufferSize = VertexBufferConfig.NumVerts * VertexBufferConfig.VertexStride0;
        u8* vertexBuffer = new u8[vertexBufferSize];
        gpuFile.ReadToMemory(vertexBuffer, vertexBufferSize);

        //Return submesh data buffers
        return MeshInstanceData
        {
            .VertexBuffer = std::span<u8>(vertexBuffer, vertexBufferSize),
            .IndexBuffer = std::span<u8>(indexBuffer, indexBufferSize)
        };
    }
}

void StaticMesh::Write(BinaryWriter& out)
{
    //Todo: Implement
}

void StaticMesh::Write(const string& path)
{
    BinaryWriter out(path);
    Write(out);
}

//Todo: See if more of this function can be moved into MeshHelpers:: to reuse on other mesh formats
void StaticMesh::WriteToObj(const string& gpuFilePath, const string& outputFolderPath, const string& diffuseMapPath, const string& specularMapPath, const string& normalMapPath)
{
    BinaryReader gpuFile(gpuFilePath);
    if (!std::filesystem::exists(outputFolderPath))
        return;

    //For each submesh write a .obj file and a .mtl file
    for (u32 i = 0; i < SubMeshes.size(); i++)
    {
        SubmeshData& submesh = SubMeshes[i];

        //Output file paths
        string objFilePath = outputFolderPath + "\\" + Path::GetFileNameNoExtension(Name) + std::to_string(i) + ".obj";
        string mtlName = Path::GetFileNameNoExtension(Name) + std::to_string(i) + "_mat"; //Material name used inside .mtl and .obj files
        string mtlFileName = Path::GetFileNameNoExtension(Name) + std::to_string(i) + ".mtl"; //.mtl filename
        string mtlFilePath = outputFolderPath + "\\" + mtlFileName;

        //Contains indices, vertices, UVs, etc
        std::ofstream obj(objFilePath, std::ios_base::out | std::ios_base::trunc);
        //Lists what textures to use for diffuse, specular, normal, etc
        std::ofstream mtl(mtlFilePath, std::ios_base::out | std::ios_base::trunc);

        //Write material name to obj
        obj << "mtllib " << mtlName << "\n";
        obj << "usemtl " << mtlName << "\n";

        //Try to get vertex and index buffers
        std::optional<MeshInstanceData> maybeMeshData = ReadSubmeshData(gpuFile, i);
        if (!maybeMeshData)
        {
            printf("Failed to get mesh data for submesh %d of static mesh %s. Stopping export.\n", i, Name.c_str());
            return;
        }
        MeshInstanceData meshData = maybeMeshData.value();
        std::span<u8> indexBufferBytes = meshData.IndexBuffer;
        std::span<u8> vertexBufferBytes = meshData.VertexBuffer;

        //Write vertex data
        bool result = MeshHelpers::WriteVerticesToObj(obj, VertexBufferConfig.Format, vertexBufferBytes);
        if (!result)
        {
            printf("Failed to write vertex data for submesh %d of static mesh %s. Stopping export.\n", i, Name.c_str());
            return;
        }

        //Write faces
        std::span<u16> indices = std::span<u16>((u16*)indexBufferBytes.data(), indexBufferBytes.size_bytes() / 2);
        for (u32 j = 1; j < indices.size() - 2; j++) //Todo: Why is j starting at 1?
        {
            //Get index values. Increment by one since .obj indices start at 1 instead of 0
            u16 index0 = indices[j] + 1;
            u16 index1 = indices[j + 1] + 1;
            u16 index2 = indices[j + 2] + 1;

            //Output face as "f index0/index0 index1/index1 index2/index2". E.g. "f 2/2 27/27 16/16"
            obj << "f " << index0 << "/" << index0 << " " << index1 << "/" << index1 << " " << index2 << "/" << index2 << "\n";
        }

        //Release mesh data
        delete[] meshData.IndexBuffer.data();
        delete[] meshData.VertexBuffer.data();

        //Write material data if textures provided
        mtl << "newmtl " << mtlName << "\n";
        mtl << "Ka 1.000 1.000 1.000" << "\n";
        mtl << "Kd 1.000 1.000 1.000" << "\n";
        mtl << "Ks 0.000 0.000 0.000" << "\n";
        mtl << "\n";

        if (diffuseMapPath != "")
            mtl << "map_Kd " << diffuseMapPath << "\n";
        if (specularMapPath != "")
            mtl << "map_Ns " << specularMapPath << "\n";
        if (normalMapPath != "")
            mtl << "map_bump " << normalMapPath << "\n";
    }
}
