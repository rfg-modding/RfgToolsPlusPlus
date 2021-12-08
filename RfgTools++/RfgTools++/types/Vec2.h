#pragma once
#include "common/Typedefs.h"

struct Vec2
{
    f32 x = 0.0f;
    f32 y = 0.0f;

    Vec2 operator-(const Vec2& B)
    {
        return Vec2{ x - B.x, y - B.y };
    }

    Vec2 operator+(const Vec2& B)
    {
        return Vec2{ x + B.x, y + B.y };
    }

    Vec2 operator/(f32 scalar)
    {
        return Vec2{ x / scalar, y / scalar };
    }

    Vec2 operator*(f32 scalar)
    {
        return Vec2{ x * scalar, y * scalar };
    }

    Vec2 operator+=(const Vec2& B)
    {
        *this = *this + B;
        return *this;
    }

    Vec2 operator-=(const Vec2& B)
    {
        *this = *this - B;
        return *this;
    }

    f32 Distance(const Vec2& B) const
    {
        return sqrtf(powf(B.x - x, 2.0f) + powf(B.y - y, 2.0f));
    }

    f32 Magnitude()
    {
        return sqrtf((x * x) + (y * y));
    }

    Vec2 Normalize()
    {
        return *this / Magnitude();
    }
};