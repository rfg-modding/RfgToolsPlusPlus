#include "Packfile3.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/concurrency/Parallel.h"
#include <BinaryTools/BinaryReader.h>
//Todo: Currently have manually placed copy of zconf.h in here because of zlib cmake scripts renaming it. Need to automate that fix
#include <zlib.h>
#include <iostream>
#include <future>

void Packfile3::ReadMetadata(BinaryReader* reader)
{
    //Create BinaryReader if one isn't provided
    if (reader == nullptr)
        reader = new BinaryReader(path_);

    //Read header directly into memory
    reader->ReadToMemory(&Header, sizeof(Packfile3Header));
    reader->Align(2048); //Align to entries block

    //Basic header validation
    if (Header.Signature != 1367935694)
        throw std::exception(("Error! Invalid packfile signature. Expected 1367935694, detected " + std::to_string(Header.Signature)).c_str());
    if (Header.Version != 3)
        throw std::exception(("Error! Invalid packfile version. Expected 3, detected " + std::to_string(Header.Version)).c_str());

    //Set flag shorthand vars
    Compressed = (Header.Flags & PACKFILE_FLAG_COMPRESSED) == PACKFILE_FLAG_COMPRESSED;
    Condensed = (Header.Flags & PACKFILE_FLAG_CONDENSED) == PACKFILE_FLAG_CONDENSED;

    //Reserve enough space in the vector for the entries
    Entries.reserve(Header.NumberOfSubfiles);

    //Read the entries directly into the vector and update it's size via ::assign()
    reader->ReadToMemory(Entries.data(), Header.NumberOfSubfiles * sizeof(Packfile3Entry)); //Todo: Double check this isn't actually making a second copy somehow
    Entries.assign(Entries.data(), Entries.data() + Header.NumberOfSubfiles); //This seems a bit hacky
    reader->Align(2048); //Align to reach filename block

    //Read filenames into heap buffer
    filenamesBuffer_ = new u8[Header.NameBlockSize];
    reader->ReadToMemory(filenamesBuffer_, Header.NameBlockSize);
    reader->Align(2048); //Align to reach next data block start
    
    //Make array of pointers to each string for easy access. Actual string data is still held in single heap buffer
    //Note: Tested using std::string and it took about twice as long due to copying + more allocating. Keep in mind if ever want to switch this to use std::string
    EntryNames.push_back((const char*)filenamesBuffer_);
    for (int i = 0; i < Header.NameBlockSize - 1; i++)
    {
        if (filenamesBuffer_[i] == '\0')
            EntryNames.push_back((const char*)filenamesBuffer_ + i + 1);
    }

    dataBlockOffset_ = reader->Position();
    readMetadata_ = true;
    //Todo: May need to patch data offsets when they overflow over the i32 size limit. See RfgTools C# packfile tools
}

void Packfile3::ExtractSubfiles(const string& outputPath)
{
    //Create reader
    auto* reader = new BinaryReader(path_);

    //Read metadata if it hasn't been. Reuse the reader
    if (!readMetadata_)
        ReadMetadata(reader);

    //Seek to data block
    reader->SeekBeg(dataBlockOffset_);

    //Ensure output path exists
    Path::CreatePath(outputPath);

    //Todo: Look into having multiple sub-options that can be chosen. Should test to see if they're worthwhile. Ex:
    //Todo:     - Multithreaded extraction
    //Todo:     - Load entire data block into memory at once for speed when enough ram
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
    //Todo: Support streaming in data section for C&C instead of loading all at once

}

void Packfile3::ExtractCompressed(const string& outputPath, BinaryReader& reader)
{

}

void Packfile3::ExtractDefault(const string& outputPath, BinaryReader& reader)
{
    //Implementation that reads whole data block in at once
    u8* buffer = new u8[Header.DataSize];
    reader.SeekBeg(dataBlockOffset_);
    reader.ReadToMemory(buffer, Header.DataSize);

    u32 index = 0;
    for (const auto& entry : Entries)
    {
        u8* dataStart = buffer + entry.DataOffset;
        File::WriteToFile(outputPath + EntryNames[index], { dataStart, entry.DataSize });
        index++;
    }

    //Delete buffer after use
    delete[] buffer;
}

void Packfile3::WriteStreamsFile(const string& outputPath)
{
    
}