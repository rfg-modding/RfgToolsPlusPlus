#pragma once
#include "common/Typedefs.h"

enum class TextureFlags : u16
{
    None = 0, // 0
    Unknown0 = 1 << 0, // 1
    Unknown1 = 1 << 1, // 2
    Unknown2 = 1 << 2, // 4
    CubeTexture = 1 << 3, // 8
    Unknown4 = 1 << 4, // 16
    Unknown5 = 1 << 5, // 32
    Unknown6 = 1 << 6, // 64
    Unknown7 = 1 << 7, // 128
    Unknown8 = 1 << 8, // 256
    Unknown9 = 1 << 9, // 512
    Unknown10 = 1 << 10, // 1024
    Unknown11 = 1 << 11, // 2048
    Unknown12 = 1 << 12, // 4096
    Unknown13 = 1 << 13, // 8192
    Unknown14 = 1 << 14, // 16384
    Unknown15 = 1 << 15, // 32768
};