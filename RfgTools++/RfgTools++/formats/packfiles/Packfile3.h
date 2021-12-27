#pragma once
#include "common/Typedefs.h"
#include "common/String.h"
#include "common/Handle.h"
#include "Packfile3Header.h"
#include "Packfile3Entry.h"
#include "RfgTools++/formats/asm/AsmFile5.h"
#include <filesystem>
#include <optional>
#include <vector>

class BinaryReader;
class MemoryFile;
class MemoryFileList;

//Data source to read packfile from
enum class DataSource
{
    File,
    Memory,
    None
};

//Packfile v3 flags
#define PACKFILE_FLAG_COMPRESSED 1
#define PACKFILE_FLAG_CONDENSED 2
#define INVALID_HANDLE 0xFFFFFFFF

//Packfile version 3 used in RFG and RFGR (vpp_pc and str2_pc)
class Packfile3
{
public:
    //Load a packfile from a file
    Packfile3(const string& path);
    //Load a packfile from a memory buffer
    Packfile3(std::vector<u8> buffer);
    //Delete the default constructor to ensure Packfile3 instances are always valid (requires eiter path or buffer to parse)
    Packfile3() = delete;

    //Read the header, entries, and filenames from packfile
    void ReadMetadata(Handle<BinaryReader> reader = nullptr);

    //Extract subfiles to outputPath
    void ExtractSubfiles(const string& outputPath, bool writeStreamsFile = false);

    //Extract subfiles into memory and returns them
    MemoryFileList ExtractSubfiles(bool writeStreamsFile = false);

    //Extract a single subfile. The 'fullExtractFallback' argument must be true to use this on files which are both compressed and condensed.
    //It's disabled by default since C&C files don't support single file extracts, and must be extracted fully (slower)
    std::optional<std::vector<u8>> ExtractSingleFile(s_view name, bool fullExtractFallback = false);

    //Parse all asm_pc files in the vpp_pc. This provides a list of all files within the str2_pc files without parsing all of them.
    void ReadAsmFiles();

    bool Contains(s_view subfileName, size_t* index = nullptr);
    void SetName(const string& name) { name_ = name; }
    string Name() const { return name_; }

    //Pack all the files in the folder at inputPath into a packfile at outputPath
    static void Pack(const string& inputPath, const string& outputPath, bool compressed, bool condensed);

    Packfile3Header Header;
    bool Compressed = false;
    bool Condensed = false;
    std::vector<Packfile3Entry> Entries = {};
    std::vector<const char*> EntryNames = {};

    std::vector<AsmFile5> AsmFiles = {};

private:
    std::vector<u8> ExtractDataDefault(Handle<BinaryReader> reader, size_t entryIndex);
    std::vector<u8> ExtractDataCompressed(Handle<BinaryReader> reader, size_t entryIndex);
    //Returns the data for all entries. If provided an index it'll only return data for that entry. Prefer returning all entries and manually splitting for performance.
    std::vector<u8> ExtractDataCompressedAndCondensed(Handle<BinaryReader> reader, size_t entryIndex = -1);

    //Open stream to read data from packfile (can be in a file or in memory)
    Handle<BinaryReader> OpenStream();

    //Read streams.xml
    static void ReadStreamsFile(const string& inputPath, bool& compressed, bool& condensed, std::vector<std::filesystem::directory_entry>& subfilePaths);
    void FixEntryDataOffsets();

    std::vector<u8> buffer_ = {};
    string path_ = {};
    string name_ = {};
    bool readMetadata_ = false;
    std::unique_ptr<char[]> filenamesBuffer_; //Subfile names are all stored in this separated by null terminators
    u64 dataBlockOffset_ = 0;

    DataSource packfileSourceType = DataSource::None;
};

//A file extracted from a packfile and stored in memory. File data is owned by a MemoryFileList.
class MemoryFile
{
public:
    MemoryFile(const string& filename, size_t offset, size_t size) : Filename(filename), _offset(offset), _size(size) {}

    std::span<u8> GetSpan(u8* parent)
    {
        return { parent + _offset, _size };
    }

    const string Filename;

private:
    size_t _offset;
    size_t _size;
};

//A list of files extracted from a packfile and stored in memory. Owns the files memory and automatically handles deallocation.
class MemoryFileList
{
public:
    MemoryFileList() {}
    MemoryFileList(const std::vector<u8>& data, const std::vector<MemoryFile>& files) : _data(data), Files(files) {}
    u8* Data() { return _data.data(); }

    std::vector<MemoryFile> Files;

private:
    std::vector<u8> _data;
};