#pragma once
#include "common/Typedefs.h"
#include "MeshEnums.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <vector>

//Structure used across multiple RFG mesh formats that describes vertex and index data layout + format
//and other info relevant to using the mesh.
class MeshDataBlock
{
public:
    //General mesh data (24 bytes)
    u32 Version = 0;
    u32 VerificationHash = 0;
    u32 CpuDataSize = 0;
    u32 GpuDataSize = 0;
    u32 NumSubmeshes = 0;
    u32 SubmeshesOffset = 0;

    //Vertex data layout (12 bytes)
    u32 NumVertices = 0;
    u8 VertexStride0 = 0;
    VertexFormat VertFormat = VertexFormat::Invalid; //1 byte in files
    u8 NumUvChannels = 0;
    u8 VertexStride1 = 0;
    u32 VertexOffset = 0;

    //Index data layout (12 bytes)
    u32 NumIndices = 0;
    u32 IndicesOffset = 0;
    u8 IndexSize = 0;
    PrimitiveTopology PrimitiveType = PrimitiveTopology::Invalid; //1 byte in files
    u16 NumRenderBlocks = 0;

    //Size of all data before this line = 48 bytes in file. Data after this has dynamic size

    //RFG meshes can be split into multiple submeshes. So far only seen this in chunk files (destructibles)
    std::vector<SubmeshData> Submeshes = {};
    std::vector<RenderBlock> RenderBlocks = {};

    void Read(BinaryReader& reader, bool patchBufferOffsets = false)
    {
        u64 startPos = reader.Position();

        //Read static data that's always present in this structure in files and always 48 bytes
        reader.ReadToMemory(this, 48);
        reader.Align(16);

        //Read dynamic data
        NumRenderBlocks = 0; //Recount since NumRenderBlocks is incorrect in some files
        for (u32 i = 0; i < NumSubmeshes; i++)
        {
            SubmeshData& submesh = Submeshes.emplace_back();
            reader.ReadToMemory(&submesh, sizeof(SubmeshData));
            NumRenderBlocks += static_cast<u32>(submesh.NumRenderBlocks);
        }
        for (u32 i = 0; i < NumRenderBlocks; i++)
        {
            RenderBlock& renderBlock = RenderBlocks.emplace_back();
            reader.ReadToMemory(&renderBlock, sizeof(RenderBlock));
        }

        //Todo: Fix this for files like gterrain_pc and gtmesh_pc that have multiple meshes. Seems to need absolute offset to calculate correct align pad. Luckily they have correct offsets by default
        //Patch vertex and index offset since some files don't have correct values.
        if (patchBufferOffsets)
        {
            IndicesOffset = 16;
            u32 indicesEnd = IndicesOffset + (NumIndices * IndexSize);
            VertexOffset = indicesEnd + static_cast<u32>(BinaryWriter::CalcAlign(indicesEnd, 16));
        }

        //Patch render block offsets for easy access later
        u32 renderBlockOffset = 0;
        for (auto& submesh : Submeshes)
        {
            submesh.RenderBlocksOffset = renderBlockOffset;
            renderBlockOffset += submesh.NumRenderBlocks;
        }

        u32 endVerificationHash = reader.ReadUint32();
        if (VerificationHash != endVerificationHash)
            throw std::runtime_error("Error! Mesh verification hashes didn't match when reading a mesh data block");
        if (reader.Position() - startPos != CpuDataSize)
            throw std::runtime_error("Error! Mesh data block size doesn't equal the expected data size!");
    }
};