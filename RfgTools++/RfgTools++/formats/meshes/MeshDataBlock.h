#pragma once
#include "common/Typedefs.h"
#include "MeshEnums.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include <BinaryTools/BinaryReader.h>
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
    VertexFormat VertFormat; //1 byte in files
    u8 NumUvChannels = 0;
    u8 VertexStride1 = 0;
    u32 VertexOffset = 0;

    //Index data layout (12 bytes)
    u32 NumIndices = 0;
    u32 IndicesOffset = 0;
    u8 IndexSize = 0;
    PrimitiveTopology PrimitiveType; //1 byte in files
    u16 NumRenderBlocks = 0;

    //Size of all data before this line = 48 bytes in file. Data after this has dynamic size

    //RFG meshes can be split into multiple submeshes. So far only seen this in chunk files (destructibles)
    std::vector<SubmeshData> Submeshes = {};
    std::vector<RenderBlock> RenderBlocks = {};

    void Read(BinaryReader& reader)
    {
        //Read static data that's always present in this structure in files and always 48 bytes
        reader.ReadToMemory(this, 48);

        //Reserve spots for the dynamic objects since we know how many we need ahead of time
        Submeshes.reserve(NumSubmeshes);
        RenderBlocks.reserve(NumRenderBlocks);

        //Read dynamic data
        for (u32 i = 0; i < NumSubmeshes; i++)
        {
            SubmeshData& submesh = Submeshes.emplace_back();
            reader.ReadToMemory(&submesh, sizeof(SubmeshData));
        }
        for (u32 i = 0; i < NumRenderBlocks; i++)
        {
            RenderBlock& renderBlock = RenderBlocks.emplace_back();
            reader.ReadToMemory(&renderBlock, sizeof(RenderBlock));
        }
    }
};