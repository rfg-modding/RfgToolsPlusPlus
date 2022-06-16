#pragma once
#include "common/Typedefs.h"
#include "common/String.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include "MeshDataBlock.h"
#include "MeshEnums.h"
#include "MeshHeaderShared.h"
#include "MeshHelpers.h"
#include "ChunkTypes.h"
#include <vector>

struct ChunkHeader;
struct Destroyable;
struct Subpiece;
struct SubpieceData;
struct Link;
struct Dlod;
struct RbbNode;
struct DestroyableInstanceData;

//RFG cchk_pc/gchk_pc files
class ChunkMesh
{
public:
    ChunkHeader Header;
    MeshDataBlock MeshHeader;
    std::vector<string> Textures = {};
    std::vector<Destroyable> Destroyables = {};

	//Read header data from cpu file
	void Read(std::span<u8> cpuFileBytes, std::string_view name);

	//Read vertex and index buffers of a mesh. Must call Read to get data from cpu file first
	std::optional<MeshInstanceData> ReadMeshData(BinaryReader& gpuFile);

	string Name;

private:
	void ReadHavokBinaryTagfile(BinaryReader& cpuFile);

	bool _readHeader = false;
};