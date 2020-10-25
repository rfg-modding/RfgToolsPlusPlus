#pragma once
#include "common/Typedefs.h"

enum PegFormat : u16
{
    None = 0,
    BM_1555 = 1,
    BM_888 = 2,
    BM_8888 = 3,
    PS2_PAL4 = 200,
    PS2_PAL8 = 201,
    PS2_MPEG32 = 202,
    PC_DXT1 = 400,
    PC_DXT3 = 401,
    PC_DXT5 = 402,
    PC_565 = 403,
    PC_1555 = 404,
    PC_4444 = 405,
    PC_888 = 406,
    PC_8888 = 407,
    PC_16_DUDV = 408,
    PC_16_DOT3_COMPRESSED = 409,
    PC_A8 = 410,
    XBOX2_DXN = 600,
    XBOX2_DXT3A = 601,
    XBOX2_DXT5A = 602,
    XBOX2_CTX1 = 603,
    PS3_DXT5N = 700,
}