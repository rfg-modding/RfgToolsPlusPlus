#pragma once
#include "common/Typedefs.h"
#include "MeshEnums.h"
#include <BinaryTools/BinaryReader.h>

class IndexBufferData
{
public:
    u32 NumIndices;
    u32 IndicesOffset;
    u8 IndexSize;
    PrimitiveTopology PrimitiveType;
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