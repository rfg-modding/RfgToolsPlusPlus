#include "StaticMesh.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "hashes/Hash.h"
#include <filesystem>

void StaticMesh::Read(BinaryReader& cpuFile, const string& name)
{
    Name = name;
    Header.Read(cpuFile, 0xC0FFEE11);

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

    //Read mesh data
    //Seek to mesh data offset
    cpuFile.SeekBeg(Header.MeshOffset);

    MeshVersion = cpuFile.ReadUint32();
    MeshSimpleCrc = cpuFile.ReadUint32();
    CpuDataSize = cpuFile.ReadUint32();
    GpuDataSize = cpuFile.ReadUint32();
    NumSubmeshes = cpuFile.ReadUint32();
    SubmeshesOffset = cpuFile.ReadUint32();
    VertexBufferConfig.Read(cpuFile);
    IndexBufferConfig.Read(cpuFile);

    for (int i = 0; i < NumSubmeshes; i++)
    {
        SubmeshData& submesh = SubMeshes.emplace_back();
        submesh.Read(cpuFile);
    }
    for (int i = 0; i < NumSubmeshes; i++)
    {
        RenderBlock& renderBlock = RenderBlocks.emplace_back();
        renderBlock.Read(cpuFile);
    }
    
    //Todo: Compare with previous crc and report error if they don't match
    u32 MeshSimpleCrc2 = cpuFile.ReadUint32();
    
    //Align to 16 bytes before next section
    cpuFile.Align(16);

    //Read material data block
    MaterialBlock.Read(cpuFile);

    //Read texture names
    cpuFile.SeekBeg(Header.TextureNamesOffset);
    for (auto& textureDesc : MaterialBlock.TextureDescs)
    {
        cpuFile.SeekBeg(Header.TextureNamesOffset + textureDesc.NameOffset);
        TextureNames.push_back(cpuFile.ReadNullTerminatedString());
    }

    readHeader_ = true;
}

std::optional<MeshInstanceData> StaticMesh::ReadSubmeshData(BinaryReader& gpuFile)
{
    if (!readHeader_)
        return {};

    //Todo: Add support for different data layouts based on the PrimitiveTopology and VertexFormat enums
    //Note: Currently assuming only one index and vertex type for static meshes, need to set up way of dynamically handling different layouts across meshes
    //Read verification crc at start of gpu file
    u32 gpuFileMeshSimpleCrc = gpuFile.ReadUint32(); //Todo: Compare with other CRCs and error if not equal
    gpuFile.Align(16);

    //Read index buffer
    u32 indexBufferSize = IndexBufferConfig.NumIndices * IndexBufferConfig.IndexSize;
    u8* indexBuffer = new u8[indexBufferSize];
    gpuFile.ReadToMemory(indexBuffer, indexBufferSize);
    gpuFile.Align(16);

    //Read vertex buffer
    u32 vertexBufferSize = VertexBufferConfig.NumVerts * VertexBufferConfig.VertexStride0;
    u8* vertexBuffer = new u8[vertexBufferSize];
    gpuFile.ReadToMemory(vertexBuffer, vertexBufferSize);
    gpuFile.Align(16);

    //Return submesh data
    return MeshInstanceData
    {
        .VertexBuffer = std::span<u8>(vertexBuffer, vertexBufferSize),
        .IndexBuffer = std::span<u8>(indexBuffer, indexBufferSize)
    };
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
