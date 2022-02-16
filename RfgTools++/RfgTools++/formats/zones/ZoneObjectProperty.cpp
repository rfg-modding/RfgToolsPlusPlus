#include "ZoneFile.h"
#include "hashes/HashGuesser.h"

u8* ZoneObjectProperty::Data()
{
	//Assumes zone file was loaded into memory in a single buffer. Property data is right after this
	return ((u8*)this) + 8;
}

std::optional<string> ZoneObjectProperty::Name()
{
	return HashGuesser::GuessHashOriginString(NameHash);
}