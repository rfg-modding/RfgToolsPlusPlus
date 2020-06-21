#pragma once
#include "common/Typedefs.h"
#include "Vec3.h"
#include "Mat3.h"

struct Mat4x3
{
    Mat3 Rotation;
    Vec3 Translation;
};