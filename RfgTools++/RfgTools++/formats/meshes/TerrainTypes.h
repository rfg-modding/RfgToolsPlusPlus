#pragma once
#include "common/Typedefs.h"
#include "RfgTools++/types/Vec3.h"
#include "RfgTools++/types/Mat3.h"
#include <BinaryTools/BinaryReader.h>

struct TerrainRenderableData
{
    u32 MeshDataOffset = 0;
    u32 RenderableOffset = 0;
    Vec3 AabbMin;
    Vec3 AabbMax;
    Vec3 BspherePosition;
    f32 BsphereRadius = 0.0f;

    void Read(BinaryReader& data)
    {
        MeshDataOffset = data.ReadUint32();
        RenderableOffset = data.ReadUint32();
        data.ReadToMemory(&AabbMin, sizeof(Vec3));
        data.ReadToMemory(&AabbMax, sizeof(Vec3));
        data.ReadToMemory(&BspherePosition, sizeof(Vec3));
        BsphereRadius = data.ReadFloat();
    }
};
static_assert(sizeof(TerrainRenderableData) == 48, "sizeof(TerrainRenderableData) must equal 48 to match game data.");

struct TerrainSubzoneData
{
    u32 SubzoneIndex = 0;
    Vec3 Position;
    u32 PatchCount = 0;
    u32 PatchesOffset = 0;
    TerrainRenderableData RenderableData;
    u32 NumDecals = 0;
    u32 DecalsOffset = 0;
    u32 StitchMeshDataOffset = 0;
    u32 StitchRenderableOffset = 0;
    u32 NumStitchPieces = 0;
    u32 StitchPiecesOffset = 0;
    u32 NumRoadDecalMeshes = 0;
    u32 RoadDecalMeshesOffset = 0;
    u32 HeaderVersion = 0;
    //996 bytes padding

    void Read(BinaryReader& data)
    {
        SubzoneIndex = data.ReadUint32();
        data.ReadToMemory(&Position, sizeof(Vec3));
        PatchCount = data.ReadUint32();
        PatchesOffset = data.ReadUint32();
        RenderableData.Read(data);
        NumDecals = data.ReadUint32();
        DecalsOffset = data.ReadUint32();
        StitchMeshDataOffset = data.ReadUint32();
        StitchRenderableOffset = data.ReadUint32();
        NumStitchPieces = data.ReadUint32();
        StitchPiecesOffset = data.ReadUint32();
        NumRoadDecalMeshes = data.ReadUint32();
        RoadDecalMeshesOffset = data.ReadUint32();
        HeaderVersion = data.ReadUint32();
        data.Skip(996);
    }
};
static_assert(sizeof(TerrainSubzoneData) == 108, "sizeof(TerrainSubzoneData) must equal 108 to match game data.");

struct TerrainPatch
{
    u32 InstanceOffset = 0;
    Vec3 Position;
    Mat3 Rotation;
    u32 SubmeshIndex = 0;
    Vec3 LocalAabbMin;
    Vec3 LocalAabbMax;
    Vec3 LocalBspherePosition;
    f32 LocalBsphereRadius = 0.0f;

    void Read(BinaryReader& data)
    {
        InstanceOffset = data.ReadUint32();
        data.ReadToMemory(&Position, sizeof(Vec3));
        data.ReadToMemory(&Rotation, sizeof(Mat3));
        SubmeshIndex = data.ReadUint32();
        data.ReadToMemory(&LocalAabbMin, sizeof(Vec3));
        data.ReadToMemory(&LocalAabbMax, sizeof(Vec3));
        data.ReadToMemory(&LocalBspherePosition, sizeof(Vec3));
        LocalBsphereRadius = data.ReadFloat();
    }
};
static_assert(sizeof(TerrainPatch) == 96, "sizeof(TerrainPatch) must equal 96 to match game data.");

struct TerrainStitchInstance
{
    u32 StitchChunkNameOffset = 0;
    u32 NumSkirts = 0;
    u32 SkirtsOffset = 0;
    u32 NumStitchedSkirts = 0;
    u32 StitchedSkirtsOffset = 0;
    Vec3 Position;
    Mat3 Rotation;
    u32 HavokHandle = 0;

    void Read(BinaryReader& data)
    {
        StitchChunkNameOffset = data.ReadUint32();
        NumSkirts = data.ReadUint32();
        SkirtsOffset = data.ReadUint32();
        NumStitchedSkirts = data.ReadUint32();
        StitchedSkirtsOffset = data.ReadUint32();
        data.ReadToMemory(&Position, sizeof(Vec3));
        data.ReadToMemory(&Rotation, sizeof(Mat3));
        HavokHandle = data.ReadUint32();
        data.Skip(32);
    }
};
static_assert(sizeof(TerrainStitchInstance) == 72, "sizeof(TerrainStitchInstance) must equal 72 to match game data.");

struct RoadMeshData
{
    u32 NumMeshInstances = 0;
    u32 MaterialOffset = 0;
    u32 MaterialHandle = 0;
    u32 MaterialMapOffset = 0;
    u32 MeshDataOffset = 0;
    u32 MeshOffset = 0;
    u32 RenderableOffset = 0;
    Vec3 Position;

    void Read(BinaryReader& data)
    {
        NumMeshInstances = data.ReadUint32();
        MaterialOffset = data.ReadUint32();
        MaterialHandle = data.ReadUint32();
        MaterialMapOffset = data.ReadUint32();
        MeshDataOffset = data.ReadUint32();
        MeshOffset = data.ReadUint32();
        RenderableOffset = data.ReadUint32();
        data.ReadToMemory(&Position, sizeof(Vec3));
    }
};
static_assert(sizeof(RoadMeshData) == 40, "sizeof(RoadMeshData) must equal 72 to match game data.");