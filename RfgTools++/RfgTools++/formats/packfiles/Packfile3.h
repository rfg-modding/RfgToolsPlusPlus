#pragma once
#include "common/Typedefs.h"
#include "Common/memory/Span.h"
#include "Packfile3Header.h"
#include "Packfile3Entry.h"
#include <vector>
#include <span>
#include <optional>

#define PACKFILE_FLAG_COMPRESSED 1
#define PACKFILE_FLAG_CONDENSED 2
#define INVALID_HANDLE 0xFFFFFFFF

class BinaryReader;

class Packfile3
{
public:
    Packfile3(const string& path) : path_(path) {}
    ~Packfile3() { delete filenamesBuffer_; }

    void ReadMetadata(BinaryReader* reader = nullptr);
    void ExtractSubfiles(const string& outputPath);
    bool CanExtractSingleFile() const;
    //Attempts to extract a subfiles data as a raw byte array. User must free memory once they're done with it
    //Handles decompressing compressed subfiles. Currently does not work on packfiles with both the compressed & condensed flags
    std::optional<std::span<u8>> ExtractSingleFile(s_view name);
    bool Contains(s_view subfileName);

    Packfile3Header Header;
    bool Compressed = false;
    bool Condensed = false;
    std::vector<Packfile3Entry> Entries = {};
    std::vector<const char*> EntryNames = {};
    std::vector<string> EntryNamesAlt = {};

    //Todo: Implement this + test speed difference
    //NOT YET IMPLEMENTED
    //The largest data block size that will be loaded into memory all at once
    //This is faster than reading each sub-block individually, but increases RAM usage.
    //For files with both the compressed and condensed flag this does not matter. Those must be loaded in all at once currently.
    u64 MaxSingleReadDataExtractSizeBytes = 300000000;

private:
    bool Contains(s_view subfileName, u32& index);
    void ExtractCompressedAndCondensed(const string& outputPath, BinaryReader& reader);
    void ExtractCompressed(const string& outputPath, BinaryReader& reader);
    void ExtractDefault(const string& outputPath, BinaryReader& reader);
    void WriteStreamsFile(const string& outputPath);

    string path_ = {};
    bool readMetadata_ = false;
    u8* filenamesBuffer_ = nullptr;
    u64 dataBlockOffset_ = 0;
};