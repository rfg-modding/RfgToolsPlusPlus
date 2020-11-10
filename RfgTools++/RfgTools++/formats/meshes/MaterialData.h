#pragma once
#include "common/Typedefs.h"
#include "BinaryTools/BinaryReader.h"

class MaterialData
{
public:
    u32 ShaderHandle;
    u32 NameChecksum;
    u32 MaterialFlags; //Todo: Find meaning of values for this
    u16 NumTextures;
    u8 NumConstants;
    u8 MaxConstants;
    u32 TextureOffset; //Seems to be a ptr set at runtime
    u32 ConstantNameChecksumsOffset; //Seems to be a ptr set at runtime
    u32 ConstantBlockOffset; //Seems to be a ptr set at runtime

    public void Read(BinaryReader& data)
    {
        ShaderHandle = data.ReadUint32();
        NameChecksum = data.ReadUint32();
        MaterialFlags = data.ReadUint32();
        NumTextures = data.ReadUint16();
        NumConstants = data.ReadChar();
        MaxConstants = data.ReadChar();
        TextureOffset = data.ReadUint32();
        ConstantNameChecksumsOffset = data.ReadUint32();
        ConstantBlockOffset = data.ReadUint32();
    }
}