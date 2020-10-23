#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>

//Data about a subfile stored inside a packfile for packfile version 3
class Packfile3Entry
{
public:
    //Offset of name in subfile name list
    u32 NameOffset = 0;
    //Junk data we don't need. Set by game at runtime
    u8 Junk1[4] = {0};
    //Offset of uncompressed data of the entry from the start of the data block
    u64 DataOffset = 0;
    //Hash of the entries filename
    u32 NameHash = 0;
    //Size of the entries uncompressed data
    u32 DataSize = 0;
    //Size of the entries compressed data
    u32 CompressedDataSize = 0;
    //More data that is junk for our purposes
    u8 Junk2[4] = {0};

    void Read(BinaryReader& reader)
    {
        reader.Skip(8);
        DataOffset = reader.ReadUint32();
        NameHash = reader.ReadUint32();
        DataSize = reader.ReadUint32();
        CompressedDataSize = reader.ReadUint32();
        reader.Skip(4);
    }

    void WriteToBinary(BinaryWriter& out)
    {
        out.WriteUint32(NameOffset);
        out.WriteNullBytes(4); //Junk data
        out.WriteUint32(static_cast<u32>(DataOffset));
        out.WriteUint32(NameHash);
        out.WriteUint32(DataSize);
        out.WriteUint32(CompressedDataSize);
        out.WriteNullBytes(4); //Junk data
    }
};