#pragma once
#include "common/Typedefs.h"
#include "BinaryTools/BinaryReader.h"

class RenderBlock
{
public:
    u16 MaterialMapIndex = 0;
    u16 padding0 = 0; //2 bytes padding present in file
    u32 StartIndex = 0;
    u32 NumIndices = 0;
    u32 MinIndex = 0;
    u32 MaxIndex = 0;

    void Read(BinaryReader& in)
    {
        in.ReadToMemory(this, sizeof(RenderBlock));
    }
};

//This is mostly an alarm bell for if the size of this struct changes. It shouldn't unless the game gets updated or modders change how mesh loading works
static_assert(sizeof(RenderBlock) == 20,
    "Error. sizeof(RenderBlock) must equal 20 bytes for its serialization code to work since it reads the file data directly onto the struct in memory without any parsing.");