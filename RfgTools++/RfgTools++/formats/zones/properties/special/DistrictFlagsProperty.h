#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//Todo: Confirm that these are the right flags
enum class DistrictFlags : u8
{
    None = 0,
    AllowCough = 1,
    AllowAmbEdfCivilianDump = 2,
    PlayCapstoneUnlockedLines = 4,
    DisableMoraleChange = 8,
    DisableControlChange = 16,
};

class DistrictFlagsProperty : public IZoneProperty
{
public:
    DistrictFlags Data = DistrictFlags::None;

#pragma warning(disable:4100)
    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
        DataType = ZonePropertyType::DistrictFlags;
        Data = (DistrictFlags)reader.ReadUint8();
        return true;
    }
#pragma warning(default:4100)
};