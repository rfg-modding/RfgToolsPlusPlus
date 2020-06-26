#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>

class StringProperty : public IZoneProperty
{
public:
    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        return false;
    }
};