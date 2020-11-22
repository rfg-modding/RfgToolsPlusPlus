#pragma once
#include "common/Typedefs.h"
#include "MeshHeaderShared.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include "MeshEnums.h"
#include "MaterialBlock.h"
#include "VertexBufferData.h"
#include "IndexBufferData.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <filesystem>
#include <optional>
#include <vector>
#include <span>

struct MeshInstanceData
{
    std::span<u8> VertexBuffer;
    std::span<u8> IndexBuffer;
};

class StaticMesh
{
public:
    //Read header data from cpu file
    void Read(BinaryReader& reader, const string& name, u32 signature, u32 version, bool isChunkFile = false);
    //Read raw data of a submesh. Must call Read to get data from cpu file first
    std::optional<MeshInstanceData> ReadSubmeshData(BinaryReader& gpuFile, u32 index, bool isChunkFile = false);
    void Write(BinaryWriter& out);
    void Write(const string& path);
    void WriteToObj(const string& gpuFilePath, const string& outputFolderPath, const string& diffuseTexturePath = "", const string& specularTexturePath = "", const string& normalTexturePath = "");

    string Name;
    //Shared mesh header used by all mesh types
    MeshHeaderShared Header;
    //Static mesh specific header data (may be similar to other mesh types data)
    u32 NumLods;
    u32 LodSubmeshIdOffset;
    u32 MeshTagsOffset;
    u32 MeshTagsNumTags;
    u32 MeshTagsInternalOffset;
    u32 CmIndex;

    //Todo: Use MeshDataBlock here. It is exactly this layout so no point in making two separate definitions
    //Mesh data
    u32 MeshVersion;
    u32 MeshSimpleCrc; //Hash used several times in cpu and gpu file to validate mesh
    u32 CpuDataSize; //Size of data section from MeshOffset to material map data (minus alignment padding at end) in bytes
    u32 GpuDataSize; //Size of gpu file in bytes
    u32 NumSubmeshes;
    u32 SubmeshesOffset; //Seems to be a pointer set at runtime
    VertexBufferData VertexBufferConfig;
    IndexBufferData IndexBufferConfig;

    //Submeshes
    std::vector<SubmeshData> SubMeshes;

    //Render blocks
    std::vector<RenderBlock> RenderBlocks;

    //Material data block
    MaterialBlock MaterialBlock;

    //Texture names
    std::vector<string> TextureNames;

private:
    bool readHeader_ = false;
};