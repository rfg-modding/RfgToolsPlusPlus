#include "StaticMesh.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "hashes/Hash.h"
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

void StaticMesh::Write(BinaryWriter& out)
{
    //Todo: Implement
}

void StaticMesh::Write(const string& path)
{
    BinaryWriter out(path);
    Write(out);
}
