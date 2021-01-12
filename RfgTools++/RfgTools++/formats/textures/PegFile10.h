#pragma once
#include "common/Typedefs.h"
#include "PegEntry10.h"
#include <optional>
#include <vector>

//Version 10 of texture files used in RFG and RFGR (cpeg_pc and cvbm_pc)
class PegFile10
{
public:
    //Same data layout as the file (24 bytes)
    u32 Signature;
    u16 Version;
    u16 Platform;
    u32 DirectoryBlockSize;
    u32 DataBlockSize;
    u16 NumberOfBitmaps;
    u16 Flags;
    u16 TotalEntries;
    u16 AlignValue;

    //Past here isn't the same data layout as the file
    std::vector<PegEntry10> Entries = {};

    //Read cpu file (cpeg/cvbm) data. Does not read out gpu file data to save on memory consumption
    void Read(BinaryReader& cpuFile, BinaryReader& gpuFile);
    //Write out new cpu file (cpeg/cvbm) and gpu file (gpeg/gvbm). If any textures have been edited their new data will be written. Must call ReadAllTextureData first
    void Write(BinaryWriter& cpuFile, BinaryWriter& gpuFile);
    //Read pixel data for single texture from gpu file. Read() must be called first
    void ReadTextureData(BinaryReader& gpuFile, PegEntry10& entry);
    //Read pixel data for all textures from gpu file. Read() must be called first. By default only reads pixel data for unedited textures
    void ReadAllTextureData(BinaryReader& gpuFile, bool overwriteEdits = false);
    //Set texture pixel data
    void SetTextureData(const string& name, std::span<u8> data);
    //Free any allocated memory
    void Cleanup();
    //Get entry index in ::Entries
    std::optional<u32> GetEntryIndex(const string& name);
    //Get entry pixel data
    std::optional<std::span<u8>> GetTextureData(const string& name);
    //Get entry pixel data
    std::optional<std::span<u8>> GetTextureData(u32 index);

private:
    bool headerRead_ = false;
};