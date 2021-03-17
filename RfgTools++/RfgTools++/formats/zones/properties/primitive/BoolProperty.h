#pragma once
#include "common/Typedefs.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//Boolean zone object property. Possible values: True, False
class BoolProperty : public IZoneProperty
{
public:
    bool Data;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
        DataType = ZonePropertyType::Bool;
        Data = (bool)reader.ReadUint8(); //Bool stored as a single byte in binary file
        return true;
    }
};