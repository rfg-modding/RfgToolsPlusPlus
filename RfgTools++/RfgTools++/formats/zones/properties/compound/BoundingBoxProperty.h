#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

class BoundingBoxProperty : public IZoneProperty
{
public:
    Vec3 Min;
    Vec3 Max;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        DataType = ZonePropertyType::BoundingBox;

        //Read min
        Min.x = reader.ReadFloat();
        Min.y = reader.ReadFloat();
        Min.z = reader.ReadFloat();
        //Read max
        Max.x = reader.ReadFloat();
        Max.y = reader.ReadFloat();
        Max.z = reader.ReadFloat();
        return true;
    }
};