#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>

//Data about a subfile stored inside a packfile for packfile version 3
class Packfile3Entry
{
public:
    //Junk data we don't need. Set by game at runtime
    u8 Junk1[8] = {};
    //Offset of uncompressed data of the entry from the start of the data block
    u64 DataOffset = 0;
    //Hash of the entries filename
    u32 NameHash = 0;
    //Size of the entries uncompressed data
    u32 DataSize = 0;
    //Size of the entries compressed data
    u32 CompressedDataSize = 0;
    //More data that is junk for our purposes
    u8 Junk2[4] = {};

    void Read(BinaryReader& reader)
    {
        reader.Skip(8);
        DataOffset = reader.ReadUint32();
        NameHash = reader.ReadUint32();
        DataSize = reader.ReadUint32();
        CompressedDataSize = reader.ReadUint32();
        reader.Skip(4);
    }
};