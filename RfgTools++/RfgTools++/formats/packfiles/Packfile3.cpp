#include "Packfile3.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "compression/Compression.h"
#include "hashes/Hash.h"
#include <tinyxml2.h>
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <filesystem>
#include <iostream>
#include <future>
#include <zlib.h>

tinyxml2::XMLDocument* GetStreamsFile(Packfile3* packfile);
void WriteStreamsFile(Packfile3* packfile, const string& outputPath);

Packfile3::Packfile3(const string& path) : path_(path), packfileSourceType(DataSource::File)
{
    name_ = Path::GetFileName(path_);
}

Packfile3::Packfile3(std::vector<u8> buffer) : buffer_(buffer), packfileSourceType(DataSource::Memory)
{

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
        else
            return;
    }

    //Read header directly into memory
    reader->ReadToMemory(&Header, sizeof(Packfile3Header));
    reader->Align(2048); //Align to entries block

    //Basic header validation
    if (Header.Signature != 1367935694)
        throw std::exception(("Error! Invalid packfile signature. Expected 1367935694, detected " + std::to_string(Header.Signature)).c_str());
    if (Header.Version != 3)
        throw std::exception(("Error! Invalid packfile version. Expected 3, detected " + std::to_string(Header.Version)).c_str());

    //Set flag shorthand vars
    Compressed = (Header.Flags & PACKFILE_FLAG_COMPRESSED) != 0;
    Condensed = (Header.Flags & PACKFILE_FLAG_CONDENSED) != 0;

    //Return early if there's no data after the header
    if (packfileSourceType == DataSource::File && std::filesystem::file_size(path_) <= 2048)
        return;
    if (packfileSourceType == DataSource::Memory && buffer_.size() <= 2048)
        return;

    //Read entries
    Entries.reserve(Header.NumberOfSubfiles);
    for (u32 i = 0; i < Header.NumberOfSubfiles; i++)
    {
        Packfile3Entry& entry = Entries.emplace_back();
        entry.Read(*reader);
    }
    reader->Align(2048); //Align to reach filename block

    //Read filenames into heap buffer
    filenamesBuffer_ = std::unique_ptr<char[]>(new char[Header.NameBlockSize]);
    reader->ReadToMemory(filenamesBuffer_.get(), Header.NameBlockSize);
    reader->Align(2048); //Align to reach next data block start

    //Make array of pointers to each string for easy access. String data is still held in single buffer
    EntryNames.push_back(reinterpret_cast<const char*>(filenamesBuffer_.get()));
    for (u32 i = 0; i < Header.NameBlockSize - 1; i++)
    {
        if (filenamesBuffer_[i] == '\0')
            EntryNames.push_back(reinterpret_cast<const char*>(filenamesBuffer_.get()) + i + 1);
    }

    dataBlockOffset_ = reader->Position();
    FixEntryDataOffsets();
    readMetadata_ = true;
    if (ownsReader)
        delete reader;
}

void Packfile3::ExtractSubfiles(const string& outputPath, bool writeStreamsFile)
{
    //Create reader
    BinaryReader* reader = nullptr;
    defer(delete reader);
    if (packfileSourceType == DataSource::File)
        reader = new BinaryReader(path_);
    else if (packfileSourceType == DataSource::Memory)
        reader = new BinaryReader(buffer_);

    //Read metadata if it hasn't been. Reuse the reader
    if (!readMetadata_)
        ReadMetadata(reader);

    if (reader->Length() <= 2048)
        return;

    //Seek to data block
    reader->SeekBeg(dataBlockOffset_);

    //Ensure output path exists
    Path::CreatePath(outputPath);

    //Extract data to files. Pick method based on flags
    if (Compressed && Condensed)
        ExtractCompressedAndCondensed(outputPath, *reader);
    else if (Compressed)
        ExtractCompressed(outputPath, *reader);
    else
        ExtractDefault(outputPath, *reader);

    //@streams.xml contains files in packfile and their order
    if (writeStreamsFile)
        WriteStreamsFile(this, outputPath); //Todo: Consider writing this for all packfiles
}

MemoryFileList Packfile3::ExtractSubfiles(bool writeStreamsFile)
{
    //Only supports C&C files currently
    if(!(Compressed && Condensed))
        return {};

    //Create reader
    BinaryReader* reader = nullptr;
    defer(delete reader);
    if (packfileSourceType == DataSource::File)
        reader = new BinaryReader(path_);
    else if (packfileSourceType == DataSource::Memory)
        reader = new BinaryReader(buffer_);

    //Read all compressed data to buffer and inflate it
    std::vector<u8> input(Header.CompressedDataSize);
    std::vector<u8> output(Header.DataSize);
    reader->SeekBeg(dataBlockOffset_);
    reader->ReadToMemory(input.data(), Header.CompressedDataSize);
    Compression::Inflate(input, output);

    //Split output into subfiles
    std::vector<MemoryFile> files = {};
    for (u32 i = 0; i < Entries.size(); i++)
    {
        Packfile3Entry& entry = Entries[i];
        files.push_back({ EntryNames[i], entry.DataOffset, entry.DataSize });
    }

    if (writeStreamsFile)
    {
        //Construct streams file and printer
        tinyxml2::XMLDocument* doc = GetStreamsFile(this);
        defer(delete doc);
        tinyxml2::XMLPrinter printer;
        doc->Accept(&printer);

        //Append xml string to output and create MemoryFile for it
        const size_t oldSize = output.size();
        output.resize(output.size() + printer.CStrSize());
        memcpy(output.data() + oldSize, printer.CStr(), printer.CStrSize());
        files.push_back({ "@streams.xml", oldSize, (size_t)printer.CStrSize() });
    }

    return MemoryFileList(output, files);
}

void Packfile3::ExtractCompressedAndCondensed(const string& outputPath, BinaryReader& reader)
{
    //Read compressed data into a buffer and inflate it
    std::vector<u8> input(Header.CompressedDataSize);
    std::vector<u8> output(Header.DataSize);
    reader.ReadToMemory(input.data(), Header.CompressedDataSize);
    Compression::Inflate(input, output);

    //Write subfiles to drive
    for (u32 i = 0; i < Entries.size(); i++)
    {
        Packfile3Entry& entry = Entries[i];
        File::WriteToFile(outputPath + EntryNames[i], { output.data() + entry.DataOffset, entry.DataSize });
    }
}

void Packfile3::ExtractCompressed(const string& outputPath, BinaryReader& reader)
{
    //One by one, read subfiles from the packfile, inflate them, and write them to the disk
    for (u32 i = 0; i < Entries.size(); i++)
    {
        Packfile3Entry& entry = Entries[i];

        //Input/output buffers for decompression
        std::vector<u8> input(entry.CompressedDataSize);
        std::vector<u8> output(entry.DataSize);

        //Read file to buffer, decompress it, then save to drive
        reader.ReadToMemory(input.data(), entry.CompressedDataSize);
        reader.Align(2048);
        Compression::Inflate(input, output);
        File::WriteToFile(outputPath + EntryNames[i], output);
    }
}

void Packfile3::ExtractDefault(const string& outputPath, BinaryReader& reader)
{
    //Read subfiles from packfile and write them to the disk
    for (u32 i = 0; i < Entries.size(); i++)
    {
        Packfile3Entry& entry = Entries[i];
        std::vector<u8> entryData(entry.DataSize);
        reader.SeekBeg(dataBlockOffset_ + entry.DataOffset);
        reader.ReadToMemory(entryData.data(), entry.DataSize);
        File::WriteToFile(outputPath + EntryNames[i], entryData);
    }
}

void Packfile3::ReadStreamsFile(const string& inputPath, bool& compressed, bool& condensed, std::vector<std::filesystem::directory_entry>& subfilePaths)
{
    //Todo: Add actual logging and error handling. This is lame
    //Make sure the file exists and open it
    if (!std::filesystem::exists(inputPath + "\\@streams.xml"))
        throw std::runtime_error("Failed to find @streams.xml file for .str2_pc file.");

    tinyxml2::XMLDocument streams;
    streams.LoadFile((inputPath + "\\@streams.xml").c_str());

    //Get <streams> block
    tinyxml2::XMLElement* streamsBlock = streams.FirstChildElement("streams");
    if (!streamsBlock)
        throw std::runtime_error("Couldn't find <streams> block in @streams.xml file.");

    //Read attributes from <streams> block
    const tinyxml2::XMLAttribute* compressedAttrib = streamsBlock->FindAttribute("compressed");
    const tinyxml2::XMLAttribute* condensedAttrib = streamsBlock->FindAttribute("condensed");
    if (!compressedAttrib)
        throw std::runtime_error("Couldn't find attribute 'compressed' on <stream> block in @streams.xml");
    if (!condensedAttrib)
        throw std::runtime_error("Couldn't find attribute 'condensed' on <stream> block in @streams.xml");

    //Set flags from attributes
    compressed = compressedAttrib->Value() == string("True") ? true : false;
    condensed = condensedAttrib->Value() == string("True") ? true : false;

    //Read entries from <streams> block
    tinyxml2::XMLElement* entryElement = streamsBlock->FirstChildElement("entry");
    while (entryElement)
    {
        //Note: The value of the element is also a name. Using the attrib just in case there's names with restricted xml characters which may get mangled as an element value
        //Read name attribute from <entry> block
        const tinyxml2::XMLAttribute* nameAttrib = entryElement->FindAttribute("name");
        if (!compressedAttrib)
            throw std::runtime_error("Couldn't find attribute 'name' on <entry> block in @streams.xml");

        //Create new entry from name attrib
        string entryName(nameAttrib->Value());
        subfilePaths.push_back(std::filesystem::directory_entry(std::filesystem::path(inputPath + "\\" + entryName)));

        //Move to next entry
        entryElement = entryElement->NextSiblingElement("entry");
    }
}

std::optional<std::vector<u8>> Packfile3::ExtractSingleFile(s_view name, bool fullExtractFallback)
{
    //Check if single file extract is supported and the subfile exists
    u32 targetIndex = INVALID_HANDLE;
    if (((Compressed && Condensed) && !fullExtractFallback) || !Contains(name, targetIndex))
        return {};

    //Open packfile for reading
    BinaryReader* reader = nullptr;
    defer(delete reader);
    if (packfileSourceType == DataSource::File)
        reader = new BinaryReader(path_);
    else if (packfileSourceType == DataSource::Memory)
        reader = new BinaryReader(buffer_);

    Packfile3Entry& entry = Entries[targetIndex];

    //C&C files store data in one big compressed block so we must fully decompress it to extract a single file
    if (Compressed && Condensed)
    {
        //Create decompression input/output buffers
        std::vector<u8> input(Header.CompressedDataSize);
        std::vector<u8> output(Header.DataSize);

        //Read all compressed data into buffer and inflate it
        reader->SeekBeg(dataBlockOffset_);
        reader->ReadToMemory(input.data(), Header.CompressedDataSize);
        Compression::Inflate(input, output);

        //Copy data from single file into new vector
        std::vector<u8> entryData(entry.DataSize);
        memcpy(entryData.data(), output.data() + entry.DataOffset, entry.DataSize);
        return entryData;
    }
    else if (Compressed)
    {
        //Seek to file data. Compressed offset isn't stored so we calculate it by summing previous entries
        reader->SeekBeg(dataBlockOffset_);
        for (u32 i = 0; i < targetIndex; i++)
        {
            reader->Skip(Entries[i].CompressedDataSize);
            reader->Align(2048);
        }

        //Create decompression input/output buffers
        std::vector<u8> input(entry.CompressedDataSize);
        std::vector<u8> output(entry.DataSize);

        //Read the files compressed data into buffer and inflate it
        reader->ReadToMemory(input.data(), entry.CompressedDataSize);
        Compression::Inflate(input, output);

        return output;
    }
    else
    {
        //Read file into buffer and return it
        std::vector<u8> entryData(entry.DataSize);
        reader->SeekBeg(dataBlockOffset_ + entry.DataOffset);
        reader->ReadToMemory(entryData.data(), entry.DataSize);
        return entryData;
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

    //Parse each asm_pc file inside the packfile
    for (u32 i = 0; i < Entries.size(); i++)
    {
        const char* name = EntryNames[i];
        if (Path::GetExtension(name) != ".asm_pc")
            continue;

        //Extract the asm_pc file
        std::optional<std::vector<u8>> data = ExtractSingleFile(name);
        if (!data.has_value())
            continue;

        //Parse the asm_pc file
        AsmFile5& asmFile = AsmFiles.emplace_back();
        BinaryReader reader(data.value());
        asmFile.Read(reader, name);

        //Remove containers that don't have a corresponding str2_pc file. For some reason asm_pc files have these "ghost files"
        auto iterator = asmFile.Containers.begin();
        while (iterator != asmFile.Containers.end())
        {
            bool foundContainer = false;
            for (auto& entryName : EntryNames)
                if (iterator->Name + ".str2_pc" == entryName)
                    foundContainer = true;

            if (!foundContainer)
                iterator = asmFile.Containers.erase(iterator);
            else
                iterator++;
        }
    }
}

void Packfile3::Pack(const string& inputPath, const string& outputPath, bool compressed, bool condensed)
{
    //Ensure the input folder exists
    if (!std::filesystem::exists(inputPath))
        throw std::runtime_error("Input path: \"" + inputPath + "\" does not exist. Cannot pack!");
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
    std::vector<std::filesystem::directory_entry> subfilePaths = {};

    string extension = Path::GetExtension(outputPath);
    bool usingStreamsFile = (extension == ".str2_pc");
    bool isStr2 = (extension == ".str2_pc");
    if (usingStreamsFile)
        ReadStreamsFile(inputPath, compressed, condensed, subfilePaths);

    //Get list of subfile paths once. Expects that they won't change/move until packing is complete
    //Done this way so we can easily swap this out with a list from a @streams.xml file
    if (!usingStreamsFile)
    {
        for (auto& subfile : std::filesystem::directory_iterator(inputPath))
        {
            if (subfile.path().filename().string() == "@streams.xml")
                continue;

            subfilePaths.push_back(subfile);
        }
    }

    //Create entry for each file in input folder. Calc size/offset values
    u32 curSubfile = 0;
    for (auto& inFile : subfilePaths)
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

        if (compressed && condensed && curSubfile != subfilePaths.size() - 1 && !isStr2)
        {
            u32 alignPad = (u32)BinaryWriter::CalcAlign(curDataOffset, 16);
            u32 alignPad2 = (u32)BinaryWriter::CalcAlign(totalDataSize, 16);
            curDataOffset += alignPad;
            totalDataSize += alignPad2;
        }
        else if (!condensed)
            curDataOffset += (u32)out.CalcAlign(curDataOffset, 2048);

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
    dataStart += (u32)entries.size() * 28; //Each entry is 28 bytes
    dataStart += (u32)out.CalcAlign(dataStart, 2048); //Align(2048) after end of entries
    dataStart += totalNamesSize; //Filenames list
    dataStart += (u32)out.CalcAlign(dataStart, 2048); //Align(2048) after end of file names
    out.WriteNullBytes(dataStart - out.Position());

    //Write subfile data
    if (compressed && condensed)
    {
        //Todo: Try to use piecemeal deflate method with z_stream to compress data
        //Todo: There's only one zlib header
        //Todo: Each entry has a compressed size value, so can't just make one big buffer and compress it all at once
        z_stream deflateStream = {};
        deflateStream.zalloc = Z_NULL;
        deflateStream.zfree = Z_NULL;
        deflateStream.opaque = Z_NULL;
        deflateStream.avail_in = 0;
        deflateStream.next_in = nullptr;
        deflateStream.avail_out = 0;
        deflateStream.next_out = nullptr;
        deflateInit(&deflateStream, isStr2 ? Z_BEST_COMPRESSION : Z_BEST_SPEED);

        uLong lastOut = 0;
        u64 tempDataOffset = 0;
        for (u32 i = 0; i < entries.size(); i++)
        {
            Packfile3EntryExt& entry = entries[i];
            std::vector<u8> subFileData = File::ReadAllBytes(entry.FullPath);
            tempDataOffset += subFileData.size();
            //Add align(16) null bytes after uncompressed data. Not added to entry.DataSize but necessary for compression for some reason
            if (i != entries.size() - 1 && !isStr2)
            {
                u32 alignPad = (u32)BinaryWriter::CalcAlign(tempDataOffset, 16);
                if (alignPad != 0)
                {
                    tempDataOffset += alignPad;
                    for (u32 j = 0; j < alignPad; j++)
                    {
                        subFileData.push_back(0);
                    }
                }
            }

            uLong deflateUpperBound = deflateBound(&deflateStream, (uLong)subFileData.size());
            char* dest = new char[deflateUpperBound];
            defer(delete[] dest);

            deflateStream.next_in = (Bytef*)subFileData.data();
            deflateStream.avail_in = (u32)subFileData.size();
            deflateStream.next_out = (Bytef*)dest;
            deflateStream.avail_out = deflateUpperBound;
            deflate(&deflateStream, Z_SYNC_FLUSH);

            uLong entryCompressedSize = deflateStream.total_out - lastOut;
            entry.Entry.CompressedDataSize = entryCompressedSize;
            header.CompressedDataSize += entryCompressedSize;

            out.WriteFromMemory(dest, entryCompressedSize);
            out.Flush();
            lastOut = deflateStream.total_out;
        }
        deflateEnd(&deflateStream);
    }
    else if (compressed)
    {
        u32 i = 0;
        for (auto& entry : entries)
        {
            //Read subfile data and compress it
            std::vector<u8> subFileData = File::ReadAllBytes(entry.FullPath);
            Compression::DeflateResult compressedData = Compression::Deflate({ (u8*)subFileData.data(), subFileData.size() });
            defer(delete[] compressedData.Buffer);

            //Write compressed data to file
            out.WriteFromMemory(compressedData.Buffer, compressedData.DataSize);
            out.Flush();

            //Update data sizes
            entry.Entry.CompressedDataSize = (u32)compressedData.DataSize;
            header.CompressedDataSize += (u32)compressedData.DataSize;
            header.DataSize += entry.Entry.DataSize;

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
    else
    {
        u32 i = 0;
        for (auto& entry : entries)
        {
            std::vector<u8> subFileData = File::ReadAllBytes(entry.FullPath);
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
//Ignores packfiles that are compressed AND condensed since those must be fully extracted and data offsets aren't relevant in that case.
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
        if (String::EqualIgnoreCase(EntryNames[i], subfileName))
        {
            index = i;
            return true;
        }
    }

    return false;
}

tinyxml2::XMLDocument* GetStreamsFile(Packfile3* packfile)
{
    tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument;

    auto* streamsBlock = doc->NewElement("streams");
    streamsBlock->SetAttribute("endian", "Little");
    streamsBlock->SetAttribute("compressed", packfile->Compressed ? "True" : "False");
    streamsBlock->SetAttribute("condensed", packfile->Condensed ? "True" : "False");

    //Set entries
    for (u32 i = 0; i < packfile->Entries.size(); i++)
    {
        auto* entryElement = streamsBlock->InsertNewChildElement("entry");
        entryElement->SetAttribute("name", packfile->EntryNames[i]);
        entryElement->SetText(packfile->EntryNames[i]);
    }

    doc->InsertFirstChild(streamsBlock);
    return doc;
}

void WriteStreamsFile(Packfile3* packfile, const string& outputPath)
{
    tinyxml2::XMLDocument* doc = GetStreamsFile(packfile);
    doc->SaveFile((outputPath + "\\@streams.xml").c_str());
    delete doc;
}