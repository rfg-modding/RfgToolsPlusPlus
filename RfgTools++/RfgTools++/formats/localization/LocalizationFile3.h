#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>

struct LocalizationEntry3
{
    u32 KeyHash;
    u32 Offset;
    u32 Length;

    void Read(BinaryReader& reader)
    {
        KeyHash = reader.ReadUint32();
        Offset = reader.ReadUint32();
        Length = reader.ReadUint32();
    }
};

//Version 3 of rfglocatext files used by RFG
class LocalizationFile3
{
public:
    void Read(BinaryReader& reader, std::string_view name);

    //Header
    u32 Signature = 0;
    u32 Version = 0;
    u32 NumStrings = 0;

    //Data
    std::vector<LocalizationEntry3> Entries = {};
    std::vector<std::wstring> EntryData = {};

    //Runtime metadata. Not in format
    string Name;

};