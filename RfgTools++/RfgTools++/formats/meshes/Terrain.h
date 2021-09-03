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

//RFG ctmesh_pc and gtmesh_pc files
class Terrain
{
public:
	//Read header data from cpu file
	void Read(BinaryReader& cpuFile, const string& name);

	string Name;

private:
	bool readHeader_ = false;
};