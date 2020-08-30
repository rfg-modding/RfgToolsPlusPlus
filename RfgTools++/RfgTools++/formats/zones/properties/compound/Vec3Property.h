#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

class Vec3Property : public IZoneProperty
{
public:
    Vec3 Data;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        DataType = ZonePropertyType::Vec3;

        Data.x = reader.ReadFloat();
        Data.y = reader.ReadFloat();
        Data.z = reader.ReadFloat();
        return true;
    }
};