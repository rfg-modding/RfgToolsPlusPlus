#pragma once
#include "common/Typedefs.h"
#include "MeshDataBlock.h"
#include "MeshEnums.h"
#include <span>
#include <fstream>

struct MeshInstanceData
{
    MeshDataBlock Info;
    std::span<u8> VertexBuffer;
    std::span<u8> IndexBuffer;
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

    //Write binary vertex buffer to a .obj file
    bool WriteVerticesToObj(std::ofstream& obj, MeshDataBlock& info, std::span<u8> vertexBufferBytes);
}