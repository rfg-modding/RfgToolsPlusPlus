#include "ZoneFile.h"
#include <BinaryTools/BinaryWriter.h>
#include "hashes/HashGuesser.h"

std::optional<string> ZoneObject::Classname()
{
	return HashGuesser::GuessHashOriginString(ClassnameHash);
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