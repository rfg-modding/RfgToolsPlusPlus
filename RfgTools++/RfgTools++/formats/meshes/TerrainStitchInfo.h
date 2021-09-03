#pragma once
#include "common/Typedefs.h"
#include "RfgTools++/types/Vec2.h"
#include <BinaryTools/BinaryReader.h>

struct TerrainStitchInfo
{
	Vec2 Bmin;
	Vec2 Bmax;
	u32 FilenameOffset;

	void Read(BinaryReader& cpuFile)
	{
		cpuFile.ReadToMemory(this, sizeof(TerrainStitchInfo));
	}
};

static_assert(sizeof(TerrainStitchInfo) == 20, "sizeof(TerrainStitchInfo) must be 20 bytes to match game data.");