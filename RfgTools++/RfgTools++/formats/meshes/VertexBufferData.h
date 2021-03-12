#pragma once
#include "common/Typedefs.h"
#include "MeshEnums.h"
#include <BinaryTools/BinaryReader.h>

//Description of mesh vertex buffer data layout. Typically you'll find this in mesh cpu files describing a vertex buffer in the gpu file.
//E.g. csmesh_pc files have one of these per submesh which describe a vertex buffer in the matching gsmesh_pc file.
//See also: IndexBufferData.h
class VertexBufferData
{
public:
    u32 NumVerts; //The number of vertices in the buffer
    u8 VertexStride0; //The size of each vertex in bytes
    VertexFormat Format; //The format of the vertex
    u8 NumUvChannels; //The number of texture channels
    u8 VertexStride1; //Unknown purpose
    u32 VertexOffset; //Seems to be a pointer set at runtime

    void Read(BinaryReader& data)
    {
        NumVerts = data.ReadUint32();
        VertexStride0 = data.ReadChar();
        Format = (VertexFormat)data.ReadChar();
        NumUvChannels = data.ReadChar();
        VertexStride1 = data.ReadChar();
        VertexOffset = data.ReadUint32();
    }
};