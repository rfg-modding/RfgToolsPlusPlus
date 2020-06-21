#pragma once
#include "common/Typedefs.h"

//Todo: Figure out how to interpret this data. Seems to be some kind of hash table but unsure what more to make of it
//Data section in version 36 of zone files. Found in RFG and RFGR. Only found in zone files with certain flag values
struct ZonePcRelationData36
{
    u8 Padding0[4];
    u16 Free = 0;
    u16 Slot[7280];
    u16 Next[7280];
    u8 Padding1[2];
    u32 Keys[7280];
    u32 Data[7280];
};

//Required for the read technique used to work
static_assert(sizeof(ZonePcRelationData36) == 87368,
    "Error! sizeof(ZonePcRelationData36) must equal 87368 bytes. This data is loaded straight into memory and cast to this struct so it must be this size to work.");