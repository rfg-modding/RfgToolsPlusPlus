#include "TerrainLowLod.h"

#define HAVOK_BINARY_TAGFILE_SIGNATURE 1212891981

void TerrainLowLod::Read(BinaryReader& cpuFile, const string& name)
{
	Name = name;

	//Read header
	cpuFile.ReadToMemory(&Signature, 36);

	//Validate header
	if (Signature != 1381123412) //ASCII string "TERR"
		throw std::runtime_error("Error! Invalid cterrain_pc signature. Expected 1381123412. Detected " + std::to_string(Signature));
	if (Version != 31)
		throw std::runtime_error("Error! Invalid cterrain_pc version. Expected 31. Detected " + std::to_string(Version));

	TextureNames = cpuFile.ReadSizedStringList(TextureNamesSize);
	cpuFile.Align(4);

	StitchPieceNames = cpuFile.ReadSizedStringList(StitchPieceNamesSize);
	for (u32 i = 0; i < NumStitchPieces; i++)
	{
		TerrainStitchInfo& stitchPiece = StitchPieces.emplace_back();
		stitchPiece.Read(cpuFile);
	}
	cpuFile.Align(4);

	FmeshNames = cpuFile.ReadSizedStringList(FmeshNamesSize);
	cpuFile.Align(4);

	//Misc terrain data
	Data.Read(cpuFile);

	u32 terrainMaterialNamesListSize = cpuFile.ReadUint32();
	TerrainMaterialNames = cpuFile.ReadSizedStringList(terrainMaterialNamesListSize);
	cpuFile.Align(16);

	//Todo: Fix calculations so this isn't necessary
	if (cpuFile.PeekUint32() < 1000000) //Hack to get to the right offset
	{
		cpuFile.Skip(4);
		cpuFile.Align(16);
	}

	cpuFile.Skip(4);
	u32 numMaterials = cpuFile.ReadUint32();
	cpuFile.Skip(28);
	cpuFile.Skip(numMaterials * 4);
	cpuFile.Align(16);

	//Todo: Fix calculations so this isn't necessary
	if (cpuFile.PeekUint32() == 0) //Hack to get to the right offset
	{
		cpuFile.Skip(4);
		cpuFile.Align(16);
	}

	for (u32 i = 0; i < numMaterials; i++)
	{
		RfgMaterial& material = Materials.emplace_back();
		material.Read(cpuFile);
	}

	//Havok binary tagfile
	if (Data.ShapeHandle != 0xFFFFFFFF)
		ReadHavokBinaryTagfile(cpuFile);

	cpuFile.Align(4);
	cpuFile.Skip(Data.NumSubzones * 4);
	if (Data.NumSidemapMaterials > 0)
	{
		cpuFile.Skip(8);
		cpuFile.Skip(Data.NumSidemapMaterials * 4 * 2);
		for (u32 i = 0; i < Data.NumSidemapMaterials; i++)
		{
			SidemapMaterial& material = SidemapMaterials.emplace_back();
			material.Read(cpuFile);
		}
	}

	//Seems to be navmesh / pathfinding data
	cpuFile.Align(4);
	u32 maybeNumNavmeshes = cpuFile.ReadUint32();
	u32 maybeNavmeshSize = cpuFile.ReadUint32();
	cpuFile.Skip(maybeNavmeshSize - 4);
	cpuFile.Align(16);

	if (cpuFile.PeekUint32() == HAVOK_BINARY_TAGFILE_SIGNATURE)
		ReadHavokBinaryTagfile(cpuFile);

	//Likely invisible barrier data
	cpuFile.Align(4);
	cpuFile.Skip(Data.NumInvisibleBarriers * 8);
	cpuFile.Align(16);

	if (cpuFile.PeekUint32() == HAVOK_BINARY_TAGFILE_SIGNATURE)
		ReadHavokBinaryTagfile(cpuFile);

	//Todo: Determine purpose, maybe related to undergrowth/grass placement
	//Layer map data. Seems to have BitDepth * ResX * ResY bits
	cpuFile.Align(16);
	cpuFile.Skip(Data.LayerMap.DataSize);
	cpuFile.Skip(Data.LayerMap.NumMaterials * 4);
	for (int i = 0; i < Data.LayerMap.NumMaterials; i++)
	{
		LayerMapMaterialNames.push_back(cpuFile.ReadNullTerminatedString());
	}
	cpuFile.Align(4);
	cpuFile.Skip(Data.LayerMap.NumMaterials * 4);

	if (Data.NumUndergrowthLayers > 0)
	{
		//Undergrowth layer data
		for (u32 i = 0; i < Data.NumUndergrowthLayers; i++)
		{
			auto& layer = UndergrowthLayers.emplace_back();
			layer.Read(cpuFile);
		}
		size_t numModels = 0;
		for (auto& layer : UndergrowthLayers)
			numModels += layer.NumModels;

		cpuFile.Skip(numModels * 16);
		for (u32 i = 0; i < NumFmeshNames; i++)
		{
			LayerMapMaterialNames2.push_back(cpuFile.ReadNullTerminatedString());

			//Skip extra null terminators that sometimes appear here for some reason
			if (i < NumFmeshNames - 1)
			{
				if (cpuFile.PeekChar() == 0)
					cpuFile.Skip(1);
				if (cpuFile.PeekChar() == 0)
					cpuFile.Skip(1);
			}
		}
		cpuFile.Align(4);
		cpuFile.Skip(16384); //Todo: Determine what this data is

		//Undergrowth cell data
		for (u32 i = 0; i < Data.NumUndergrowthCellLayerDatas; i++)
		{
			auto& cell = UndergrowthCellData.emplace_back();
			cell.Read(cpuFile);
		}
		cpuFile.Align(4);

		//More undergrowth cell data
		for (u32 i = 0; i < Data.NumUndergrowthCellLayerDatas; i++)
		{
			auto& cell = SingleUndergrowthCellData.emplace_back();
			cell.Read(cpuFile);
		}
		size_t numSingleUndergrowths = 0;
		for (auto& cell : SingleUndergrowthCellData)
			numSingleUndergrowths += cell.NumSingleUndergrowth;

		for (u32 i = 0; i < numSingleUndergrowths; i++)
		{
			auto& undergrowth = SingleUndergrowthData.emplace_back();
			undergrowth.Read(cpuFile);
		}

		//Todo: Properly calculate offset instead of using this hack
		cpuFile.Align(4);
		while (cpuFile.PeekUint32() == 0 || cpuFile.PeekUint32() > 1000 || cpuFile.PeekUint32() < 5)
			cpuFile.Skip(4);
	}
	cpuFile.Align(4);

	u32 minimapMaterialNamesListSize = cpuFile.ReadUint32();
	MinimapMaterialNames = cpuFile.ReadSizedStringList(minimapMaterialNamesListSize);
	cpuFile.Align(16);

	MinimapMaterials.Read(cpuFile);
	cpuFile.Skip(432);

	//Read mesh config data
	for (u32 i = 0; i < 9; i++)
	{
		MeshDataBlock& mesh = Meshes.emplace_back();
		mesh.Read(cpuFile);
	}

	//Todo: Check if there's more data here

	readHeader_ = true;
}

std::optional<MeshInstanceData> TerrainLowLod::ReadMeshData(BinaryReader& gpuFile, u32 index)
{
	if (!readHeader_ || index >= Meshes.size())
		return {};

	//Calculate offset of submesh data in gpu file
	u64 meshStartPos = 0;
	for (u32 i = 0; i < index; i++)
	{
		//Jump to vertex buffer start and skip it + the mesh end CRC
		MeshDataBlock& mesh = Meshes[i];
		meshStartPos += mesh.VertexOffset + (mesh.NumVertices * mesh.VertexStride0);
		meshStartPos += 4; //End CRC
	}

	//Get mesh data
	MeshDataBlock& mesh = Meshes[index];
	SubmeshData& submesh = mesh.Submeshes[0];
	RenderBlock& renderBlock = mesh.RenderBlocks[0];

	gpuFile.SeekBeg(meshStartPos);
	u32 startCRC = gpuFile.ReadUint32();
	if(startCRC != mesh.VerificationHash)
		throw std::runtime_error("Error! Mesh header CRC mismatch in " + Name + ". Expected " + std::to_string(mesh.VerificationHash) + ", Read " + std::to_string(startCRC));

	//Read index buffer
	gpuFile.SeekBeg(meshStartPos + mesh.IndicesOffset);
	u32 indexBufferSize = mesh.NumIndices * mesh.IndexSize;
	u8* indexBuffer = new u8[indexBufferSize];
	gpuFile.ReadToMemory(indexBuffer, indexBufferSize);

	//Read vertex buffer
	gpuFile.SeekBeg(meshStartPos + mesh.VertexOffset);
	u32 vertexBufferSize = mesh.NumVertices * mesh.VertexStride0;
	u8* vertexBuffer = new u8[vertexBufferSize];
	gpuFile.ReadToMemory(vertexBuffer, vertexBufferSize);

	u32 endCRC = gpuFile.ReadUint32();
	if (startCRC != endCRC)
		throw std::runtime_error("Error! Mesh start/end CRC mismatch in " + Name + ". Expected " + std::to_string(startCRC) + ", Read " + std::to_string(endCRC));

	//Return data buffers
	return MeshInstanceData
	{
		.VertexBuffer = std::span<u8>(vertexBuffer, vertexBufferSize),
		.IndexBuffer = std::span<u8>(indexBuffer, indexBufferSize)
	};
}

void TerrainLowLod::ReadHavokBinaryTagfile(BinaryReader& cpuFile)
{
	//Todo: Actually read and parse these
	long tagfileStartOffset = cpuFile.Position();
	u32 maybeHavokBinaryTagfileSig = cpuFile.ReadUint32();
	if (maybeHavokBinaryTagfileSig != 1212891981)
		throw std::runtime_error("Invalid havok binary tagfile sig in " + Name);

	cpuFile.Skip(4); //Maybe num havok objects
	u32 tagfileSize = cpuFile.ReadUint32();
	u32 maybeNumCollisionModels = cpuFile.ReadUint32();

	//Skip havok tagfile data for now until it's understood
	cpuFile.SeekBeg(tagfileStartOffset + tagfileSize);
}