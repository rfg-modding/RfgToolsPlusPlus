#pragma once
#include "common/Typedefs.h"
#include "Common/memory/Span.h"
#include "Packfile3Header.h"
#include "Packfile3Entry.h"
#include "RfgTools++/formats/asm/AsmFile5.h"
#include <vector>
#include <span>
#include <optional>

#define PACKFILE_FLAG_COMPRESSED 1
#define PACKFILE_FLAG_CONDENSED 2
#define INVALID_HANDLE 0xFFFFFFFF

class BinaryReader;

//Packfile version 3 used in RFG and RFGR
class Packfile3
{
public:
    Packfile3(const string& path);

    void ReadMetadata(BinaryReader* reader = nullptr);
    void ExtractSubfiles(const string& outputPath);
    bool CanExtractSingleFile() const;
    //Attempts to extract a subfiles data as a raw byte array. User must free memory once they're done with it
    //Handles decompressing compressed subfiles. Currently does not work on packfiles with both the compressed & condensed flags
    std::optional<std::span<u8>> ExtractSingleFile(s_view name);
    bool Contains(s_view subfileName);

    //Read data from any asm_pc files the vpp_pc contains. Used to read contents of str2_pc files inside vpp_pc files
    void ReadAsmFiles();

    //Done here instead of in a destructor due to destructor for some reason being called when sticking these in a std::vector
    void Cleanup() { delete filenamesBuffer_; }
    const char* NameCstr() const { return name_.data(); }
    string Name() const { return name_; }

    Packfile3Header Header;
    bool Compressed = false;
    bool Condensed = false;
    std::vector<Packfile3Entry> Entries = {};
    std::vector<const char*> EntryNames = {};

    std::vector<AsmFile5> AsmFiles = {};

private:
    bool Contains(s_view subfileName, u32& index);
    void ExtractCompressedAndCondensed(const string& outputPath, BinaryReader& reader);
    void ExtractCompressed(const string& outputPath, BinaryReader& reader);
    void ExtractDefault(const string& outputPath, BinaryReader& reader);
    void WriteStreamsFile(const string& outputPath);

    string path_ = {};
    string name_ = {};
    bool readMetadata_ = false;
    u8* filenamesBuffer_ = nullptr;
    u64 dataBlockOffset_ = 0;
};