#include "Terrain.h"

void Terrain::Read(BinaryReader& cpuFile, const string& name)
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
		//Todo: Come up with a less hacky way of doing this
		//Skip unknown data at before road mesh header that has indices which can be parsed
		u32 i = cpuFile.ReadUint32();
		while (true)
		{
			if (cpuFile.PeekChar() == 0)
			{
				cpuFile.Skip(4);
			}
			else if (cpuFile.PeekChar() == i + 1)
			{
				cpuFile.Skip(4);
				i++;

				//Hit version at start of mesh data block, stop
				if (cpuFile.PeekChar() != 0)
				{
					cpuFile.SeekCur(-4);
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
	if (!readHeader_)// || index >= Meshes.size())
		return {};

	gpuFile.SeekBeg(0);
	u32 startCRC = gpuFile.ReadUint32();
	if (startCRC != TerrainMesh.VerificationHash)
		throw std::runtime_error("Error! Mesh header CRC mismatch in " + Name + ". Expected " + std::to_string(TerrainMesh.VerificationHash) + ", Read " + std::to_string(startCRC));

	//Read index buffer
	gpuFile.SeekBeg(TerrainMesh.IndicesOffset);
	u32 indexBufferSize = TerrainMesh.NumIndices * TerrainMesh.IndexSize;
	u8* indexBuffer = new u8[indexBufferSize];
	gpuFile.ReadToMemory(indexBuffer, indexBufferSize);

	//Read vertex buffer
	gpuFile.SeekBeg(TerrainMesh.VertexOffset);
	u32 vertexBufferSize = TerrainMesh.NumVertices * TerrainMesh.VertexStride0;
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
