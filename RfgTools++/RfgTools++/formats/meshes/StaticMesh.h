#pragma once
#include "common/Typedefs.h"
#include "MeshHeaderShared.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include "MeshHelpers.h"
#include "MeshEnums.h"
#include "MaterialBlock.h"
#include "MeshDataBlock.h"
#include "VertexBufferData.h"
#include "IndexBufferData.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <filesystem>
#include <optional>
#include <vector>
#include <span>

//Static or character mesh used in RFG and RFGR (csmesh_pc and ccmesh_pc)
//They're near identical except for some special data at the end of each file which isn't read yet (see end of StaticMesh::Read())
class StaticMesh
{
public:
    //Read header data from cpu file
    void Read(BinaryReader& reader, const string& name, u32 signature, u32 version);
    //Read raw data of a submesh. Must call Read to get data from cpu file first
    std::optional<MeshInstanceData> ReadMeshData(BinaryReader& gpuFile);
    //Write mesh as a gltf file
    void WriteToGltf(BinaryReader& gpuFile, const string& outPath, const string& diffuseTextureName = "", const string& specularTextureName = "", const string& normalTextureName = "");

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

    //Mesh data
    MeshDataBlock MeshInfo;

    //Material data block
    MaterialBlock MaterialBlock;

    //Texture names
    std::vector<string> TextureNames;

private:
    bool readHeader_ = false;
};