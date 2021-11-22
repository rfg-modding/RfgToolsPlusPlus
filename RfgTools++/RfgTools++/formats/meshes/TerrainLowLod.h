#pragma once
#include "common/Typedefs.h"
#include "MeshHeaderShared.h"
#include "TerrainStitchInfo.h"
#include "TerrainData.h"
#include "SidemapMaterial.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include "MeshDataBlock.h"
#include "MeshEnums.h"
#include "MaterialBlock.h"
#include "Undergrowth.h"
#include "VertexBufferData.h"
#include "IndexBufferData.h"
#include "MeshHelpers.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <filesystem>
#include <optional>
#include <vector>
#include <span>

//RFG cterrain_pc and gterrain_pc files. Contains low lod meshes and some data used by high lod terrain.
class TerrainLowLod
{
public:
    //Read header data from cpu file
    void Read(BinaryReader& cpuFile, const string& name);
    //Read vertex and index buffers of a mesh (low lod terrain files contain 9). Must call Read to get data from cpu file first
    std::optional<MeshInstanceData> ReadMeshData(BinaryReader& gpuFile, u32 index);

    //Header data - 36 bytes
    u32 Signature;
    u32 Version;
    u32 NumTextureNames;
    u32 TextureNamesSize;
    u32 NumFmeshNames;
    u32 FmeshNamesSize;
    u32 StitchPieceNamesSize;
    u32 NumStitchPieceNames;
    u32 NumStitchPieces;

    std::vector<string> TextureNames;

    std::vector<string> StitchPieceNames;
    std::vector<TerrainStitchInfo> StitchPieces;

    std::vector<string> FmeshNames;

    TerrainData Data;
    std::vector<string> TerrainMaterialNames;
    std::vector<RfgMaterial> Materials;

    std::vector<UndergrowthLayerData> UndergrowthLayers;
    std::vector<UndergrowthCellLayerData> UndergrowthCellData;
    std::vector<SingleUndergrowthCellLayerData> SingleUndergrowthCellData;
    std::vector<SingleUndergrowthData> SingleUndergrowthData;

    std::vector<SidemapMaterial> SidemapMaterials;
    std::vector<string> LayerMapMaterialNames;
    std::vector<string> LayerMapMaterialNames2;

    std::vector<string> MinimapMaterialNames;
    RfgMaterial MinimapMaterials;

    //Mesh data
    std::vector<MeshDataBlock> Meshes;

    //Members below this aren't part of the file
    string Name;

private:
    void ReadHavokBinaryTagfile(BinaryReader& cpuFile);

    bool readHeader_ = false;
};