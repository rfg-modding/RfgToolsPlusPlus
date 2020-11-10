#pragma once
#include "common/Typedefs.h"
#include "MeshEnums.h"
#include <BinaryTools/BinaryReader.h>

class VertexBufferData
{
public:
    u32 NumVerts;
    u8 VertexStride0;
    VertexFormat Format;
    u8 NumUvChannels;
    u8 VertexStride1;
    u32 VertexOffset; //Seems to be a pointer set at runtime

    void Read(BinaryReader& data)
    {
        NumVerts = data.ReadUint32();
        VertexStride0 = data.ReadChar();
        Format = (VertexFormat)data.ReadChar();
        NumUvChannels = data.ReadChar();
        VertexStride1 = data.ReadChar();
        VertexOffset = data.ReadUint32();
    }
};