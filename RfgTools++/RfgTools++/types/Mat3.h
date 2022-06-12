#pragma once
#include "common/Typedefs.h"
#include "Vec3.h"

struct Mat3
{
    Mat3() {}
    Mat3(const Vec3& r, const Vec3& u, const Vec3& f) : rvec(r), uvec(u), fvec(f) {}

    Vec3 rvec = { 1.0f, 0.0f, 0.0f };
    Vec3 uvec = { 0.0f, 1.0f, 0.0f };
    Vec3 fvec = { 0.0f, 0.0f, 1.0f };

    Mat3 operator*(const Mat3& b) const
    {
        Mat3 a = *this;
        Mat3 out = {};
        out.rvec =
        {
            (a.rvec.x * b.rvec.x) + (a.rvec.y * b.uvec.x) + (a.rvec.z * b.fvec.x),
            (a.rvec.x * b.rvec.y) + (a.rvec.y * b.uvec.y) + (a.rvec.z * b.fvec.y),
            (a.rvec.x * b.rvec.z) + (a.rvec.y * b.uvec.z) + (a.rvec.z * b.fvec.z)
        };
        out.uvec =
        {
            (a.uvec.x * b.rvec.x) + (a.uvec.y * b.uvec.x) + (a.uvec.z * b.fvec.x),
            (a.uvec.x * b.rvec.y) + (a.uvec.y * b.uvec.y) + (a.uvec.z * b.fvec.y),
            (a.uvec.x * b.rvec.z) + (a.uvec.y * b.uvec.z) + (a.uvec.z * b.fvec.z),
        };
        out.fvec =
        {
            (a.fvec.x * b.rvec.x) + (a.fvec.y * b.uvec.x) + (a.fvec.z * b.fvec.x),
            (a.fvec.x * b.rvec.y) + (a.fvec.y * b.uvec.y) + (a.fvec.z * b.fvec.y),
            (a.fvec.x * b.rvec.z) + (a.fvec.y * b.uvec.z) + (a.fvec.z * b.fvec.z),
        };
        return out;
    }
};