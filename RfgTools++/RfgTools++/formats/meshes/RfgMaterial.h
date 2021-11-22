#pragma once
#include "common/Typedefs.h"
#include "TextureDesc.h"
#include "MaterialConstant.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>
#include <exception>

class RfgMaterial
{
public:
    //General data
    u32 ShaderHandle = 0;
    u32 NameChecksum = 0;
    u32 MaterialFlags = 0; //Todo: Find meaning of flags
    u16 NumTextures = 0;
    u8 NumConstants = 0;
    u8 MaxConstants = 0;
    u32 TextureOffset = 0; //Seems to be a ptr set at runtime
    u32 ConstantNameChecksumsOffset = 0; //Seems to be a ptr set at runtime
    u32 ConstantBlockOffset = 0; //Seems to be a ptr set at runtime

    //Material texture descriptions
    std::vector<TextureDesc> TextureDescs = {};

    //Constant name checksums
    std::vector<u32> ConstantNameChecksums = {};

    //Constant values
    std::vector<MaterialConstant> Constants = {};

    void Read(BinaryReader& data)
    {
        //Read material data size from file. Used for validation once done reading material data
        u32 materialDataStart = (u32)data.Position();
        u32 materialDataSize = data.ReadUint32();

        //Read general data
        ShaderHandle = data.ReadUint32();
        NameChecksum = data.ReadUint32();
        MaterialFlags = data.ReadUint32();
        NumTextures = data.ReadUint16();
        NumConstants = data.ReadChar();
        MaxConstants = data.ReadChar();
        TextureOffset = data.ReadUint32();
        ConstantNameChecksumsOffset = data.ReadUint32();
        ConstantBlockOffset = data.ReadUint32();

        //Read texture descriptions
        for (int i = 0; i < NumTextures; i++)
        {
            TextureDesc& desc = TextureDescs.emplace_back();
            desc.Read(data);
        }

        //Read constant name checksums
        for (int i = 0; i < NumConstants; i++)
        {
            ConstantNameChecksums.push_back(data.ReadUint32());
        }
        data.Align(16);

        //Read material constant values
        for (int i = 0; i < MaxConstants; i++)
        {
            MaterialConstant& constant = Constants.emplace_back();
            constant.Read(data);
        }
        data.Align(16);

        //Validate data read by checking if we read the amount of data we expected to
        if (data.Position() != size_t(materialDataStart) + size_t(materialDataSize))
            throw std::runtime_error("Material data block read failed. Unexpected size. Expected it to be " + std::to_string(materialDataSize) + " bytes. Was really " + std::to_string(data.Position() - materialDataStart) + " bytes");
    }
};