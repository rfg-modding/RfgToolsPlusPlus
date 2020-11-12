#include "MeshEnums.h"

string to_string(PrimitiveTopology value)
{
    switch (value)
    {
    case PrimitiveTopology::TriangleStrip:
        return "TriangleStrip";
    case PrimitiveTopology::TriangleList:
        return "TriangleList";
    default:
        return "InvalidEnum";
    }
}

string to_string(VertexFormat value)
{
    switch (value)
    {
    case VertexFormat::Pixlit:
        return "Pixlit";
    case VertexFormat::PixlitCa:
        return "PixlitCa";
    case VertexFormat::PixlitNmap:
        return "PixlitNmap";
    case VertexFormat::PixlitNmapCa:
        return "PixlitNmapCa";
    case VertexFormat::Unlit:
        return "Unlit";
    case VertexFormat::ParticlePointsprite:
        return "ParticlePointsprite";
    case VertexFormat::ParticleBillboard:
        return "ParticleBillboard";
    case VertexFormat::ParticleRadial:
        return "ParticleRadial";
    case VertexFormat::ParticleDrop:
        return "ParticleDrop";
    case VertexFormat::ParticleRibbon:
        return "ParticleRibbon";
    case VertexFormat::ParticleOriented:
        return "ParticleOriented";
    case VertexFormat::Primitive3D:
        return "Primitive3D";
    case VertexFormat::Primitive2D:
        return "Primitive2D";
    case VertexFormat::SgMesh:
        return "SgMesh";
    case VertexFormat::HeightMesh:
        return "HeightMesh";
    case VertexFormat::HeightMeshLowLod:
        return "HeightMeshLowLod";
    case VertexFormat::ParticleParametric:
        return "ParticleParametric";
    case VertexFormat::Compositor:
        return "Compositor";
    case VertexFormat::CloneUvs:
        return "CloneUvs";
    case VertexFormat::CloneNmap:
        return "CloneNmap";
    case VertexFormat::CloneClr:
        return "CloneClr";
    case VertexFormat::Spline2D:
        return "Spline2D";
    case VertexFormat::ParticleCorona:
        return "ParticleCorona";
    case VertexFormat::ParticleRibbonParametric:
        return "ParticleRibbonParametric";
    case VertexFormat::ConditionalBbox:
        return "ConditionalBbox";
    case VertexFormat::TerrainRoad:
        return "TerrainRoad";
    case VertexFormat::HeightMeshLandmarkLod:
        return "HeightMeshLandmarkLod";
    case VertexFormat::StarFieldPoint:
        return "StarFieldPoint";
    case VertexFormat::StarFieldBillboard:
        return "StarFieldBillboard";
    case VertexFormat::MeteorShowerLine:
        return "MeteorShowerLine";
    case VertexFormat::Pixlit0Uv:
        return "Pixlit0Uv";
    case VertexFormat::Pixlit1Uv:
        return "Pixlit1Uv";
    case VertexFormat::Pixlit1UvCa:
        return "Pixlit1Uv";
    case VertexFormat::Pixlit1UvNmap:
        return "Pixlit1UvNmap";
    case VertexFormat::Pixlit1UvNmapCa:
        return "Pixlit1UvNmapCa";
    case VertexFormat::Pixlit2Uv:
        return "Pixlit2Uv";
    case VertexFormat::Pixlit2UvCa:
        return "Pixlit2UvCa";
    case VertexFormat::Pixlit2UvNmap:
        return "Pixlit2UvNmap";
    case VertexFormat::Pixlit2UvNmapCa:
        return "Pixlit2UvNmapCa";
    case VertexFormat::Pixlit3Uv:
        return "Pixlit3Uv";
    case VertexFormat::Pixlit3UvCa:
        return "Pixlit3UvCa";
    case VertexFormat::Pixlit3UvNmap:
        return "Pixlit3UvNmap";
    case VertexFormat::Pixlit3UvNmapCa:
        return "Pixlit3UvNmapCa";
    case VertexFormat::Pixlit4Uv:
        return "Pixlit4Uv";
    case VertexFormat::Pixlit4UvCa:
        return "Pixlit4UvCa";
    case VertexFormat::Pixlit4UvNmap:
        return "Pixlit4UvNmap";
    case VertexFormat::Pixlit4UvNmapCa:
        return "Pixlit4UvNmapCa";
    case VertexFormat::Clone1UvUvs:
        return "Clone1UvUvs";
    case VertexFormat::Clone2UvUvs:
        return "Clone2UvUvs";
    case VertexFormat::UncompressedMorph:
        return "UncompressedMorph";
    case VertexFormat::Invalid:
        return "Invalid";
    default:
        return "InvalidEnum";
    }
}