#pragma once
#include "common/Typedefs.h"
#include "Vec2.h"
#include <sstream>

//Todo: Put this in a utility namespace
//Converts a value to a string with specific precision. Allows control of how many digits are present in the converted string
template <typename T>
string to_string_precise(const T Value, const int Precision = 1)
{
    std::ostringstream Out;
    Out.precision(Precision);
    Out << std::fixed << Value;
    return Out.str();
}

struct Vec3
{
    Vec3() {}
    Vec3(f32 x_, f32 y_, f32 z_) : x(x_), y(y_), z(z_) {}

    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;

    Vec3 operator-(const Vec3& B) const
    {
        return Vec3{x - B.x, y - B.y, z - B.z};
    }

    Vec3 operator+(const Vec3& B) const
    {
        return Vec3{ x + B.x, y + B.y, z + B.z };
    }

    Vec3 operator+=(const Vec3& B)
    {
        *this = *this + B;
        return *this;
    }

    Vec3 operator/(f32 scalar) const
    {
        return Vec3{ x / scalar, y / scalar, z / scalar };
    }

    Vec3 operator*(f32 scalar) const
    {
        return Vec3{ x * scalar, y * scalar, z * scalar };
    }

    f32 Distance(const Vec3& B) const
    {
        return sqrtf(powf(B.x - x, 2.0f) + powf(B.y - y, 2.0f) + powf(B.z - z, 2.0f));
    }

    Vec3 Cross(const Vec3& B) const
    {
        Vec3 C;
        C.x = (y * B.z) - (z * B.y);
        C.y = (z * B.x) - (x * B.z);
        C.z = (x * B.y) - (y * B.x);
        return C;
    }

    f32 Magnitude() const
    {
        return sqrtf((x * x) + (y * y) + (z * z));
    }

    Vec3 Normalize() const
    {
        return *this / Magnitude();
    }

    string String() const
    {
        return "{"
                    + to_string_precise(x, 2) + ", "
                    + to_string_precise(y, 2) + ", "
                    + to_string_precise(z, 2) +
                "}";
    }

    //Swizzle functions
    Vec2 XY() { return Vec2{ x, y }; }
    Vec2 XZ() { return Vec2{ x, z }; }
};