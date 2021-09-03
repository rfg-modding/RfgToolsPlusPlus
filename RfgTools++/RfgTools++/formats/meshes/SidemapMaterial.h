#pragma once
#include "common/Typedefs.h"
#include "RfgMaterial.h"
#include <string>
#include <vector>

struct SidemapMaterial
{
    std::vector<string> MaterialNamesList;
    RfgMaterial Material;

    void Read(BinaryReader& cpuFile)
    {
        u32 materialNamesListSize = cpuFile.ReadUint32();
        MaterialNamesList = cpuFile.ReadSizedStringList(materialNamesListSize);
        cpuFile.Align(16);
        Material.Read(cpuFile);
    }
};
