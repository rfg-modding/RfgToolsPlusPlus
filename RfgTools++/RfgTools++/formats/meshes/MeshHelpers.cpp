#include "MeshHelpers.h"

bool MeshHelpers::WriteVerticesToObj(std::ofstream& obj, VertexFormat format, std::span<u8> vertexBufferBytes)
{
    if (format == VertexFormat::Pixlit1Uv)
    {
        //Convert vertexBufferBytes to span of vertex type to access vertex data
        struct Vertex_Pixlit1Uv
        {
            f32 posX, posY, posZ;
            u8 normalX, normalY, normalZ, normalW;
            i16 uvX, uvY;
        };
        static_assert(sizeof(Vertex_Pixlit1Uv) == 20);
        std::span<Vertex_Pixlit1Uv> vertices = MeshHelpers::ToTypedSpan<Vertex_Pixlit1Uv>(vertexBufferBytes);

        //Write vertices
        for (auto& vertex : vertices)
            obj << "v " << std::to_string(vertex.posX) << " " << std::to_string(vertex.posY) << " " << std::to_string(vertex.posZ) << "\n";

        //Write uv data
        for (auto& vertex : vertices)
            obj << "vt " << std::to_string(abs((f32)vertex.uvX / 1024.0f)) << " " << std::to_string(abs((f32)vertex.uvY / 1024.0f)) << "\n";

        //Note: Not writing normal data since tools like blender generate them automatically anyway
    }
    else if (format == VertexFormat::Pixlit1UvNmap)
    {
        //Convert vertexBufferBytes to span of vertex type to access vertex data
        struct Vertex_Pixlit1UvNmap
        {
            f32 posX, posY, posZ;
            u8 normalX, normalY, normalZ, normalW;
            u8 tangentX, tangentY, tangentZ, tangentW;
            i16 uvX, uvY;
        };
        static_assert(sizeof(Vertex_Pixlit1UvNmap) == 24);
        std::span<Vertex_Pixlit1UvNmap> vertices = MeshHelpers::ToTypedSpan<Vertex_Pixlit1UvNmap>(vertexBufferBytes);

        //Write vertices
        for (auto& vertex : vertices)
            obj << "v " << std::to_string(vertex.posX) << " " << std::to_string(vertex.posY) << " " << std::to_string(vertex.posZ) << "\n";

        //Write uv data
        for (auto& vertex : vertices)
            obj << "vt " << std::to_string(abs((f32)vertex.uvX / 1024.0f)) << " " << std::to_string(abs((f32)vertex.uvY / 1024.0f)) << "\n";

    }
    else if (format == VertexFormat::Pixlit1UvNmapCa)
    {
        //Convert vertexBufferBytes to span of vertex type to access vertex data
        struct Vertex_Pixlit1UvNmapCa
        {
            f32 posX, posY, posZ;
            u8 normalX, normalY, normalZ, normalW;
            u8 tangentX, tangentY, tangentZ, tangentW;
            u8 blendWeightX, blendWeightY, blendWeightZ, blendWeightW;
            u8 blendIndexX, blendIndexY, blendIndexZ, blendIndexW;
            i16 uvX, uvY;
        };
        static_assert(sizeof(Vertex_Pixlit1UvNmapCa) == 32);
        std::span<Vertex_Pixlit1UvNmapCa> vertices = MeshHelpers::ToTypedSpan<Vertex_Pixlit1UvNmapCa>(vertexBufferBytes);

        //Write vertices
        for (auto& vertex : vertices)
            obj << "v " << std::to_string(vertex.posX) << " " << std::to_string(vertex.posY) << " " << std::to_string(vertex.posZ) << "\n";

        //Write uv data
        for (auto& vertex : vertices)
            obj << "vt " << std::to_string(abs((f32)vertex.uvX / 1024.0f)) << " " << std::to_string(abs((f32)vertex.uvY / 1024.0f)) << "\n";

    }
    else if (format == VertexFormat::Pixlit2UvNmap)
    {
        //Convert vertexBufferBytes to span of vertex type to access vertex data
        struct Vertex_Pixlit2UvNmap
        {
            f32 posX, posY, posZ;
            u8 normalX, normalY, normalZ, normalW;
            u8 tangentX, tangentY, tangentZ, tangentW;
            i16 uv0X, uv0Y;
            i16 uv1X, uv1Y;
        };
        static_assert(sizeof(Vertex_Pixlit2UvNmap) == 28);
        std::span<Vertex_Pixlit2UvNmap> vertices = MeshHelpers::ToTypedSpan<Vertex_Pixlit2UvNmap>(vertexBufferBytes);

        //Write vertices
        for (auto& vertex : vertices)
            obj << "v " << std::to_string(vertex.posX) << " " << std::to_string(vertex.posY) << " " << std::to_string(vertex.posZ) << "\n";

        //Todo: Support more advanced formats so more than one uv channel can be outputted
        //Note: Only outputs the first uv channel since obj only supports having 1
        //Write uv data
        for (auto& vertex : vertices)
            obj << "vt " << std::to_string(abs((f32)vertex.uv0X / 1024.0f)) << " " << std::to_string(abs((f32)vertex.uv0Y / 1024.0f)) << "\n";

    }
    else if (format == VertexFormat::Pixlit3UvNmap)
    {
        //Convert vertexBufferBytes to span of vertex type to access vertex data
        struct Vertex_Pixlit3UvNmap
        {
            f32 posX, posY, posZ;
            u8 normalX, normalY, normalZ, normalW;
            u8 tangentX, tangentY, tangentZ, tangentW;
            i16 uv0X, uv0Y;
            i16 uv1X, uv1Y;
            i16 uv2X, uv2Y;
        };
        static_assert(sizeof(Vertex_Pixlit3UvNmap) == 32);
        std::span<Vertex_Pixlit3UvNmap> vertices = MeshHelpers::ToTypedSpan<Vertex_Pixlit3UvNmap>(vertexBufferBytes);

        //Write vertices
        for (auto& vertex : vertices)
            obj << "v " << std::to_string(vertex.posX) << " " << std::to_string(vertex.posY) << " " << std::to_string(vertex.posZ) << "\n";

        //Todo: Support more advanced formats so more than one uv channel can be outputted
        //Note: Only outputs the first uv channel since obj only supports having 1
        //Write uv data
        for (auto& vertex : vertices)
            obj << "vt " << std::to_string(abs((f32)vertex.uv0X / 1024.0f)) << " " << std::to_string(abs((f32)vertex.uv0Y / 1024.0f)) << "\n";

    }
    else if (format == VertexFormat::Pixlit3UvNmapCa)
    {
        //Convert vertexBufferBytes to span of vertex type to access vertex data
        struct Vertex_Pixlit3UvNmapCa
        {
            f32 posX, posY, posZ;
            u8 normalX, normalY, normalZ, normalW;
            u8 tangentX, tangentY, tangentZ, tangentW;
            u8 blendWeightX, blendWeightY, blendWeightZ, blendWeightW;
            u8 blendIndexX, blendIndexY, blendIndexZ, blendIndexW;
            i16 uv0X, uv0Y;
            i16 uv1X, uv1Y;
            i16 uv2X, uv2Y;
        };
        static_assert(sizeof(Vertex_Pixlit3UvNmapCa) == 40);
        std::span<Vertex_Pixlit3UvNmapCa> vertices = MeshHelpers::ToTypedSpan<Vertex_Pixlit3UvNmapCa>(vertexBufferBytes);

        //Write vertices
        for (auto& vertex : vertices)
            obj << "v " << std::to_string(vertex.posX) << " " << std::to_string(vertex.posY) << " " << std::to_string(vertex.posZ) << "\n";

        //Todo: Support more advanced formats so more than one uv channel can be outputted
        //Note: Only outputs the first uv channel since obj only supports having 1
        //Write uv data
        for (auto& vertex : vertices)
            obj << "vt " << std::to_string(abs((f32)vertex.uv0X / 1024.0f)) << " " << std::to_string(abs((f32)vertex.uv0Y / 1024.0f)) << "\n";

    }
    else //Todo: Add remaining vertex formats
    {
        printf("Unsupported vertex format %s passed to MeshHelpers::WriteVerticesToObj()!", to_string(format).c_str());
        return false;
    }

    return true;
}
