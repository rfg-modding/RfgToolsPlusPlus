#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>

//Todo: Determine the contents of this property
class NavpointDataProperty : public IZoneProperty
{
public:
    u32 NavpointType;
    u32 UnkFlag1;
    f32 Radius;
    f32 Speed;
    u32 UnkFlag2;
    u32 UnkFlag3;
    u32 UnkVar1;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
        DataType = ZonePropertyType::NavpointData;

        if (size != 28)
        {
            //Todo: Need to add a logging callback to RfgTools++ so tools like Nanoforge can log these types of messages with their logging system.
            printf("Encountered NavpointData property whos size does not equal 28 bytes! Unsupported!\n");
            return false;
        }

        NavpointType = reader.ReadUint32();
        UnkFlag1 = reader.ReadUint32();
        Radius = reader.ReadFloat();
        Speed = reader.ReadFloat();
        UnkFlag2 = reader.ReadUint32();
        UnkFlag3 = reader.ReadUint32();
        UnkVar1 = reader.ReadUint32();

        return true;
    }
};