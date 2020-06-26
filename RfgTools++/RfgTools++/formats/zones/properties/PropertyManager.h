#pragma once
#include "common/Typedefs.h"
#include "hashes/Hash.h"
#include "RfgTools++/formats/zones/ZoneObject36.h"
#include <BinaryTools/BinaryReader.h>
#include <unordered_map>
#include <functional>

class IZoneProperty;

class PropertyManager
{
public:
    static void ReadObjectProperties(ZoneObject36& object, BinaryReader& reader);

private:
    static void Init();

    //Zone property definitions. Matches name string hashes (the u32 key) to a function which tries to read that properties data 
    //and returns a pointer to the concrete implementation of that property. Will need to be modified to support writing properties back to files.
    static std::unordered_map<u32, std::function<IZoneProperty*(BinaryReader& reader, u16 type, u16 size, u32 nameHash)>> propertyTypes4_;
    static std::unordered_map<u32, std::function<IZoneProperty*(BinaryReader& reader, u16 type, u16 size, u32 nameHash)>> propertyTypes5_;
    static std::unordered_map<u32, std::function<IZoneProperty*(BinaryReader& reader, u16 type, u16 size, u32 nameHash)>> propertyTypes6_;
    static bool initialized_;
};