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

    //Read properties from binary blob at the end of the object
    PropertyManager::ReadObjectProperties(*this, reader);
}