#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "types/Mat3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//Orient + position zone object property. Has an objects position and orientation combined into one property.
class OpProperty : public IZoneProperty
{
public:
    Vec3 Position;
    Mat3 Orient;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
        DataType = ZonePropertyType::Op;

        //Read position
        Position.x = reader.ReadFloat();
        Position.y = reader.ReadFloat();
        Position.z = reader.ReadFloat();

        //Read orient
        Orient.rvec.x = reader.ReadFloat();
        Orient.rvec.y = reader.ReadFloat();
        Orient.rvec.z = reader.ReadFloat();

        Orient.uvec.x = reader.ReadFloat();
        Orient.uvec.y = reader.ReadFloat();
        Orient.uvec.z = reader.ReadFloat();

        Orient.fvec.x = reader.ReadFloat();
        Orient.fvec.y = reader.ReadFloat();
        Orient.fvec.z = reader.ReadFloat();

        return true;
    }
};