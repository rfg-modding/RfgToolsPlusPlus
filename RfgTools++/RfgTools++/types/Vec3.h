#pragma once
#include "common/Typedefs.h"

struct Vec3
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;

    Vec3 operator-(const Vec3& B)
    {
        return Vec3{x - B.x, y - B.y, z - B.z};
    }
};