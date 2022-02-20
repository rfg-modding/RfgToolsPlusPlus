#include "ZoneFile.h"
#include <BinaryTools/BinaryWriter.h>
#include "hashes/HashGuesser.h"

string ZoneObject::Classname()
{
	std::optional<string> maybeClassname = HashGuesser::GuessHashOriginString(ClassnameHash);
	if (maybeClassname)
		return maybeClassname.value();
	else
		return "Unknown";
}

ZoneObjectProperty* ZoneObject::GetProperty(const string& propertyName)
{
	ZoneObjectProperty* prop = Properties();
	while (prop)
	{
		if (auto name = prop->Name(); name && name.value() == propertyName)
			return prop;
		prop = NextProperty(prop);
	}

	return nullptr;
}

bool ZoneObject::HasProperty(const string& propertyName)
{
	return GetProperty(propertyName) != nullptr;
}

ZoneObjectProperty* ZoneObject::Properties()
{
	return (ZoneObjectProperty*)(((u8*)this) + sizeof(ZoneObject));
}

ZoneObjectProperty* ZoneObject::NextProperty(ZoneObjectProperty* current)
{
	u8* data = current->Data();
	u8* dataEnd = data + current->Size + BinaryWriter::CalcAlign(current->Size, 4);
	return (ZoneObjectProperty*)(dataEnd);
}