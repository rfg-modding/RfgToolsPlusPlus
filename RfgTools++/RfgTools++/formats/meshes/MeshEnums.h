#pragma once
#include "common/Typedefs.h"

enum class PrimitiveTopology : u8
{
    TriangleStrip = 0, //internally D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
    TriangleList = 1, //internally D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST
};

enum class VertexFormat : u8
{
    Pixlit = 0,
    PixlitCa = 1,
    PixlitNmap = 2,
    PixlitNmapCa = 3,
    Unlit = 4,
    ParticlePointsprite = 5,
    ParticleBillboard = 6,
    ParticleRadial = 7,
    ParticleDrop = 8,
    ParticleRibbon = 9,
    ParticleOriented = 10,
    Primitive3D = 11,
    Primitive2D = 12,
    SgMesh = 13,
    HeightMesh = 14,
    HeightMeshLowLod = 15,
    ParticleParametric = 16,
    Compositor = 17,
    CloneUvs = 18,
    CloneNmap = 19,
    CloneClr = 20,
    Spline2D = 21,
    ParticleCorona = 22,
    ParticleRibbonParametric = 23,
    ConditionalBbox = 24,
    TerrainRoad = 25,
    HeightMeshLandmarkLod = 26,
    StarFieldPoint = 27,
    StarFieldBillboard = 28,
    MeteorShowerLine = 29,
    Pixlit0Uv = 30,
    Pixlit1Uv = 31,
    Pixlit1UvCa = 32,
    Pixlit1UvNmap = 33,
    Pixlit1UvNmapCa = 34,
    Pixlit2Uv = 35,
    Pixlit2UvCa = 36,
    Pixlit2UvNmap = 37,
    Pixlit2UvNmapCa = 38,
    Pixlit3Uv = 39,
    Pixlit3UvCa = 40,
    Pixlit3UvNmap = 41,
    Pixlit3UvNmapCa = 42,
    Pixlit4Uv = 43,
    Pixlit4UvCa = 44,
    Pixlit4UvNmap = 45,
    Pixlit4UvNmapCa = 46,
    Clone1UvUvs = 47,
    Clone2UvUvs = 48,
    UncompressedMorph = 49,
    Invalid = 255
};

string to_string(PrimitiveTopology value);
string to_string(VertexFormat value);