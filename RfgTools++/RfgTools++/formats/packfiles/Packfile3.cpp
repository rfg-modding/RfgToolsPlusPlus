#include "Packfile3.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "common/concurrency/Parallel.h"
#include "compression/Compression.h"
#include "hashes/Hash.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <filesystem>
#include <iostream>
#include <future>
#include <zlib.h>

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

    u64 totalDataSize = 0;
    u64 totalDataSize2 = 0;
    u64 offset = 0;
    u64 offset2 = 0;
    u64 offset3 = 0;

    //Header data size and compressed data size for C&C packfile
    u64 offset4 = 0; //data size
    u64 offset5 = 0; //compressed data size

    int i = 0;
    for (auto& entry : Entries)
    {
        offset += entry.CompressedDataSize;
        offset2 += entry.DataSize;
        offset3 += entry.DataSize;

        offset4 += entry.DataSize;
        offset5 += entry.CompressedDataSize;

        u64 pad = BinaryWriter::CalcAlign(offset, 2048);
        u64 pad2 = BinaryWriter::CalcAlign(offset2, 2048);
        u64 pad3 = BinaryWriter::CalcAlign(offset5, 2048);

        if (i == Entries.size() - 1)
            pad = 0;
        if (i == Entries.size() - 1)
            pad2 = 0;
        if (i == Entries.size() - 1)
            pad3 = 0;

        offset += pad;
        offset2 += pad2;
        if (!Condensed)
            offset5 += pad3;

        totalDataSize += entry.DataSize;
        totalDataSize2 += entry.DataSize + pad;

        if (Compressed && Condensed && i != Entries.size() - 1)
        {
            u32 alignPad = BinaryWriter::CalcAlign(offset4, 16);
            offset4 += alignPad;
        }
        else if (!Condensed)
            offset4 += BinaryWriter::CalcAlign(offset4, 2048);

        i++;
    }

    auto a = 2;
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

void Packfile3::Pack(const string& inputPath, const string& outputPath, bool compressed, bool condensed)
{
    if (!std::filesystem::exists(inputPath))
    {
        throw std::runtime_error("Input path: \"" + inputPath + "\" does not exist. Cannot pack!");
    }
    Path::CreatePath(std::filesystem::path(outputPath).parent_path().string());

    //Data used by this function
    struct Packfile3EntryExt
    {
        Packfile3Entry Entry;
        string FullPath;
    };
    u32 curNameOffset = 0;
    u32 curDataOffset = 0;
    u32 totalDataSize = 0;
    u32 totalNamesSize = 0;
    Packfile3Header header;
    BinaryWriter out(outputPath);
    std::vector<string> filenames = {};
    std::vector<Packfile3EntryExt> entries = {};

    //Todo: Come up with a less flimsy way of handling this. Could get incorrect value if new file added to folder at inopportune moment
    //Todo: Maybe create a std::vector of file paths first, only using one directory_iterator
    u32 numSubfiles = 0;
    for (auto& inFile : std::filesystem::directory_iterator(inputPath))
        numSubfiles++;

    //Create entry for each file in input folder. Calc size/offset values
    u32 curSubfile = 0;
    for (auto& inFile : std::filesystem::directory_iterator(inputPath))
    {
        string filename = inFile.path().filename().string();
        filenames.push_back(filename);

        entries.push_back
        (
            {
                Packfile3Entry
                {
                    .NameOffset = curNameOffset,
                    .DataOffset = curDataOffset,
                    .NameHash = Hash::HashVolition(filename),
                    .DataSize = (u32)inFile.file_size(),
                    .CompressedDataSize = compressed ? 0 : 0xFFFFFFFF,
                },
                inFile.path().string()
            }
        );

        curNameOffset += (u32)filename.size() + 1;
        curDataOffset += (u32)inFile.file_size();
        totalDataSize += (u32)inFile.file_size();
        totalNamesSize += (u32)filename.size() + 1;

        if (compressed && condensed && curSubfile != numSubfiles - 1)
        {
            u32 alignPad = BinaryWriter::CalcAlign(curDataOffset, 16);
            u32 alignPad2 = BinaryWriter::CalcAlign(totalDataSize, 16);
            curDataOffset += alignPad;
            //header.DataSize += (u32)inFile.file_size();
            totalDataSize += alignPad2;
        }
        else if (!condensed)
            curDataOffset += out.CalcAlign(curDataOffset, 2048);

        curSubfile++;
    }

    //Set packfile flags
    u32 packfileFlags = 0;
    if (compressed)
        packfileFlags |= 1;
    if (condensed)
        packfileFlags |= 2;

    //Todo: See if we can delay this to the end when we know all the header values
    //Set header values that we know
    header = Packfile3Header
    {
        .Signature = 0x51890ACE,
        .Version = 3,
        .ShortName = {0},
        .PathName = {0},
        .Flags = packfileFlags,
        .NumberOfSubfiles = (u32)entries.size(),
        .FileSize = 0, //Not yet known, set after writing file data. Includes padding
        .EntryBlockSize = (u32)entries.size() * 28, //Doesn't include padding
        .NameBlockSize = totalNamesSize, //Doesn't include padding
        .DataSize = (compressed && condensed) ? totalDataSize : 0, //Includes padding
        .CompressedDataSize = compressed ? 0 : 0xFFFFFFFF, //Not known, set to 0xFFFFFFFF if not compressed
    };

    //Calc data start and skip to it's location. We'll circle back and write header + entries at the end when he have all stats
    u32 dataStart = 0;
    dataStart += 2048; //Header size
    dataStart += entries.size() * 28; //Each entry is 28 bytes
    dataStart += out.CalcAlign(dataStart, 2048); //Align(2048) after end of entries
    dataStart += totalNamesSize; //Filenames list
    dataStart += out.CalcAlign(dataStart, 2048); //Align(2048) after end of file names
    out.WriteNullBytes(dataStart - out.Position());

    //Write subfile data
    if (compressed && condensed) //WriteDataCompressedAndCondensed(writer.BaseStream);
    {
        //Todo: Try to use piecemeal deflate method with z_stream to compress data
        //Todo: There's only one zlib header
        //Todo: Each entry has a compressed size value, so can't just make one big buffer and compress it all at once
        z_stream deflateStream;
        deflateStream.zalloc = Z_NULL;
        deflateStream.zfree = Z_NULL;
        deflateStream.opaque = Z_NULL;
        deflateStream.avail_in = 0; //(u32)input.size_bytes();
        deflateStream.next_in = nullptr; //input.data();
        deflateStream.avail_out = 0; //(u32)output.size_bytes();
        deflateStream.next_out = nullptr; //output.data();
        int result = deflateInit(&deflateStream, Z_BEST_SPEED);
        if (result != Z_OK)
        {
            auto a = 2;
        }
        //deflate(&deflateStream, Z_NO_FLUSH);
        //deflateEnd(&deflateStream);

        uLong lastOut = 0;
        u64 tempDataOffset = 0;
        for (u32 i = 0; i < entries.size(); i++)
        {
            Packfile3EntryExt& entry = entries[i];
            std::vector<char> subFileData = File::ReadAllBytes(entry.FullPath);
            tempDataOffset += subFileData.size();
            //Add align(16) null bytes after uncompressed data. Not added to entry.DataSize but necessary for compression for some reason
            if (i != entries.size() - 1)
            {
                u32 alignPad = BinaryWriter::CalcAlign(tempDataOffset, 16);
                if (alignPad != 0)
                {
                    tempDataOffset += alignPad;
                    for (u32 j = 0; j < alignPad; j++)
                    {
                        subFileData.push_back(0);
                    }
                }
            }

            uLong deflateUpperBound = deflateBound(&deflateStream, subFileData.size());
            char* dest = new char[deflateUpperBound];

            deflateStream.next_in = (Bytef*)subFileData.data();
            deflateStream.avail_in = subFileData.size();
            deflateStream.next_out = (Bytef*)dest;
            deflateStream.avail_out = deflateUpperBound;
            int result2 = deflate(&deflateStream, Z_SYNC_FLUSH);
            if (result2 != Z_OK)
            {
                auto a = 2;
            }

            uLong entryCompressedSize = deflateStream.total_out - lastOut;
            entry.Entry.CompressedDataSize = entryCompressedSize;
            header.CompressedDataSize += entryCompressedSize;
            //header.DataSize += entry.Entry.DataSize;
            //if (i != entries.size() - 1)
            //{
            //    u32 uncompressedPad = (u32)BinaryWriter::CalcAlign(header.DataSize, 2048);
            //    header.DataSize += uncompressedPad; //header.DataSize is calculated the same way even when compressed
            //}

            out.WriteFromMemory(dest, entryCompressedSize);
            out.Flush();
            lastOut = deflateStream.total_out;
            delete[] dest;
        }
        deflateEnd(&deflateStream);

        //DeflateResult Deflate(std::span<u8> input)
        //{
        //    uLong deflateUpperBound = compressBound(input.size_bytes());
        //    uLongf destLen = deflateUpperBound;
        //    char* dest = new char[deflateUpperBound];

        //    compress2((Bytef*)dest, &destLen, (Bytef*)input.data(), input.size_bytes(), Z_BEST_SPEED);
        //    return DeflateResult
        //    {
        //        .Buffer = (u8*)dest,
        //        .BufferSize = deflateUpperBound,
        //        .DataSize = destLen
        //    };
        //}


        //u8* bulkUncompressedData = new u8[totalDataSize];
        //u64 bulkOffset = 0;
        //for (auto& entry : entries)
        //{
        //    //Read subfile data and pack it into a big buffer
        //    std::vector<char> subFileData = File::ReadAllBytes(entry.FullPath);
        //    memcpy(bulkUncompressedData + bulkOffset, subFileData.data(), subFileData.size());
        //    bulkOffset += subFileData.size();
        //}

        //////Todo: See if this method is correct
        //////Todo: See if theres a less ram intensive way of doing this. Maybe by repeatedly updating z_stream and doing in steps
        //////Compress the whole buffer at once and write to file
        ////Compression::DeflateResult compressedData = Compression::Deflate({ bulkUncompressedData, totalDataSize });
        ////out.WriteFromMemory(compressedData.Buffer, compressedData.DataSize);
        ////out.Flush();

        ////entry.
    }
    else if (compressed) //WriteDataCompressed(writer.BaseStream);
    {
        u32 i = 0;
        for (auto& entry : entries)
        {
            //Read subfile data and compress it
            std::vector<char> subFileData = File::ReadAllBytes(entry.FullPath);
            Compression::DeflateResult compressedData = Compression::Deflate({ (u8*)subFileData.data(), subFileData.size() });
            
            //Write compressed data to file
            out.WriteFromMemory(compressedData.Buffer, compressedData.DataSize);
            out.Flush();

            //Update data sizes and free compressed data buffer
            entry.Entry.CompressedDataSize = (u32)compressedData.DataSize;
            header.CompressedDataSize += (u32)compressedData.DataSize;
            header.DataSize += entry.Entry.DataSize;
            delete[] compressedData.Buffer;

            //Add alignment padding for all except final entry
            if (i != entries.size() - 1)
            {
                u32 padSize = (u32)out.Align(2048);
                u32 uncompressedPad = (u32)BinaryWriter::CalcAlign(header.DataSize, 2048);
                header.DataSize += uncompressedPad; //header.DataSize is calculated the same way even when compressed
                header.CompressedDataSize += padSize;
            }
            i++;
        }
    }
    else //WriteDataDefault(writer, condensed);
    {
        u32 i = 0;
        for (auto& entry : entries)
        {
            std::vector<char> subFileData = File::ReadAllBytes(entry.FullPath);
            out.WriteFromMemory(subFileData.data(), subFileData.size());
            header.DataSize += entry.Entry.DataSize;

            //There's no padding bytes if the packfile is condensed or after the final entry
            if (!condensed && i != entries.size() - 1)
                header.DataSize += (u32)out.Align(2048);

            i++;
        }
    }

    //Write header data
    header.FileSize = (u32)out.Length();
    out.SeekBeg(0);
    header.WriteToBinary(out);

    for (auto& entry : entries)
    {
        entry.Entry.WriteToBinary(out);
    }
    out.Align(2048);

    for (auto& filename : filenames)
    {
        out.WriteNullTerminatedString(filename);
    }
    out.Align(2048);
    out.Flush();
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