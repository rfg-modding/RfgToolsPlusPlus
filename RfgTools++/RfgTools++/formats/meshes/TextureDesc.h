#pragma once
#include "common/Typedefs.h"
#include "BinaryTools/BinaryReader.h"

class TextureDesc
{
public:
    u32 NameOffset;
    u32 NameChecksum; //Todo: Confirm this is actually a checksum and figure out how to calc & validate it
    u32 TextureIndex;

    public void Read(BinaryReader& data)
    {
        NameOffset = data.ReadUint32();
        NameChecksum = data.ReadUint32();
        TextureIndex = data.ReadUint32();
    }
}