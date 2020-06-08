#pragma once
#include "common/Typedefs.h"

//Data about a subfile stored inside a packfile
class Packfile3Entry
{
public:
    //Junk data we don't need. Set by game at runtime
    u8 Junk1[8] = {};
    //Offset of uncompressed data of the entry from the start of the data block
    u32 DataOffset = 0;
    //Hash of the entries filename
    u32 NameHash = 0;
    //Size of the entries uncompressed data
    u32 DataSize = 0;
    //Size of the entries compressed data
    u32 CompressedDataSize = 0;
    //More data that is junk for our purposes
    u8 Junk2[4] = {};
};

//Required for the read technique used to work
static_assert(sizeof(Packfile3Entry) == 28, 
    "Error! sizeof(Packfile3Entry) must equal 28 bytes. This data is loaded straight into memory and cast to this struct so it must be this size to work.");