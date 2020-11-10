#pragma once
#include "common/Typedefs.h"
#include "MaterialData.h"
#include "TextureDesc.h"
#include "MaterialConstant.h"
#include <BinaryTools/BinaryReader.h>

class MaterialBlock
{
public:
    //Materials
    std::vector<MaterialData> Materials = {};
    
    //Material texture descriptions
    std::vector<TextureDesc> TextureDescs = {};
    
    //Constant name checksums
    std::vector<u32> ConstantNameChecksums = {};
    
    //Constant values
    std::vector<MaterialConstant> Constants = {};

    void Read(BinaryReader& data)
    {
        //material map data
        u32 unk1 = data.ReadUint32();
        u32 numMaterials = data.ReadUint32();
        u32 unk2 = data.ReadUint32();
        data.Skip(4);
        u32 maybeFirstMaterialOffset = data.ReadUint32();
        data.Align(16);

        //material data
        //Todo: Add support for multiple materials, need to find a file with multiple to understand the layout
        if (numMaterials != 1)
            throw std::runtime_error("Error! Meshes with > 1 material are unsupported. Show this to the maintainer so they can add support.");

        u32 unk3 = data.ReadUint32();
        MaterialData& material = Materials.emplace_back();
        material.Read(data);

        //Read material texture descs
        for (auto& material : Materials)
        {
            for (int i = 0; i < material.NumTextures; i++)
            {
                TextureDesc& desc = TextureDescs.emplace_back();
                desc.Read(data);
            }
        }
        //Todo: Check if need to align data again here

        //Read constant name checksums //Todo: Attempt to get the strings which generated these
        for (auto& material : Materials)
        {
            for (int i = 0; i < material.NumConstants; i++)
            {
                ConstantNameChecksums.push_back(data.ReadUint32());
            }
        }
        data.Align(16);

        //Read material constant values //Todo: Interpret what the values actually mean
        for (auto& material : Materials)
        {
            for (int i = 0; i < material.MaxConstants; i++)
            {
                MaterialConstant& constant = Constants.emplace_back();
                constant.Read(data);
                //Todo: Only read num_constants then skip the rest
            }
        }
        //Todo: Check if need to align data again here
        //data.Align(16);
    }
};