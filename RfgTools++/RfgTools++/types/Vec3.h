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

    Vec3 operator+=(const Vec3& B)
    {
        *this = *this + B;
        return *this;
    }

    Vec3 operator/(f32 scalar)
    {
        return Vec3{ x / scalar, y / scalar, z / scalar };
    }

    f32 Distance(const Vec3& B) const
    {
        return sqrt(pow(B.x - x, 2) + pow(B.y - y, 2) + pow(B.z - z, 2));
    }

    Vec3 Cross(const Vec3& B)
    {
        Vec3 C;
        C.x = (y * B.z) - (B.y * z);
        C.y = (B.x * z) - (y * B.z);
        C.z = (x * B.y) - (B.x * y);
        return C;
    }

    f32 Magnitude()
    {
        return sqrtf((x * x) + (y * y) + (z * z));
    }

    Vec3 Normalize()
    {
        return *this / Magnitude();
    }
};