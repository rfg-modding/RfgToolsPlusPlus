#pragma once
#include "common/Typedefs.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

class StringProperty : public IZoneProperty
{
public:
    string Data;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        DataType = ZonePropertyType::String;
        Data = reader.ReadFixedLengthString(size);
        if (reader.PeekChar() == 0)
            reader.Skip(1); //Special case, sometimes strings have a null terminator following 'Size' bytes. Skip null terminator if present

        return true;
    }
};