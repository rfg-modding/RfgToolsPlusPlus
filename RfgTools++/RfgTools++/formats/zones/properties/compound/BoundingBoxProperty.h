#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//Bounding box zone object property
class BoundingBoxProperty : public IZoneProperty
{
public:
    Vec3 Min;
    Vec3 Max;

#pragma warning(disable:4100)
    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
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
#pragma warning(default:4100)
};