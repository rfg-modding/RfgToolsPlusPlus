#pragma once
#include "common/Typedefs.h"
#include "Common/memory/Span.h"
#include "Packfile3Header.h"
#include "Packfile3Entry.h"
#include "RfgTools++/formats/asm/AsmFile5.h"
#include <vector>
#include <span>
#include <optional>
#include <filesystem>

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
//Essentially a labelled span. Used for files stored in memory.
struct MemoryFile
{
    string Filename;
    std::span<u8> Bytes;
};

//Packfile version 3 used in RFG and RFGR (vpp_pc and str2_pc)
class Packfile3
{
public:
    //Used when the packfile source is a file
    Packfile3(const string& path);
    //Used when the packfile source is a memory buffer
    Packfile3(std::span<u8> buffer);
    //Delete the default constructor since we want to ensure the path or buffer are set properly
    Packfile3() = delete;
    ~Packfile3();
    Packfile3(const Packfile3& other); //Copy constructor
    Packfile3(Packfile3&& other) noexcept; //Move constructor
    Packfile3& operator=(const Packfile3& other); //Copy assignment operator
    Packfile3& operator=(Packfile3&& other) noexcept; //Move assignment operator

    //Functions which need access to the file
    //Reads header, entries, and filenames from packfile
    void ReadMetadata(BinaryReader* reader = nullptr);
    //Extracts subfiles to outputPath
    void ExtractSubfiles(const string& outputPath, bool writeStreamsFile = false);
    //Todo: Make the way this functions returned memory must be freed more obvious. Currently odd and easy to mess up
    //Extracts subfiles in memory and returns it. NOTE: This overload only supports C&C packfiles and the user must free the returned files memory.
    //Also note that you only need to free the first file since the extraction method decompresses them this way
    std::vector<MemoryFile> ExtractSubfiles(bool writeStreamsFile = false);

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

    static void Pack(const string& inputPath, const string& outputPath, bool compressed, bool condensed);

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

    static void ReadStreamsFile(const string& inputPath, bool& compressed, bool& condensed, std::vector<std::filesystem::directory_entry>& subfilePaths);

    std::span<u8> buffer_ = {};
    string path_ = {};
    string name_ = {};
    bool readMetadata_ = false;
    u8* filenamesBuffer_ = nullptr;
    u64 dataBlockOffset_ = 0;

    DataSource packfileSourceType = DataSource::None;
};

//Todo: Move these into a util file
MemoryFile GetStreamsFileMemory(Packfile3* packfile);
void WriteStreamsFile(Packfile3* packfile, const string& outputPath);