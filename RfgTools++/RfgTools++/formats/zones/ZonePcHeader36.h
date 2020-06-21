#pragma once
#include "common/Typedefs.h"

//Header layout used in version 36 of zone files. Found in RFG and RFGR
struct ZonePcHeader36
{
    u32 Signature = 0;
    u32 Version = 0;
    u32 NumObjects = 0;
    u32 NumHandles = 0;
    u32 DistrictHash = 0; //Todo: Identify string name for district based on this
    u32 DistrictFlags = 0;
};

//Required for the read technique used to work
static_assert(sizeof(ZonePcHeader36) == 24,
    "Error! sizeof(ZonePcHeader36) must equal 24 bytes. This data is loaded straight into memory and cast to this struct so it must be this size to work.");