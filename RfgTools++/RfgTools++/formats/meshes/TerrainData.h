#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>
#include "RfgTools++/types/Vec3.h"

struct TerrainLayerMap
{
    u32 ResX;
    u32 ResY;
    u32 BitDepth;
    u32 DataSize;
    u32 DataOffset;
    u32 NumMaterials;
    u32 MaterialNamesOffset;
    u32 MaterialIndexOffset;
};
static_assert(sizeof(TerrainLayerMap) == 32, "sizeof(TerrainLayerMap) must equal 32 bytes to match game data.");

struct TerrainData
{
    Vec3 Bmin;
    Vec3 Bmax;
    u32 Xres;
    u32 Zres;
    u32 NumOccluders;
    u32 OccludersOffset;
    u32 TerrainMaterialMapOffset;
    u32 TerrainMaterialsOffset;
    u32 NumTerrainMaterials;
    u32 MinimapMaterialHandle;
    u32 MinimapMaterialOffset;
    u32 LowLodPatchesOffset;
    u32 LowLodMaterialOffset;
    u32 LowLodMaterialMapOffset;
    u32 NumSubzones;
    u32 SubzonesOffset;
    u32 PfDataOffset;
    //84
    TerrainLayerMap LayerMap;
    //116
    u32 NumUndergrowthLayers;
    u32 UndergrowthLayersOffset;
    u32 UndergrowthCellDataOffset;
    u32 NumUndergrowthCellLayerDatas;
    u32 UndergrowthCellLayerDataOffset;
    u32 SingleUndergrowthCellLayerDataOffset;
    u32 StitchPieceCmIndex;
    u32 NumInvisibleBarriers;
    u32 InvisibleBarriersOffset;
    u32 ShapeHandle;
    u32 NumSidemapMaterials;
    u32 SidemapDataOffset;
    u32 ObjectStubOffset;
    u32 StitchPhysicsInstancesOffset;
    u32 NumStitchPhysicsInstances;
    u32 ObjectStubPtr;
    u32 ObjectStubPtrPadding;
    //880 bytes padding

	void Read(BinaryReader& cpuFile)
	{
		cpuFile.ReadToMemory(this, sizeof(TerrainData));
        cpuFile.Skip(880);
	}
};
static_assert(sizeof(TerrainData) == 184, "sizeof(TerrainData) must be 184 bytes to match game data.");