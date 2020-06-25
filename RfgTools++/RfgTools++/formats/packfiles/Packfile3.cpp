#include "Packfile3.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "common/concurrency/Parallel.h"
#include "compression/Compression.h"
#include <BinaryTools/BinaryReader.h>
#include <filesystem>
#include <iostream>
#include <future>

Packfile3::Packfile3(const string& path) : path_(path), packfileSourceType(DataSource::File)
{
    name_ = Path::GetFileName(path_);
}

void Packfile3::ReadMetadata(BinaryReader* reader)
{
    //Create BinaryReader if one isn't provided
    bool ownsReader = false;
    if (reader == nullptr)
    {
        ownsReader = true;
        if (packfileSourceType == DataSource::File)
            reader = new BinaryReader(path_);
        else if (packfileSourceType == DataSource::Memory)
            reader = new BinaryReader(buffer_);
    }

    u64 pos_1 = reader->Position();
    //Read header directly into memory
    reader->ReadToMemory(&Header, sizeof(Packfile3Header));
    auto pos_2 = reader->Position();
    reader->Align(2048); //Align to entries block
    auto pos_3 = reader->Position();

    //Basic header validation
    if (Header.Signature != 1367935694)
        throw std::exception(("Error! Invalid packfile signature. Expected 1367935694, detected " + std::to_string(Header.Signature)).c_str());
    if (Header.Version != 3)
        throw std::exception(("Error! Invalid packfile version. Expected 3, detected " + std::to_string(Header.Version)).c_str());

    auto pos_4 = reader->Position();
    //Set flag shorthand vars
    Compressed = (Header.Flags & PACKFILE_FLAG_COMPRESSED) == PACKFILE_FLAG_COMPRESSED;
    Condensed = (Header.Flags & PACKFILE_FLAG_CONDENSED) == PACKFILE_FLAG_CONDENSED;

    if (packfileSourceType == DataSource::File && std::filesystem::file_size(path_) <= 2048)
        return;
    else if (packfileSourceType == DataSource::Memory && buffer_.size() <= 2048)
        return;

    //Reserve enough space in the vector for the entries
    Entries.reserve(Header.NumberOfSubfiles);

    auto pos = reader->Position();

    //Read entries
    for (u32 i = 0; i < Header.NumberOfSubfiles; i++)
    {
        pos = reader->Position();

        Packfile3Entry& entry = Entries.emplace_back();
        entry.Read(*reader);
    }
    auto pos2 = reader->Position();
    reader->Align(2048); //Align to reach filename block
    auto pos3 = reader->Position();

    //Read filenames into heap buffer
    filenamesBuffer_ = new u8[Header.NameBlockSize];
    reader->ReadToMemory(filenamesBuffer_, Header.NameBlockSize);
    auto pos4 = reader->Position();
    reader->Align(2048); //Align to reach next data block start
    auto pos5 = reader->Position();
    
    //Make array of pointers to each string for easy access. Actual string data is still held in single heap buffer
    //Note: Tested using std::string and it took about twice as long due to copying + more allocating. Keep in mind if ever want to switch this to use std::string
    EntryNames.push_back(reinterpret_cast<const char*>(filenamesBuffer_));
    for (int i = 0; i < Header.NameBlockSize - 1; i++)
    {
        auto pos6 = reader->Position();
        if (filenamesBuffer_[i] == '\0')
            EntryNames.push_back(reinterpret_cast<const char*>(filenamesBuffer_) + i + 1);
    }

    dataBlockOffset_ = reader->Position();
    FixEntryDataOffsets();
    readMetadata_ = true;
    if (ownsReader)
        delete reader;
}

void Packfile3::ExtractSubfiles(const string& outputPath)
{
    //Create reader
    BinaryReader* reader = nullptr;
    if (packfileSourceType == DataSource::File)
        reader = new BinaryReader(path_);
    else if (packfileSourceType == DataSource::Memory)
        reader = new BinaryReader(buffer_);

    //Read metadata if it hasn't been. Reuse the reader
    if (!readMetadata_)
        ReadMetadata(reader);

    if (std::filesystem::file_size(path_) <= 2048)
        return;

    //Seek to data block
    reader->SeekBeg(dataBlockOffset_);

    //Ensure output path exists
    Path::CreatePath(outputPath);

    //Todo: Look into having multiple sub-options that can be chosen. Should test to see if they're worthwhile. Ex:
    //Todo:     - Multithreaded extraction
    //Todo:     - Load entire data block into memory at once for speed when enough ram
    //Todo:     - For very large files add an option to stream in data to reduce memory usage
    //Extract data to files. Pick method based on flags
    if (Compressed && Condensed)
        ExtractCompressedAndCondensed(outputPath, *reader);
    else if(Compressed)
        ExtractCompressed(outputPath, *reader);
    else
        ExtractDefault(outputPath, *reader);

    //Write streams file for str2_pc files
    if (Path::GetExtension(outputPath) == ".str2_pc")
        WriteStreamsFile(outputPath); //Todo: Consider writing this for all packfiles

    delete reader;
}

void Packfile3::ExtractCompressedAndCondensed(const string& outputPath, BinaryReader& reader)
{
    //Todo: Support streaming in data section for C&C instead of loading all at once. Some users don't have enough ram for the larger files to be extracted this way
    //Currently reads compressed data into one huge buffer and inflates, then writes out

    //Read all compressed data into buffer and inflate it
    u8* inputBuffer = new u8[Header.CompressedDataSize];
    u8* outputBuffer = new u8[Header.DataSize];
    reader.ReadToMemory(inputBuffer, Header.CompressedDataSize);
    Compression::Inflate({ inputBuffer, Header.CompressedDataSize }, { outputBuffer, Header.DataSize });

    //Write each subfile to disk
    u32 index = 0;
    for (const auto& entry : Entries)
    {
        File::WriteToFile(outputPath + EntryNames[index], { outputBuffer + entry.DataOffset, entry.DataSize });
        index++;
    }

    //Delete buffers after use
    delete[] inputBuffer;
    delete[] outputBuffer;
}

void Packfile3::ExtractCompressed(const string& outputPath, BinaryReader& reader)
{
    //Simple implementation that reads each subfile in from packfile, inflates data, and writes them out
    u32 index = 0;
    for (const auto& entry : Entries)
    {
        //Read file data into buffer and write to separate file
        u8* inputBuffer = new u8[entry.CompressedDataSize];
        reader.ReadToMemory(inputBuffer, entry.CompressedDataSize);
        reader.Align(2048); //Compressed offset not stored in packfile. Just read data and align to next block

        //Create buffer for decompressed data and inflate, then write to file
        u8* outputBuffer = new u8[entry.DataSize];
        Compression::Inflate({ inputBuffer, entry.CompressedDataSize }, { outputBuffer, entry.DataSize });
        File::WriteToFile(outputPath + EntryNames[index], { outputBuffer, entry.DataSize });

        //Delete buffers after use
        delete[] inputBuffer;
        delete[] outputBuffer;
        index++;
    }
}

void Packfile3::ExtractDefault(const string& outputPath, BinaryReader& reader)
{
    //Simple implementation that reads each subfile in from packfile and writes them out
    u32 index = 0;
    for (const auto& entry : Entries)
    {
        //Read file data into buffer and write to separate file
        u8* buffer = new u8[entry.DataSize];
        reader.SeekBeg(dataBlockOffset_ + entry.DataOffset);
        reader.ReadToMemory(buffer, entry.DataSize);
        File::WriteToFile(outputPath + EntryNames[index], {buffer, entry.DataSize});

        //Delete buffer after use
        delete[] buffer;
        index++;
    }
}

void Packfile3::WriteStreamsFile(const string& outputPath)
{
    
}

bool Packfile3::CanExtractSingleFile() const
{
    return !(Compressed && Condensed);
}

std::optional<std::span<u8>> Packfile3::ExtractSingleFile(s_view name, bool fullExtractFallback)
{
    //Todo: Add single file extraction support for C&C packfiles

    //Check if single file extract is supported and the subfile exists
    u32 targetIndex = INVALID_HANDLE;
    if ((!CanExtractSingleFile() && !fullExtractFallback) || !Contains(name, targetIndex))
        return {};

    //Open packfile for reading
    BinaryReader* reader = nullptr;
    if (packfileSourceType == DataSource::File)
        reader = new BinaryReader(path_);
    else if (packfileSourceType == DataSource::Memory)
        reader = new BinaryReader(buffer_);

    Packfile3Entry& entry = Entries[targetIndex];
    
    //This option is stupidly inefficient and only done as a fallback for str2_pc files right now
    if (Compressed && Condensed)
    {
        //Todo: Support streaming in data section for C&C instead of loading all at once. Some users don't have enough ram for the larger files to be extracted this way
        //Currently reads compressed data into one huge buffer and inflates, then writes out
        reader->SeekBeg(dataBlockOffset_);
        //Read all compressed data into buffer and inflate it
        u8* inputBuffer = new u8[Header.CompressedDataSize];
        u8* outputBuffer = new u8[Header.DataSize];
        reader->ReadToMemory(inputBuffer, Header.CompressedDataSize);
        Compression::Inflate({ inputBuffer, Header.CompressedDataSize }, { outputBuffer, Header.DataSize });

        //Write each subfile to disk
        u32 index = 0;
        for (const auto& entry : Entries)
        {
            if (EntryNames[index] == name)
            {
                //Copy data we want into new buffer
                u8* singleFileBuffer = new u8[entry.DataSize];
                memcpy(singleFileBuffer, outputBuffer + entry.DataOffset, entry.DataSize);

                //Delete the rest of the data
                delete[] inputBuffer;
                delete[] outputBuffer;
                return std::span<u8>{ singleFileBuffer, entry.DataSize };
            }
            index++;
        }

        //Delete buffers after use
        delete[] inputBuffer;
        delete[] outputBuffer;
    }
    else if (Compressed)
    {
        //Compressed offset for entries isn't stored. Calculate by running through previous entries
        reader->SeekBeg(dataBlockOffset_);
        for (u32 i = 0; i < targetIndex; i++)
        {
            reader->Skip(Entries[i].CompressedDataSize);
            reader->Align(2048);
        }

        //Read compressed data to inputBuffer
        u8* inputBuffer = new u8[entry.CompressedDataSize];
        reader->ReadToMemory(inputBuffer, entry.CompressedDataSize);

        //Decompress/inflate data into outputBuffer
        u8* outputBuffer = new u8[entry.DataSize];
        Compression::Inflate({ inputBuffer, entry.CompressedDataSize }, { outputBuffer, entry.DataSize });

        //Delete inputBuffer and return outputBuffer filled with inflated data
        delete[] inputBuffer;
        delete reader;
        return std::span<u8>{ outputBuffer, entry.DataSize };
    }
    else
    {
        //Read data into buffer and return it
        u8* buffer = new u8[entry.DataSize];
        reader->SeekBeg(dataBlockOffset_ + entry.DataOffset);
        reader->ReadToMemory(buffer, entry.DataSize);
        delete reader;
        return std::span<u8>{ buffer, entry.DataSize };
    }
}

bool Packfile3::Contains(s_view subfileName)
{
    u32 index = 0;
    return Contains(subfileName, index);
}

void Packfile3::ReadAsmFiles()
{
    if (!readMetadata_)
        ReadMetadata();

    for (u32 i = 0; i < Entries.size(); i++)
    {
        if (Path::GetExtension(EntryNames[i]) != ".asm_pc")
            continue;

        auto* name = EntryNames[i];
        auto data = ExtractSingleFile(EntryNames[i]);
        if (!data)
            continue;

        AsmFile5& asmFile = AsmFiles.emplace_back();
        BinaryReader reader(data.value());
        asmFile.Read(reader);

        //Remove containers that don't have a corresponding str2_pc file. For some reason asm_pc files have these "ghost files"
        auto iterator = asmFile.Containers.begin();
        while (iterator != asmFile.Containers.end())
        {
            bool foundContainer = false;
            for (auto& name : EntryNames)
                if (iterator->Name + ".str2_pc" == name)
                    foundContainer = true;
            
            if (!foundContainer)
                iterator = asmFile.Containers.erase(iterator);
            else
                iterator++;
        }

        delete[] data.value().data();
    }
}

//Fix data offsets. Values in packfile not always valid.
//Ignores packfiles that are compressed AND condensed since those must
//be fully extracted and data offsets aren't relevant in that case.
void Packfile3::FixEntryDataOffsets()
{
    if (Compressed && Condensed)
        return;

    u64 runningDataOffset = 0; //Track relative offset from data section start
    for(auto& entry : Entries)
    {
        //Set entry offset
        entry.DataOffset = runningDataOffset;

        //Update offset based on entry size and storage type
        if (Compressed) //Compressed, not condensed
        {
            runningDataOffset += entry.CompressedDataSize;
            long alignmentPad = GetAlignmentPad(runningDataOffset);
            runningDataOffset += alignmentPad;
        }
        else //Not compressed, maybe condensed
        {
            runningDataOffset += entry.DataSize;
            if (!Condensed)
            {
                long alignmentPad = GetAlignmentPad(runningDataOffset);
                runningDataOffset += alignmentPad;
            }
        }
    }
}

u32 Packfile3::GetAlignmentPad(u64 position)
{
    int remainder = (int)(position % 2048U);
    if (remainder > 0)
    {
        return 2048 - remainder;
    }
    return 0;
}

bool Packfile3::Contains(s_view subfileName, u32& index)
{
    for (u32 i = 0; i < Entries.size(); i++)
    {
        if (EntryNames[i] == subfileName)
        {
            index = i;
            return true;
        }
    }

    return false;
}