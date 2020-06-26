#include "ZoneObject36.h"
#include "ZoneObject36.h"
#include "hashes/HashGuesser.h"
#include "hashes/Hash.h"
#include "properties/PropertyManager.h"
#include <iostream>

void ZoneObject36::Read(BinaryReader& reader)
{
    //Read data from file. First 56 bytes of this struct match the file data layout
    reader.ReadToMemory(this, 56);

    //Try to guess the object classname string from the hash
    auto result = HashGuesser::GuessHashOriginString(ClassnameHash);
    Classname = result ? result.value() : "unknown";

    PropertyManager::ReadObjectProperties(*this, reader);

    ////Read object properties
    //for (u32 i = 0; i < NumProps; i++)
    //{
    //    u16 type = reader.ReadUint16();
    //    u16 size = reader.ReadUint16();
    //    u32 propertyNameHash = reader.ReadUint32();
    //    u64 propertyStartPos = reader.Position();

    //    //Attempt to read property data
    //    IZoneProperty* prop = ReadProperty(type, size, propertyNameHash);

    //    //Skip property if read failed
    //    if (!prop)
    //    {
    //        reader.SeekBeg(propertyStartPos);
    //        reader.Skip(size);
    //    }
    //    
    //    //Else add it to the list
    //    Properties.push_back(prop);
    //}
}