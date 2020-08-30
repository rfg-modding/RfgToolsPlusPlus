#pragma once
#include "common/Typedefs.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

class FloatProperty : public IZoneProperty
{
public:
    f32 Data;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        DataType = ZonePropertyType::Float;
        Data = reader.ReadFloat();
        return true;
    }
};