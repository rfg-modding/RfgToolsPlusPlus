#pragma once
#include "common/Typedefs.h"
#include "RfgTools++/types/Vec3.h"

class SubmeshData
{
public:
    u32 NumRenderBlocks = 0;
    Vec3 Offset = Vec3{ 0.0f, 0.0f, 0.0f };
    Vec3 Bmin = Vec3{ 0.0f, 0.0f, 0.0f };
    Vec3 Bmax = Vec3{ 0.0f, 0.0f, 0.0f };
    u32 RenderBlocksOffset = 0; //Todo: Figure out if we care about this data. May be runtime offset
};

//This is mostly an alarm bell for if the size of this struct changes. It shouldn't unless the game gets updated or modders change how mesh loading works
static_assert(sizeof(SubmeshData) == 44,
    "Error. sizeof(SubmeshData) must equal 44 bytes for its serialization code to work since it reads the file data directly onto the struct in memory without any parsing.");