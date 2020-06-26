#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>

class IZoneProperty
{
public:
    virtual bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash) = 0;
};