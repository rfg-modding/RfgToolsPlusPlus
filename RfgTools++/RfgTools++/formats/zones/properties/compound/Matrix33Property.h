#pragma once
#include "common/Typedefs.h"
#include "types/Mat3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//3x3 matrix zone object property. Commonly used to represent rotation/orientation.
class Matrix33Property : public IZoneProperty
{
public:
    Mat3 Data;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
        DataType = ZonePropertyType::Matrix33;

        Data.rvec.x = reader.ReadFloat();
        Data.rvec.y = reader.ReadFloat();
        Data.rvec.z = reader.ReadFloat();

        Data.uvec.x = reader.ReadFloat();
        Data.uvec.y = reader.ReadFloat();
        Data.uvec.z = reader.ReadFloat();
        
        Data.fvec.x = reader.ReadFloat();
        Data.fvec.y = reader.ReadFloat();
        Data.fvec.z = reader.ReadFloat();
        
        return true;
    }
};