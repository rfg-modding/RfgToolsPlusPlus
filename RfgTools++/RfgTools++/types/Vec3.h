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

    Vec3 operator+(const Vec3& B)
    {
        return Vec3{ x + B.x, y + B.y, z + B.z };
    }

    Vec3 operator/(f32 scalar) const
    {
        return Vec3{ x / scalar, y / scalar, z / scalar };
    }

    f32 Distance(const Vec3& B) const
    {
        return sqrt(pow(B.x - x, 2) + pow(B.y - y, 2) + pow(B.z - z, 2));
    }

    Vec3 Normalized() const
    {
        return *this / Distance(*this);
    }
};