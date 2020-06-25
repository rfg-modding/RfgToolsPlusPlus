#pragma once
#include "common/Typedefs.h"
#include "Common/memory/Span.h"
#include "Packfile3Header.h"
#include "Packfile3Entry.h"
#include "RfgTools++/formats/asm/AsmFile5.h"
#include <vector>
#include <span>
#include <optional>

enum class DataSource
{
    File,
    Memory,
    None
};

#define PACKFILE_FLAG_COMPRESSED 1
#define PACKFILE_FLAG_CONDENSED 2
#define INVALID_HANDLE 0xFFFFFFFF

class BinaryReader;

//Packfile version 3 used in RFG and RFGR
class Packfile3
{
public:
    //Used when the packfile source is a file
    Packfile3(const string& path);
    //Used when the packfile source is a memory buffer
    Packfile3(std::span<u8> buffer) : buffer_(buffer), packfileSourceType(DataSource::Memory) {}
    //Delete the default constructor since we want to ensure the path or buffer are set properly
    Packfile3() = delete;

    //Functions which need access to the file
    void ReadMetadata(BinaryReader* reader = nullptr);
    void ExtractSubfiles(const string& outputPath);
    //Attempts to extract a subfiles data as a raw byte array. User must free memory once they're done with it
    //Handles decompressing compressed subfiles. Currently does not work on packfiles with both the compressed & condensed flags
    //If 'fullExtractFallback' is true it'll do a full extract and pull the requested file. This is very inefficient and so you
    //should really only do it for small files like str2_pc files.
    std::optional<std::span<u8>> ExtractSingleFile(s_view name, bool fullExtractFallback = false);
    //Read data from any asm_pc files the vpp_pc contains. Used to read contents of str2_pc files inside vpp_pc files
    void ReadAsmFiles();


    //Functions which work off a cached data from the functions above
    bool CanExtractSingleFile() const;
    bool Contains(s_view subfileName);
    void SetName(const string& name) { name_ = name; }
    string Name() const { return name_; }
    const char* NameCstr() const { return name_.data(); }

    //Clear resources. Done manually to avoid hidden destruction when copies occur. Easy to miss copies and have odd bugs
    void Cleanup() 
    {
        delete filenamesBuffer_; 
        if (packfileSourceType == DataSource::Memory)
            delete[] buffer_.data();
    }

    Packfile3Header Header;
    bool Compressed = false;
    bool Condensed = false;
    std::vector<Packfile3Entry> Entries = {};
    std::vector<const char*> EntryNames = {};

    std::vector<AsmFile5> AsmFiles = {};

private:
    void FixEntryDataOffsets();
    u32 GetAlignmentPad(u64 position);
    bool Contains(s_view subfileName, u32& index);
    void ExtractCompressedAndCondensed(const string& outputPath, BinaryReader& reader);
    void ExtractCompressed(const string& outputPath, BinaryReader& reader);
    void ExtractDefault(const string& outputPath, BinaryReader& reader);
    void WriteStreamsFile(const string& outputPath);

    std::span<u8> buffer_ = {};
    string path_ = {};
    string name_ = {};
    bool readMetadata_ = false;
    u8* filenamesBuffer_ = nullptr;
    u64 dataBlockOffset_ = 0;

    DataSource packfileSourceType = DataSource::None;
};