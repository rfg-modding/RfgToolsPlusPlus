#pragma once
#include "common/Typedefs.h"
#include "MeshHeaderShared.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include "MeshEnums.h"
#include "MaterialBlock.h"
#include "VertexBufferData.h"
#include "IndexBufferData.h"
#include "MeshDataBlock.h"
#include "MeshHelpers.h"
#include "TerrainTypes.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <filesystem>
#include <optional>
#include <vector>
#include <span>

//RFG ctmesh_pc and gtmesh_pc files
class Terrain
{
public:
	//Read header data from cpu file
	void Read(BinaryReader& cpuFile, std::string_view name);
	//Read vertex and index buffers of a mesh. Must call Read to get data from cpu file first
	std::optional<MeshInstanceData> ReadTerrainMeshData(BinaryReader& gpuFile);
	std::optional<MeshInstanceData> ReadStitchMeshData(BinaryReader& gpuFile);
	std::optional<std::vector<MeshInstanceData>> ReadRoadMeshData(BinaryReader& gpuFile);

	u32 Signature;
	u32 Version;
	u32 Index;
	u32 NumStitchPieceNames;

	std::vector<string> StitchPieceNames;
	TerrainSubzoneData Subzone;
	std::vector<TerrainPatch> Patches;

	//Terrain mesh
	MeshDataBlock TerrainMesh;

	//Stitch piece data
	std::vector<TerrainStitchInstance> StitchInstances;
	std::vector<string> StitchPieceNames2;
	MeshDataBlock StitchMesh;

	//Road data
	std::vector<RoadMeshData> RoadMeshDatas;
	std::vector<MeshDataBlock> RoadMeshes;
	std::vector<RfgMaterial> RoadMaterials;
	std::vector<std::vector<string>> RoadTextures;

	//Members below this aren't part of the file
	string Name;
	bool HasStitchMesh = false;

private:
	bool readHeader_ = false;
};