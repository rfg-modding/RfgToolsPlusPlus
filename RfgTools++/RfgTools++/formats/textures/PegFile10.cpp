#include "PegFile10.h"

void PegFile10::Read(BinaryReader& cpuFile, BinaryReader& gpuFile)
{
    //Read the header. First 24 bytes of PegFile10 match data layout of peg file
    cpuFile.ReadToMemory(this, 24);

    if (Signature != 1447773511) //Equals GEKV as a string
        throw std::runtime_error("Header signature must be GEKV. Invalid peg file. Make sure that your packfile extractor didn't incorrectly extract the peg file you're trying to open.");
    if (Version != 10)
        throw std::runtime_error("Unsupported peg format version detected! Only peg version 10 is supported." + string("Version ") + std::to_string(Version) + "was detected");
    if (cpuFile.Length() != DirectoryBlockSize)
        throw std::runtime_error("Error, the size of the header file (cpeg_pc or cvbm_pc) does not match the size value stored in the header! Actual size: " + std::to_string(cpuFile.Length()) + "bytes, stored size: " + std::to_string(DirectoryBlockSize) + "bytes.");

    for (u32 i = 0; i < NumberOfBitmaps; i++)
    {
        PegEntry10& entry = Entries.emplace_back();
        entry.Read(cpuFile);
    }
    for (auto& entry : Entries)
    {
        entry.Name = cpuFile.ReadNullTerminatedString();
    }

    headerRead_ = true;
}

void PegFile10::Write(BinaryWriter& cpuFile, BinaryWriter& gpuFile)
{
    if (!headerRead_)
        return;

    
}

void PegFile10::ReadTextureData(BinaryReader& gpuFile, PegEntry10& entry)
{
    //If header not read or raw data read previously return
    if (!headerRead_ || !entry.RawData.empty())
        return;

    gpuFile.SeekBeg(0);
    gpuFile.Skip(entry.DataOffset);

    u8* data = new u8[entry.FrameSize];
    gpuFile.ReadToMemory(data, entry.FrameSize);
    entry.RawData = std::span<u8>(data, entry.FrameSize);
}

void PegFile10::ReadAllTextureData(BinaryReader& gpuFile, bool overwriteEdits)
{
    if (!headerRead_)
        return;

    for (auto& entry : Entries)
    {
        if (entry.Edited && !overwriteEdits)
            continue;

        ReadTextureData(gpuFile, entry);
    }
}

void PegFile10::SetTextureData(const string& name, std::span<u8> data)
{
    if (!headerRead_)
        return;

    auto index = GetEntryIndex(name);
    if (!index)
        return;

    Entries[index.value()].RawData = data;
    Entries[index.value()].Edited = true;
}

void PegFile10::Cleanup()
{
    for (auto& entry : Entries)
    {
        if (!entry.RawData.data())
        {
            delete[] entry.RawData.data();
        }
    }
}

std::optional<u32> PegFile10::GetEntryIndex(const string& name)
{
    for (u32 i = 0; i < Entries.size(); i++)
    {
        PegEntry10& entry = Entries[i];
        if (entry.Name == name)
            return i;
    }
    return {};
}

std::optional<std::span<u8>> PegFile10::GetTextureData(const string& name)
{
    auto index = GetEntryIndex(name);
    if (!index)
        return {};

    return GetTextureData(index.value());
}

std::optional<std::span<u8>> PegFile10::GetTextureData(u32 index)
{
    if (index >= Entries.size())
        return {};

    return Entries[index].RawData;
}