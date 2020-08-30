#pragma once
#include "common/Typedefs.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

class BoolProperty : public IZoneProperty
{
public:
    bool Data;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        DataType = ZonePropertyType::Bool;
        Data = (bool)reader.ReadUint8();
        return true;
    }
};