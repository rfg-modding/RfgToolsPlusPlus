#pragma once
#include "common/Typedefs.h"
#include <unordered_map>
#include <optional>
#include <vector>

//Mesh topologies used by RFG(R) meshes
enum class PrimitiveTopology : u8
{
    TriangleStrip = 0, //internally D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
    TriangleList = 1, //internally D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST
};

//Vertex formats used by RFG(R) meshes
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

//Data types of vertex components (e.g. position, uv, normal)
enum VertexComponentFormat
{
    F32_3, //3 32bit floats
    U8_4_UNORM, //4 8bit values which are converted to float then normalized in the shader
    U8_4_UINT, //4 8bit uints
    I16_2 //2 16bit ints
};

static u32 VertexComponentSize(VertexComponentFormat format)
{
    switch (format)
    {
    case F32_3:
        return 12;
    case U8_4_UNORM:
    case U8_4_UINT:
    case I16_2:
        return 4;
    default:
        return 0;
    }
}

//Data layout description for vertex components (e.g. position, uv)
struct VertexComponentLayout
{
    const std::string_view Name; //Component type (POSITION, NORMAL, etc). Should be equivalent to gltf component names for easy conversion.
    const VertexComponentFormat Format; //Data format
    const u32 Offset; //Component offset in bytes relative to the start of the vertex

    VertexComponentLayout(std::string_view name, VertexComponentFormat format, u32 offset)
        : Name(name), Format(format), Offset(offset)
    {

    }
};

//Data layout of a vertex format. See VertexFormat enum for all formats.
struct VertexFormatLayout
{
    const std::vector<VertexComponentLayout> Components;

    VertexFormatLayout(const std::vector<VertexComponentLayout> components) : Components(components)
    {

    }

    //Size of a vertex using this data layout
    u32 Stride()
    {
        u32 stride = 0;
        for (auto& comp : Components)
            stride += VertexComponentSize(comp.Format);

        return stride;
    }
};

string to_string(PrimitiveTopology value);
string to_string(VertexFormat value);

//Layout of RFG mesh vertex formats. Used by gltf exporter. Necessary to convert RFG vertex formats to GLTF compatible format.
//Note: Most formats not yet supported. They're added as they're needed.
static std::unordered_map<VertexFormat, VertexFormatLayout> VertexFormatLayouts
{
    {
        VertexFormat::Pixlit1Uv,
        {
            {
                { "POSITION",   F32_3,      0  },
                { "NORMAL",     U8_4_UNORM, 12 },
                { "TEXCOORD_0", I16_2,      16 }
            }
        }
    },
    {
        VertexFormat::Pixlit1UvNmap,
        {
            {
                { "POSITION",   F32_3,      0  },
                { "NORMAL",     U8_4_UNORM, 12 },
                { "TANGENT",    U8_4_UNORM, 16 },
                { "TEXCOORD_0", I16_2,      20 }
            }
        }
    },
    {
        VertexFormat::Pixlit1UvNmapCa,
        {
            {
                { "POSITION",    F32_3,      0  },
                { "NORMAL",      U8_4_UNORM, 12 },
                { "TANGENT",     U8_4_UNORM, 16 },
                { "BLENDWEIGHT", U8_4_UNORM, 20 },
                { "BLENDINDEX",  U8_4_UINT,  24 },
                { "TEXCOORD_0",  I16_2,      28 }
            }
        }
    },
    {
        VertexFormat::Pixlit2UvNmap,
        {
            {
                { "POSITION",    F32_3,      0  },
                { "NORMAL",      U8_4_UNORM, 12 },
                { "TANGENT",     U8_4_UNORM, 16 },
                { "TEXCOORD_0",  I16_2,      20 },
                { "TEXCOORD_1",  I16_2,      24 }
            }
        }
    },
    {
        VertexFormat::Pixlit3UvNmap,
        {
            {
                { "POSITION",    F32_3,      0  },
                { "NORMAL",      U8_4_UNORM, 12 },
                { "TANGENT",     U8_4_UNORM, 16 },
                { "TEXCOORD_0",  I16_2,      20 },
                { "TEXCOORD_1",  I16_2,      24 },
                { "TEXCOORD_2",  I16_2,      28 }
            }
        }
    },
    {
        VertexFormat::Pixlit3UvNmapCa,
        {
            {
                { "POSITION",    F32_3,      0  },
                { "NORMAL",      U8_4_UNORM, 12 },
                { "TANGENT",     U8_4_UNORM, 16 },
                { "BLENDWEIGHT", U8_4_UNORM, 20 },
                { "BLENDINDEX",  U8_4_UINT,  24 },
                { "TEXCOORD_0",  I16_2,      28 },
                { "TEXCOORD_1",  I16_2,      32 },
                { "TEXCOORD_2",  I16_2,      36 }
            }
        }
    }
};

static std::optional<VertexFormatLayout> GetVertexFormatLayout(VertexFormat format)
{
    auto search = VertexFormatLayouts.find(format);
    if (search == VertexFormatLayouts.end())
        return {}; //Not found
    else
        return search->second; //Found
}