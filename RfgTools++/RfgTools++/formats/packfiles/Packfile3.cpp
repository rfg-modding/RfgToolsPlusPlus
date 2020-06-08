#include "Packfile3.h"
#include <BinaryTools/BinaryReader.h>
#include <iostream>

void Packfile3::ReadMetadata()
{
    //Create BinaryReader to help read in file data
    BinaryReader reader(path_); //Todo: Reuse reader when calling this from ExtractSubfiles()

    //Read header directly into memory
    reader.ReadToMemory(&Header, sizeof(Packfile3Header));
    //Skip junk data + alignment padding
    reader.Align(2048);

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

    //Todo: Double check this isn't actually making a second copy somehow
    //Read the entries directly into the vector and update it's size via ::assign(). Align(2048) to reach filename block
    reader.ReadToMemory(Entries.data(), Header.NumberOfSubfiles * sizeof(Packfile3Entry));
    Entries.assign(Entries.data(), Entries.data() + Header.NumberOfSubfiles); //This seems a bit hacky
    reader.Align(2048);

    //Read filenames into heap buffer
    filenamesBuffer_ = new u8[Header.NameBlockSize];
    reader.ReadToMemory(filenamesBuffer_, Header.NameBlockSize);
    reader.Align(2048); //Align to reach next data block start
    
    //Make array of pointers to each string for easy access. Actual string data is still held in single heap buffer
    //Note: Tested using std::string and it took about twice as long due to copying + more allocating. Keep in mind if ever want to switch this to use std::string
    EntryNames.push_back((const char*)filenamesBuffer_);
    for (int i = 0; i < Header.NameBlockSize - 1; i++)
    {
        char val = filenamesBuffer_[i];
        if (val == '\0')
            EntryNames.push_back((const char*)filenamesBuffer_ + i + 1);
    }

    u64 b = reader.Position();
    readMetadata_ = true;
}

void Packfile3::ExtractSubfiles()
{
    if (!readMetadata_)
        ReadMetadata();


}