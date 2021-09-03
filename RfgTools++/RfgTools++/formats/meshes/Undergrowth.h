#pragma once
#include "common/Typedefs.h"
#include "RfgTools++/types/Vec2.h"
#include "RfgTools++/types/Vec3.h"
#include "RfgTools++/types/Mat4.h"
#include <BinaryTools/BinaryReader.h>
#include <string>
#include <vector>

struct UndergrowthLayerData
{
    i32 NumModels;
    u32 ModelsOffset;
    f32 MaxDensity;
    f32 MaxFadeDistance;
    f32 MinFadeDistance;
    i32 PlacementMethod;
    i32 RandomSeed;

    void Read(BinaryReader& cpuFile)
    {
        NumModels = cpuFile.ReadInt32();
        ModelsOffset = cpuFile.ReadUint32();
        MaxDensity = cpuFile.ReadFloat();
        MaxFadeDistance = cpuFile.ReadFloat();
        MinFadeDistance = cpuFile.ReadFloat();
        PlacementMethod = cpuFile.ReadInt32();
        RandomSeed = cpuFile.ReadInt32();
    }
};
static_assert(sizeof(UndergrowthLayerData) == 28, "sizeof(UndergrowthLayerData) must be 28 bytes to match game data.");

struct UndergrowthCellLayerData
{
    u8 LayerIndex;
    u8 Density;
    u8 Bitmask[8];

    void Read(BinaryReader& cpuFile)
    {
        LayerIndex = cpuFile.ReadUint8();
        Density = cpuFile.ReadUint8();
        for (int i = 0; i < 8; i++)
            Bitmask[i] = cpuFile.ReadUint8();
    }
};
static_assert(sizeof(UndergrowthCellLayerData) == 10, "sizeof(UndergrowthCellLayerData) must be 10 bytes to match game data.");

struct SingleUndergrowthCellLayerData
{
    u32 NumSingleUndergrowth;
    u32 NumExtraModelsOffset;
    u32 SingleUndergrowthOffset;

    void Read(BinaryReader& cpuFile)
    {
        NumSingleUndergrowth = cpuFile.ReadUint32();
        NumExtraModelsOffset = cpuFile.ReadUint32();
        SingleUndergrowthOffset = cpuFile.ReadUint32();
    }
};
static_assert(sizeof(SingleUndergrowthCellLayerData) == 12, "sizeof(SingleUndergrowthCellLayerData) must be 12 bytes to match game data.");

struct SingleUndergrowthData
{
    u32 MeshIndex;
    Vec3 Position;
    f32 Scale;
    f32 ColorLerp;

    void Read(BinaryReader& cpuFile)
    {
        MeshIndex = cpuFile.ReadUint32();
        cpuFile.ReadToMemory(&Position, sizeof(Vec3));
        Scale = cpuFile.ReadFloat();
        ColorLerp = cpuFile.ReadFloat();
    }
};
static_assert(sizeof(SingleUndergrowthData) == 24, "sizeof(SingleUndergrowthData) must be 24 bytes to match game data.");