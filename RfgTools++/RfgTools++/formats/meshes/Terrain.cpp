#include "Terrain.h"

void Terrain::Read(BinaryReader& cpuFile, std::string_view name)
{
	Name = name;

	//Read header
	Signature = cpuFile.ReadUint32();
	Version = cpuFile.ReadUint32();
	Index = cpuFile.ReadUint32();
	NumStitchPieceNames = cpuFile.ReadUint32();

	//Validate header
	if (Signature != 1514296659) //ASCII string "SUBZ"
		throw std::runtime_error("Error! Invalid ctmesh_pc signature. Expected 1514296659. Detected " + std::to_string(Signature));
	if (Version != 31)
		throw std::runtime_error("Error! Invalid ctmesh_pc version. Expected 31. Detected " + std::to_string(Version));

	u32 stitchPieceNamesSize = cpuFile.ReadUint32();
	StitchPieceNames = cpuFile.ReadSizedStringList(stitchPieceNamesSize);
	cpuFile.Align(4);

	Subzone.Read(cpuFile);
	for (u32 i = 0; i < Subzone.PatchCount; i++)
	{
		auto& patch = Patches.emplace_back();
		patch.Read(cpuFile);
	}
	cpuFile.Align(16);

	//Read high lod terrain mesh data
	TerrainMesh.Read(cpuFile);
	cpuFile.Align(4);

	//Read stitch piece data
	for (u32 i = 0; i < Subzone.NumStitchPieces; i++)
	{
		auto& stitchInstance = StitchInstances.emplace_back();
		stitchInstance.Read(cpuFile);
	}
	for (u32 i = 0; i < Subzone.NumStitchPieces; i++)
	{
		StitchPieceNames2.push_back(cpuFile.ReadNullTerminatedString());
		cpuFile.Align(4);
		cpuFile.Skip(4);

		//Todo: Make this less hacky
		//Skip unknown data that's between some strings
		if (i < Subzone.NumStitchPieces - 1)
		{
			while (cpuFile.PeekChar() < 33 || cpuFile.PeekChar() > 126)
				cpuFile.Skip(4);
		}
	}
	cpuFile.Skip(4);

	//Read stitch mesh data
	if (Subzone.NumRoadDecalMeshes > 0)
	{
		HasStitchMesh = true;

		//Todo: Come up with a less hacky way of doing this
		//Skip unknown data at before road mesh header that has indices which can be parsed
		u32 i = cpuFile.ReadUint32();
		while (true)
		{
			if (cpuFile.PeekChar() == 0)
			{
				cpuFile.Skip(4);
			}
			else if ((u32)cpuFile.PeekChar() == i + 1)
			{
				cpuFile.Skip(4);
				i++;

				//Hit version at start of mesh data block, stop
				if (cpuFile.PeekChar() != 0)
				{
					cpuFile.SeekReverse(4);
					break;
				}
			}
			else
				break;
		}

		cpuFile.Align(16);
		StitchMesh.Read(cpuFile);
	}
	cpuFile.Align(4);

	//Read road mesh data
	for (u32 i = 0; i < Subzone.NumRoadDecalMeshes; i++)
	{
		auto& roadMeshData = RoadMeshDatas.emplace_back();
		roadMeshData.Read(cpuFile);
	}
	for (u32 i = 0; i < Subzone.NumRoadDecalMeshes; i++)
	{
		auto& mesh = RoadMeshes.emplace_back();
		cpuFile.Align(16);
		mesh.Read(cpuFile);
		cpuFile.Align(4);

		//Todo: Find better way of doing this
		//Skip null data of varying size
		while (cpuFile.PeekUint32() == 0)
		{
			cpuFile.Skip(4);
		}

		u32 textureNamesSize = cpuFile.ReadUint32();
		RoadTextures.push_back(cpuFile.ReadSizedStringList(textureNamesSize));

		cpuFile.Align(16);
		cpuFile.Skip(16);
		cpuFile.Align(16);

		auto& roadMaterial = RoadMaterials.emplace_back();
		roadMaterial.Read(cpuFile);
	}

	readHeader_ = true;
}

std::optional<MeshInstanceData> Terrain::ReadTerrainMeshData(BinaryReader& gpuFile)
{
	if (!readHeader_)
		return {};

	gpuFile.SeekBeg(0);
	u32 startCRC = gpuFile.ReadUint32();
	if (startCRC != TerrainMesh.VerificationHash)
		throw std::runtime_error("Error! Terrain mesh header CRC mismatch in " + Name + ". Expected " + std::to_string(TerrainMesh.VerificationHash) + ", Read " + std::to_string(startCRC));

	//Read index buffer
	gpuFile.SeekBeg(TerrainMesh.IndicesOffset);
	u32 indicesSize = TerrainMesh.NumIndices * TerrainMesh.IndexSize;
	std::vector<u8> indices(indicesSize);
	gpuFile.ReadToMemory(indices.data(), indicesSize);

	//Read vertex buffer
	gpuFile.SeekBeg(TerrainMesh.VertexOffset);
	u32 verticesSize = TerrainMesh.NumVertices * TerrainMesh.VertexStride0;
	std::vector<u8> vertices(verticesSize);
	gpuFile.ReadToMemory(vertices.data(), verticesSize);

	u32 endCRC = gpuFile.ReadUint32();
	if (startCRC != endCRC)
		throw std::runtime_error("Error! Terrain mesh start/end CRC mismatch in " + Name + ". Expected " + std::to_string(startCRC) + ", Read " + std::to_string(endCRC));

	//Return data buffers
	return MeshInstanceData
	{
		.Info = TerrainMesh,
		.VertexBuffer = vertices,
		.IndexBuffer = indices
	};
}

std::optional<MeshInstanceData> Terrain::ReadStitchMeshData(BinaryReader& gpuFile)
{
	if (!HasStitchMesh)
		return {};

	//Skip terrain mesh data
	gpuFile.SeekBeg(TerrainMesh.VertexOffset); //Seek to vertex buffer which is always after index buffer
	gpuFile.Skip(TerrainMesh.NumVertices * TerrainMesh.VertexStride0);
	gpuFile.Skip(4); //Skip verification CRC

	//Start of stitch mesh data
	gpuFile.Align(16);
	u64 startPos = gpuFile.Position();
	u32 startCRC = gpuFile.ReadUint32();
	if (startCRC != StitchMesh.VerificationHash)
		throw std::runtime_error("Error! Stitch mesh header CRC mismatch in " + Name + ". Expected " + std::to_string(StitchMesh.VerificationHash) + ", Read " + std::to_string(startCRC));

	//Read index buffer
	gpuFile.SeekBeg(startPos + StitchMesh.IndicesOffset);
	u32 indicesSize = StitchMesh.NumIndices * StitchMesh.IndexSize;
	std::vector<u8> indices(indicesSize);
	gpuFile.ReadToMemory(indices.data(), indicesSize);

	//Read vertex buffer
	gpuFile.SeekBeg(startPos + StitchMesh.VertexOffset);
	u32 verticesSize = StitchMesh.NumVertices * StitchMesh.VertexStride0;
	std::vector<u8> vertices(verticesSize);
	gpuFile.ReadToMemory(vertices.data(), verticesSize);

	u32 endCRC = gpuFile.ReadUint32();
	if (startCRC != endCRC)
		throw std::runtime_error("Error! Stitch mesh start/end CRC mismatch in " + Name + ". Expected " + std::to_string(startCRC) + ", Read " + std::to_string(endCRC));

	//Return data buffers
	return MeshInstanceData
	{
		.Info = StitchMesh,
		.VertexBuffer = vertices,
		.IndexBuffer = indices
	};
}

std::optional<std::vector<MeshInstanceData>> Terrain::ReadRoadMeshData(BinaryReader& gpuFile)
{
	if (RoadMeshes.size() == 0)
		return {};

	//Skip terrain mesh data
	gpuFile.SeekBeg(TerrainMesh.VertexOffset); //Seek to vertex buffer which is always after index buffer
	gpuFile.Skip(TerrainMesh.NumVertices * TerrainMesh.VertexStride0);
	gpuFile.Skip(4); //Skip verification CRC

	//Skip stitch mesh data
	gpuFile.Align(16);
	u64 stitchStartPos = gpuFile.Position();
	gpuFile.SeekBeg(stitchStartPos + StitchMesh.VertexOffset);
	gpuFile.Skip(StitchMesh.NumVertices * StitchMesh.VertexStride0);
	gpuFile.Skip(4); //Skip verification CRC

	//Start of road mesh data
	std::vector<MeshInstanceData> out = {};
	for (u32 i = 0; i < RoadMeshes.size(); i++)
	{
		gpuFile.Align(16);
		MeshDataBlock& mesh = RoadMeshes[i];
		u64 startPos = gpuFile.Position();
		u32 startCRC = gpuFile.ReadUint32();
		gpuFile.Align(16);
		if (startCRC != mesh.VerificationHash)
			throw std::runtime_error("Error! Road mesh header CRC mismatch in " + Name + ", Road mesh " + std::to_string(i) + ". Expected " + std::to_string(mesh.VerificationHash) + ", Read " + std::to_string(startCRC));

		//Read index buffer
		gpuFile.SeekBeg(startPos + mesh.IndicesOffset);
		u32 indicesSize = mesh.NumIndices * mesh.IndexSize;
		std::vector<u8> indices(indicesSize);
		gpuFile.ReadToMemory(indices.data(), indicesSize);

		//Read vertex buffer
		gpuFile.SeekBeg(startPos + mesh.VertexOffset);
		u32 verticesSize = mesh.NumVertices * mesh.VertexStride0;
		std::vector<u8> vertices(verticesSize);
		gpuFile.ReadToMemory(vertices.data(), verticesSize);

		u32 endCRC = gpuFile.ReadUint32();
		if (startCRC != endCRC)
			throw std::runtime_error("Error! Road mesh start/end CRC mismatch in " + Name + ", Road mesh " + std::to_string(i) + ". Expected " + std::to_string(startCRC) + ", Read " + std::to_string(endCRC));

		out.push_back(MeshInstanceData
		{
			.Info = mesh,
			.VertexBuffer = vertices,
			.IndexBuffer = indices
		});
	}

	return out;
}