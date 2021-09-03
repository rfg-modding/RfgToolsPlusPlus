#include "StaticMesh.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "hashes/Hash.h"
#include "MeshHelpers.h"
#include <filesystem>

void StaticMesh::Read(BinaryReader& cpuFile, const string& name, u32 signature, u32 version)
{
    Name = name;
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
    //Todo: Read character mesh specific data found here in ccmesh_pc files

    readHeader_ = true;
}

std::optional<MeshInstanceData> StaticMesh::ReadSubmeshData(BinaryReader& gpuFile, u32 index)
{
    if (!readHeader_ || index >= SubMeshes.size())
        return {};

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

    //Read index buffer
    gpuFile.SeekBeg(firstIndexOffset);
    u32 indexBufferSize = numIndices * IndexBufferConfig.IndexSize;
    u8* indexBuffer = new u8[indexBufferSize];
    gpuFile.ReadToMemory(indexBuffer, indexBufferSize);

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
