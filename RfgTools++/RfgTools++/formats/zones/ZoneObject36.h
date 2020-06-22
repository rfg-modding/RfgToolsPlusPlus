#pragma once
#include "common/Typedefs.h"
#include "RfgTools++/types/Vec3.h"
#include "hashes/HashGuesser.h"
#include <BinaryTools/BinaryReader.h>

//Zone object used in zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR
class ZoneObject36
{
public:
    //First 56 bytes of this class are the exact data layout found in zone files
    u32 ClassnameHash = 0;
    u32 Handle = 0;
    Vec3 Bmin = {};
    Vec3 Bmax = {};
    u16 Flags = 0;
    u16 BlockSize = 0; //Size of the objects entire data block, including these values
    u32 Parent = 0;
    u32 Sibling = 0;
    u32 Child = 0;
    u32 Num = 0;  
    u16 NumProps = 0; //Number of properties describing the object that immediately follow these values in the zone file
    u16 PropBlockSize = 0; //Size of just the prop data
    //Followed by properties list. Will store separately so this can be directly memory mapped

    //Everything after this point isn't part of the file data layout
    string Classname;

    void Read(BinaryReader& reader)
    {
        //Read data from file. First 56 bytes of this struct match the file data layout
        reader.ReadToMemory(this, 56);
        //Todo: Read and parse property data //For now just skipping it because that's gonna be a lot of work
        reader.Skip(PropBlockSize);

        //Try to guess the object classname string from the hash
        auto result = HashGuesser::GuessHashOriginString(ClassnameHash);
        Classname = result ? result.value() : "unknown";
    }
};