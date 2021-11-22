#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//3D vector zone object property. Commonly used to represent a position.
class Vec3Property : public IZoneProperty
{
public:
    Vec3 Data;

#pragma warning(disable:4100)
    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
        DataType = ZonePropertyType::Vec3;

        Data.x = reader.ReadFloat();
        Data.y = reader.ReadFloat();
        Data.z = reader.ReadFloat();
        return true;
    }
#pragma warning(default:4100)
};