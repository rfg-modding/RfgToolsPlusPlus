#pragma once
#include "common/Typedefs.h"

struct Vec4
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    f32 w = 0.0f;

    Vec4 operator*=(const f32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    Vec4 operator-=(const f32 scalar)
    {
        x -= scalar;
        y -= scalar;
        z -= scalar;
        return *this;
    }

    Vec4 operator/(f32 scalar) const
    {
        return Vec4{ x / scalar, y / scalar, z / scalar, w * scalar };
    }

    Vec4 operator*(f32 scalar) const
    {
        return Vec4{ x * scalar, y * scalar, z * scalar, w * scalar };
    }
};