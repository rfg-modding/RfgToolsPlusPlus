#pragma once
#include "common/Typedefs.h"
#include "Common/memory/Span.h"
#include "Packfile3Header.h"
#include "Packfile3Entry.h"
#include <vector>
#include <span>

#define PACKFILE_FLAG_COMPRESSED 1
#define PACKFILE_FLAG_CONDENSED 2

class Packfile3
{
public:
    Packfile3(const string& path) : path_(path) {}

    Packfile3Header Header;
    std::vector<Packfile3Entry> Entries = {};
    std::vector<const char*> EntryNames = {};
    std::vector<string> EntryNamesAlt = {};



    bool Compressed = false;
    bool Condensed = false;

    void ReadMetadata();
    void ExtractSubfiles();

private:
    string path_ = {};
    bool readMetadata_ = false;
    u8* filenamesBuffer_ = nullptr;
};