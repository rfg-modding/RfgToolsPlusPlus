#pragma once
#include "common/Typedefs.h"
#include "MeshEnums.h"
#include <BinaryTools/BinaryReader.h>

//Description of mesh index buffer data layout. Typically you'll find this in mesh cpu files describing a index buffer in the gpu file.
//E.g. csmesh_pc files have one of these per submesh which describe a index buffer in the matching gsmesh_pc file.
//See also: VertexBufferData.h
class IndexBufferData
{
public:
    u32 NumIndices; //The number of indices in the buffer
    u32 IndicesOffset; //Offset from the start of the gpu file to the index buffer
    u8 IndexSize; //The size in bytes of each index. Typically 16 (a ushort)
    PrimitiveTopology PrimitiveType; //Topology of the submesh (e.g. triangle strip)
    u16 NumBlocks;

    void Read(BinaryReader& data)
    {
        NumIndices = data.ReadUint32();
        IndicesOffset = data.ReadUint32();
        IndexSize = data.ReadChar();
        PrimitiveType = (PrimitiveTopology)data.ReadChar();
        NumBlocks = data.ReadUint16();
    }
};