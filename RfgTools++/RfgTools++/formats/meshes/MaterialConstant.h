#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>

class MaterialConstant
{
public:
    f32 Constants[4];
    
    void Read(BinaryReader& data)
    {
        data.ReadToMemory(Constants, sizeof(f32) * 4);
    }
};