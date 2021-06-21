#include "LocalizationFile3.h"

void LocalizationFile3::Read(BinaryReader& reader, const string& name)
{
    Name = name;

    //Read and validate header
    Signature = reader.ReadUint32();
    Version = reader.ReadUint32();
    NumStrings = reader.ReadUint32();
    if (Signature != 2823585651)
        throw std::exception(("Error! Invalid rfglocatext file signature. Expected 2823585651, detected " + std::to_string(Signature)).c_str());
    if (Version != 3)
        throw std::exception(("Error! Invalid rfglocatext file version. Expected 3, detected " + std::to_string(Version)).c_str());

    //Read entry metadata
    for (u32 i = 0; i < NumStrings; i++)
        Entries.emplace_back().Read(reader);

    //Read entry strings
    for (u32 i = 0; i < NumStrings; i++)
        EntryData.push_back(reader.ReadNullTerminatedStringWide());
}