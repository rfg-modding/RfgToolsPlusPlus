#pragma once
#include "common/Typedefs.h"
#include "MeshDataBlock.h"
#include "types/Vec4.h"
#include "MeshEnums.h"
#include <fstream>
#include <span>

struct MeshInstanceData
{
    MeshDataBlock Info;
    std::span<u8> VertexBuffer;
    std::span<u8> IndexBuffer;
};

//Gltf compatible vertex
struct GltfVertex
{
    Vec3 Position;
    Vec3 Normal;
    Vec4 Tangent;
    Vec2 Texcoord0;
};

namespace MeshHelpers
{
    //Converts a byte span to a std::span<T>
    template<class T>
    std::span<T> ToTypedSpan(std::span<u8> in)
    {
        //Ensure that there aren't remaining bytes. For example, if T = u32 then in.size_bytes should be divisible by sizeof(u32) without any remainder.
        u32 sizeOfT = sizeof(T);
        if (in.size_bytes() % sizeof(T) != 0)
            throw std::runtime_error("Passed std::span<u8> to ToTypedSpan with a size that isn't cleanly divisible by T. T = " + string(typeid(T).name()));

        //Return typed span
        return std::span<T>((T*)in.data(), in.size_bytes() / sizeOfT);
    }

    //Convert byte span to byte vector. Needed since tinygltf stores data in std::vector<u8>
#pragma warning(disable:4505)
    static std::vector<unsigned char> ToByteVector(std::span<u8>& source)
    {
        //Copy span to vector
        std::vector<unsigned char> out;
        out.reserve(source.size_bytes());
        for (u32 i = 0; i < source.size_bytes(); i++)
            out.push_back(source[i]);

        return out;
    }
#pragma warning(default:4505)

    //Write binary vertex buffer to a .obj file
    bool WriteVerticesToObj(std::ofstream& obj, MeshDataBlock& info, std::span<u8> vertexBufferBytes);

    //Returns true if the format can be exported by ::WriteToGltf().
    bool IsExportSupported(VertexFormat format);

    //Attempt to output mesh as a gltf file. Caller is responsible for cleaning up vertices and indices. Use MeshHelpers::IsExportSupported() to check if the format is supported.
    bool WriteToGltf(const MeshDataBlock& MeshInfo, u32 NumLods, std::span<u8> indices, std::span<u8> vertices, const string& outPath,
                     const string& diffusePath = "", const string& specularPath = "", const string& normalPath = "");

    //Convert RFG vertex format to gltf compatible format. Caller must delete[] the returned span it succeeds.
    std::optional<std::span<u8>> ConvertVerticesToGltfFormat(const MeshDataBlock& meshInfo, std::span<u8> rfgVertices);
}