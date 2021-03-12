#pragma once
#include "common/Typedefs.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//Unsigned 32 bit integer zone object property
class UintProperty : public IZoneProperty
{
public:
    u32 Data;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        DataType = ZonePropertyType::Uint;
        Data = reader.ReadUint32();
        return true;
    }
};