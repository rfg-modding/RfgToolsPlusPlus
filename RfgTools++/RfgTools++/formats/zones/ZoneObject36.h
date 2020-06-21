#pragma once
#include "common/Typedefs.h"
#include "RfgTools++/types/Vec3.h"

//Zone object used in zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR
class ZoneObject36
{
public:
    u32 ClassnameHash = 0; //Todo: Identify string name for classname based on this
    u32 Handle = 0;
    Vec3 Bmin = {};
    Vec3 Bmax = {};
    u16 Flags = 0;
    u16 BlockSize = 0; //Size of the objects entire data block, including these values
    u32 Parent = 0;
    u32 Sibling = 0;
    u32 Child = 0;
    u32 Num = 0;
    
    u16 NumProps = 0; //Number of properties describing the object that immediately follow these values in the zone file
    u16 PropBlockSize = 0; //Size of just the prop data
    //Followed by properties list. Will store separately so this can be directly memory mapped
    //Todo: Maybe have a dummy variable here represently the prop block memory immediately following this and provide ways to iterate and change prop blocks
    //Todo: Alternatively transform prop blocks into dedicated classes/structs that ZonePc36 owns and manipulates. Will likely make editing easier at a perf cost due to copies
};

//Required for the read technique used to work
static_assert(sizeof(ZoneObject36) == 56,
    "Error! sizeof(ZoneObject36) must equal 56 bytes. This data is loaded straight into memory and cast to this struct so it must be this size to work.");