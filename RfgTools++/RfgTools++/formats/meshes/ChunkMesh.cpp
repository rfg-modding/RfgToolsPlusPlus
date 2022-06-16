#include "ChunkMesh.h"
#include <stdexcept>

constexpr u32 ExpectedSignature = 2966351781;
constexpr u32 ExpectedVersion = 56;
constexpr u32 ExpectedSourceVersion = 20;

void ChunkMesh::Read(std::span<u8> cpuFileBytes, std::string_view name)
{
    Name = name;
    std::span<u32> cpuFileBytesAsU32List = { (u32*)cpuFileBytes.data(), cpuFileBytes.size() / sizeof(u32) };
    BinaryReader cpuFile(cpuFileBytes);

	//Read header & validate
	cpuFile.ReadToMemory(&Header, sizeof(ChunkHeader));
	if (Header.Signature != ExpectedSignature)
		throw std::runtime_error("Invalid chunk signature. Expected " + std::to_string(ExpectedSignature) + ", found " + std::to_string(Header.Signature));
	if (Header.Version != ExpectedVersion)
		throw std::runtime_error("Invalid chunk version. Expected " + std::to_string(ExpectedVersion) + ", found " + std::to_string(Header.Version));
	if (Header.SourceVersion != ExpectedSourceVersion)
		throw std::runtime_error("Invalid chunk source version. Expected " + std::to_string(ExpectedSourceVersion) + ", found " + std::to_string(Header.SourceVersion));

	//Skip some unknown data. Skipped/read in the current best guess for how the data is structured
	cpuFile.Skip(400);
	u32 unkValue0 = cpuFile.ReadUint32();
	cpuFile.Skip(28);
	u32 unkValue1 = cpuFile.ReadUint32();
	cpuFile.Skip(32);
	u32 unkValue2 = cpuFile.ReadUint32();
	cpuFile.Skip(184);

	//Should be at the render data offset
	if (cpuFile.Position() != Header.RenderCpuDataOffset)
		throw std::runtime_error("Error! Haven't reached the chunk render data section when expected!");

	//Read mesh header
	MeshHeader.Read(cpuFile);

	//Read texture names
	cpuFile.Align(16);
	u32 textureNamesBlockSize = cpuFile.ReadUint32();
	Textures = cpuFile.ReadSizedStringList(textureNamesBlockSize);

	//TODO: Figure out what this data is
	//Some kind of material data
	cpuFile.Align(16);
	u32 materialOffset = cpuFile.ReadUint32();
	u32 numMaterials = cpuFile.ReadUint32();
	cpuFile.Skip(numMaterials * 4); //Potentially a list of material IDs or offsets
	cpuFile.Skip(materialOffset);
	cpuFile.Skip(numMaterials * 8);
	cpuFile.Align(16);
	//TODO: Figure out what data is between here and the destroyables list

	//Skip to destroyables. Haven't fully reversed the format yet
	cpuFile.SeekBeg(Header.DestructionOffset);
	cpuFile.Align(128);
	u32 numDestroyables = cpuFile.ReadUint32();
	cpuFile.Skip((numDestroyables * 8) + 4);
	cpuFile.Align(16);

	//Read destroyables
	for (size_t i = 0; i < numDestroyables; i++)
	{
        //Create new destroyable instance
        Destroyable& destroyable = Destroyables.emplace_back();
        size_t destroyableStartPos = cpuFile.Position();

        //Read base data and align to next piece of data
        cpuFile.ReadToMemory(&destroyable, 44); //Read variables at start of the destroyable (AabbTreeOffset <-> Mass)
        cpuFile.Align(128);

        //Read base object data
        destroyable.Subpieces.resize(destroyable.NumObjects);
        cpuFile.ReadToMemory(destroyable.Subpieces.data(), destroyable.NumObjects * sizeof(Subpiece));
        destroyable.SubpieceData.resize(destroyable.NumObjects);
        cpuFile.ReadToMemory(destroyable.SubpieceData.data(), destroyable.NumObjects * sizeof(SubpieceData));

        //Todo: Figure out what this data is meant to be. Game has some physical material code here. Maybe link material
        for (auto& subpiece : destroyable.Subpieces)
            cpuFile.Skip(subpiece.NumLinks * 2);

        cpuFile.Align(4);

        //Read links
        destroyable.Links.resize(destroyable.NumLinks);
        cpuFile.ReadToMemory(destroyable.Links.data(), destroyable.NumLinks * sizeof(Link));
        cpuFile.Align(4);

        //Read dlods
        destroyable.Dlods.resize(destroyable.NumDlods);
        cpuFile.ReadToMemory(destroyable.Dlods.data(), destroyable.NumDlods * sizeof(Dlod));
        cpuFile.Align(4);

        //TODO: Fix code for reading rbb node and instance data. Currently has some issues due to format only being partially reversed
        //Read rbb nodes
        //Destroyable.RbbNodes.push_back(RbbNode());
        //RbbNode* curNode = &destroyable.RbbNodes.back();
        //curNode->Read(cpuFile);
        //u32 num_objects = curNode->num_objects;
        //while (numObjects > 0)
        //{
        //    //Todo: Implement this loop. Not yet added since the test file doesn't use it
        //    throw std::exception("Support for multiple rbb nodes not yet added!");
        //}
        //cpuFile.Align(4);

        ////Read instance data. Seems to be junk in the file and set at runtime. Unless it's offset from the packfile start or something painful like that
        //if (Destroyable.InstanceDataOffset != 0xFFFFFFFF)
        //{
        //    cpuFile.ReadToMemory(&Destroyable.InstanceData, sizeof(DestroyableInstanceData));
        //    //printf("Data size: %d\n", destroyable.InstanceData.DataSize);
        //}
        //static_assert(sizeof(DestroyableInstanceData) == 20, "DestroyableInstanceData size check failed!");

        ////Seek to end of this destroyable
        ////cpuFile.SeekBeg(destroyableStartPos + destroyable.instance_data.data_size);

        ////cpuFile.SeekBeg(destroyableStartPos + destroyable.base.transform_buffer_offset);

        ////Todo: There may be some data we have to read/skip here. The code at this point was confusing. Something something aabb instances

        //cpuFile.Align(16);
        //u32 maybeTransformBufferSize = cpuFile.ReadUint32();
        //cpuFile.Skip(destroyable.Subpieces.size() * 36);
        //cpuFile.Align(16);

        //TODO: Get a good enough understanding of the format to get rid of this. This is fine as a temporary fix to get chunks rendering in the Nanoforge map editor. Long term should try to remove hacks like this.
        //Hacky way to find the next destroyable
        if (i != numDestroyables - 1 && numDestroyables > 1)
        {
            size_t pos = 0;
            size_t k = cpuFile.Position() / sizeof(u32);
            const size_t posMax = cpuFile.Length() / sizeof(u32);
            bool destroyableNotFound = false;
            while (true)
            {
                if (k >= posMax)
                {
                    destroyableNotFound = true;
                    break;
                }

                //Loop through cpuFile as an in memory array of u32s. Much quicker than using BinaryReader which uses streams under the hood
                u32 val = cpuFileBytesAsU32List[k];
                k += 1;
                size_t posReal = k * sizeof(u32);
                size_t posU32 = k;
                if (val == 0xFFFFFFFF)
                {
                    //Might've reached destroyable_base.inst_data_offset. Verify by seeing a value then a bunch of null bytes follow
                    k += 2; //cpuFile.Skip(8);
                    const size_t nullCheckCount = 7; //Could do more, usually 18 * 4 null bytes
                    bool notFound = false;
                    for (size_t l = 0; l < nullCheckCount; l++)
                    {
                        u32 val2 = cpuFileBytesAsU32List[k + l]; //cpuFile.ReadUint32();
                        if (val2 == 0)
                        {
                            continue;
                        }
                        else
                        {
                            notFound = true;
                            break;
                        }
                    }

                    if (notFound)
                    {
                        //Not found, seek back to start of scanned data
                        //cpuFile.SeekBeg(pos);
                        k = posU32;
                        continue;
                    }
                    else
                    {
                        //Seek to start of destroyable data
                        posReal -= 36;//40;
                        cpuFile.SeekBeg(posReal);
                        break;
                    }
                }
            }

            if (destroyableNotFound)
            {
                printf("Couldn't find next destroyable, i = %zd, numDestroyables = %d, chunkFilename = '%s'\n", i, numDestroyables, string(name.data()).c_str());
                break;
            }
        }
	}

    //Skip collision models. Some kind of havok data format that hasn't been reversed yet.
    cpuFile.SeekBeg(Header.CollisionModelDataOffset);
    ReadHavokBinaryTagfile(cpuFile);

    //Read destroyable UIDs, names, and indices
    cpuFile.Align(128);
    for (size_t i = 0; i < Destroyables.size(); i++)
    {
        u32 uid = cpuFile.ReadUint32();

        //Read destroyable name which is up to 24 characters long
        string destroyableName = cpuFile.ReadFixedLengthString(24);
        destroyableName.erase(std::ranges::find(destroyableName, '\0'), destroyableName.end()); //Remove extra null terminators

        i32 destroyableIndex = cpuFile.ReadInt32();
        u32 isDestroyable = cpuFile.ReadUint32();
        u32 numSnapPoints = cpuFile.ReadUint32();

        //Note: This is only here since the code can't reliably read all destroyables yet. Can be removed once fixed. See the stupid hack on line 129
        if (destroyableIndex >= Destroyables.size())
        {
            cpuFile.Skip(10 * sizeof(ChunkSnapPoint));
            continue;
        }
        if (numSnapPoints > 10)
        {
            throw std::runtime_error("Encountered chunk destroyable with > 10 snap points! Chunk name: " + Name + ", destroyable name: " + destroyableName + ", uid: " + 
                                     std::to_string(uid) + ", numSnapPoints: " + std::to_string(numSnapPoints));
        }

        Destroyable& destroyable = Destroyables[destroyableIndex];
        destroyable.UID = uid;
        destroyable.Name = destroyableName;
        destroyable.IsDestroyable = isDestroyable;
        destroyable.NumSnapPoints = numSnapPoints;
        for (size_t i = 0; i < 10; i++)
            cpuFile.ReadToMemory(&destroyable.SnapPoints[i], sizeof(ChunkSnapPoint));
    }


	_readHeader = true;
}

std::optional<MeshInstanceData> ChunkMesh::ReadMeshData(BinaryReader& gpuFile)
{
	if (!_readHeader)
		return {};

    MeshInstanceData instanceData;
    instanceData.Info = MeshHeader;

    //Read indices
    gpuFile.SeekBeg(16);
    const size_t indexBufferSizeBytes = MeshHeader.NumIndices * MeshHeader.IndexSize;
    instanceData.IndexBuffer.resize(indexBufferSizeBytes);
    gpuFile.ReadToMemory(instanceData.IndexBuffer.data(), indexBufferSizeBytes);

    //Read vertices
    gpuFile.Align(16);
    const size_t vertexBufferSizeBytes = MeshHeader.NumVertices * MeshHeader.VertexStride0;
    instanceData.VertexBuffer.resize(vertexBufferSizeBytes);
    gpuFile.ReadToMemory(instanceData.VertexBuffer.data(), vertexBufferSizeBytes);

	return instanceData;
}

void ChunkMesh::ReadHavokBinaryTagfile(BinaryReader& cpuFile)
{
    //Todo: Actually read and parse these
    size_t tagfileStartOffset = cpuFile.Position();
    u32 maybeHavokBinaryTagfileSig = cpuFile.ReadUint32();
    if (maybeHavokBinaryTagfileSig != 1212891981) //Equals ASCII "MCKH"
        throw std::runtime_error("Invalid havok binary tagfile sig in chunk file");

    cpuFile.Skip(4); //Maybe num havok objects
    u32 tagfileSize = cpuFile.ReadUint32();
    u32 maybeNumCollisionModels = cpuFile.ReadUint32();

    //Skip havok tagfile data for now until it's understood
    cpuFile.SeekBeg(tagfileStartOffset + tagfileSize);
}