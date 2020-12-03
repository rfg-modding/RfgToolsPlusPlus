#pragma once
#include "common/Typedefs.h"
#include "RfgTools++/types/Vec3.h"
#include "hashes/HashGuesser.h"
#include "properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>
#include <iostream>

//Zone object used in zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR
class ZoneObject36
{
public:
    //Attempts to get a property from the zone object. Returns nullptr if it fails.
    template <class T = IZoneProperty>
    T* GetProperty(const string& propertyName)
    {
        for (auto& prop : Properties)
        {
            if (prop && prop->Name == propertyName)
                return reinterpret_cast<T*>(prop);
        }
        return nullptr;
    }
    bool HasProperty(const string& propertyName)
    {
        return GetProperty(propertyName) != nullptr;
    }

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

    //Everything after this point isn't part of the exact file data layout
    string Classname;
    std::vector<IZoneProperty*> Properties = {};

    void Read(BinaryReader& reader);
};