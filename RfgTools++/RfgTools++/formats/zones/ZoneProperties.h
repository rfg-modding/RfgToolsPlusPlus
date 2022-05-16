#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "types/Mat3.h"

/* This file has some data structures stored in RFG zone object properties */

//Used by DistrictFlags properties
enum class DistrictFlags : u8
{
    None = 0,
    AllowCough = 1,
    AllowAmbEdfCivilianDump = 2,
    PlayCapstoneUnlockedLines = 4,
    DisableMoraleChange = 8,
    DisableControlChange = 16,
};

//For path_road_struct property. Actual data stored in path_road_data buffer property
struct RoadSplineHeader
{
    i32 NumPoints;
    u32 PointsOffset;
    i32 NumConnections;
    u32 ConnectionsOffset;
};

//navpoint_data property
enum class NavpointBranchType : i32
{
    None = 0,
    Start = 1,
    Bridge = 2,
    End = 3
};

struct NavpointData
{
    i32 Version;
    i32 Type;
    bool DontFollowRoad;
    f32 Radius;
    f32 SpeedLimit;
    bool IgnoreLanes;
    NavpointBranchType BranchType;
};
static_assert(sizeof(NavpointData) == 28, "sizeof(NavpointData) must be 28 bytes for zone serialization to work");

//covernode_data property
struct CovernodeData
{
    i32 Version;
    f32 Heading;
    i8 DefaultAngleLeft;
    i8 DefaultAngleRight;
    i8 AngleLeft;
    i8 AngleRight;
    u16 Flags;
};
static_assert(sizeof(CovernodeData) == 16, "sizeof(CovernodeData) must be 16 bytes for zone serialization to work");

//Physics constraint data
enum class ConstraintType : u32
{
    None = 0xFFFFFFFF,
    Point = 0,
    Hinge = 1,
    Prismatic = 2,
    Ragdoll = 3,
    Motor = 4,
    Fake = 5
};

struct PointConstraintData
{
    i32 Type;
    f32 xLimitMin;
    f32 xLimitMax;
    f32 yLimitMin;
    f32 yLimitMax;
    f32 zLimitMin;
    f32 zLimitMax;
    f32 StiffSpringLength;
};
constexpr size_t a3 = sizeof(PointConstraintData);

struct HingeConstraintData
{
    i32 Limited;
    f32 LimitMinAngle;
    f32 LimitMaxAngle;
    f32 LimitFriction;
};

struct PrismaticConstraintData
{
    i32 Limited;
    f32 LimitMinAngle;
    f32 LimitMaxAngle;
    f32 LimitFriction;
};

struct RagdollConstraintData
{
    f32 TwistMin;
    f32 TwistMax;
    f32 ConeMin;
    f32 ConeMax;
    f32 PlaneMin;
    f32 PlaneMax;
};

struct MotorConstraintData
{
    f32 AngularSpeed;
    f32 Gain;
    i32 Axis;
    f32 AxisInBodySpaceX;
    f32 AxisInBodySpaceY;
    f32 AxisInBodySpaceZ;
};

struct FakeConstraintData
{

};

union ConstraintTemplateData
{
    PointConstraintData Point;
    HingeConstraintData Hinge;
    PrismaticConstraintData Prismatic;
    RagdollConstraintData Ragdoll;
    MotorConstraintData Motor;
    FakeConstraintData Fake;
};

struct ConstraintTemplate
{
    ConstraintType Type;

    //Body 1
    u32 Unused0;
    u32 Unused1;
    u32 Body1Index;
    Mat3 Body1Orient;
    Vec3 Body1Pos;

    //Body 2
    u32 Unused2;
    u32 Body2Index;
    Mat3 Body2Orient;
    Vec3 Body2Pos;

    f32 Threshold;
    ConstraintTemplateData Data;
};

static_assert(sizeof(ConstraintTemplate) == 156, "ConstraintTemplate must be 156 bytes for zone property serialization to work correctly.");