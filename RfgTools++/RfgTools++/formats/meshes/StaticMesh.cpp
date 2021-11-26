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
    MeshInfo.Read(cpuFile, true);

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

std::optional<MeshInstanceData> StaticMesh::ReadMeshData(BinaryReader& gpuFile)
{
    if (!readHeader_)
        return {};

    //Read index buffer
    gpuFile.SeekBeg(MeshInfo.IndicesOffset);
    u32 indicesSize = MeshInfo.NumIndices * MeshInfo.IndexSize;
    std::vector<u8> indices(indicesSize);
    gpuFile.ReadToMemory(indices.data(), indicesSize);

    //Read vertex buffer
    gpuFile.SeekBeg(MeshInfo.VertexOffset);
    u32 verticesSize = MeshInfo.NumVertices * MeshInfo.VertexStride0;
    std::vector<u8> vertices(verticesSize);
    gpuFile.ReadToMemory(vertices.data(), verticesSize);

    //Return submesh data buffers
    return MeshInstanceData
    {
        .Info = MeshInfo,
        .VertexBuffer = vertices,
        .IndexBuffer = indices
    };
}

void StaticMesh::WriteToGltf(BinaryReader& gpuFile, const string& outPath, const string& diffusePath, const string& specularPath, const string& normalPath)
{
    if (!std::filesystem::exists(Path::GetParentDirectory(outPath)))
    {
        printf("Error in StaticMesh::WriteToGltf() for mesh %s. Output path doesn't exist.\n", Name.c_str());
        return;
    }

    //Extract mesh data
    std::optional<MeshInstanceData> data = ReadMeshData(gpuFile);
    if (!data)
    {
        printf("Error in StaticMesh::WriteToGltf() for mesh %s. Failed to read mesh data.\n", Name.c_str());
        return;
    }

    //Write mesh to gltf file
    MeshHelpers::WriteToGltf(MeshInfo, NumLods, data.value().IndexBuffer, data.value().VertexBuffer, outPath, diffusePath, specularPath, normalPath);
}