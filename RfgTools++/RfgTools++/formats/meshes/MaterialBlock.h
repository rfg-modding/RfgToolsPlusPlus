#pragma once
#include "common/Typedefs.h"
#include "TextureDesc.h"
#include "MaterialConstant.h"
#include "RfgMaterial.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>

class MaterialBlock
{
public:
    //Material map
    u32 MaterialsOffsetRelative; //Offset to material data relative to the material map
    u32 NumMaterials; //Number of materials
    std::vector<u32> MaterialOffsets; //Offset of each material in the cpu file

    //Materials
    std::vector<RfgMaterial> Materials = {};

    void Read(BinaryReader& data, u32 materialListOffset)
    {
        //Material map data
        u32 materialMapOffset = data.Position(); //Todo: Could this be alignment?
        MaterialsOffsetRelative = data.ReadUint32();
        NumMaterials = data.ReadUint32();

        //Seek to material offset list
        data.SeekBeg(materialListOffset);

        //Read list of material offsets
        for (u32 i = 0; i < NumMaterials; i++)
        {
            MaterialOffsets.push_back(data.ReadUint32());
            data.Skip(4);
        }

        //Read materials
        for (u32 i = 0; i < NumMaterials; i++)
        {
            //Seek to start of next material
            data.SeekBeg(MaterialOffsets[i]);

            //Read material data
            RfgMaterial& material = Materials.emplace_back();
            material.Read(data);
        }
    }
};