#include "ZoneFile.h"
#include "common/filesystem/File.h"
#include "common/filesystem/Path.h"
#include "hashes/HashGuesser.h"
#include <filesystem>

std::optional<ZoneFile> ZoneFile::Read(const std::string& path)
{
	//Read entire file into memory
	if (std::filesystem::exists(path))
	{
		std::vector<u8> bytes = File::ReadAllBytes(path);
		return ZoneFile::Read(bytes, Path::GetFileName(path));
	}
	else
	{
		return {};
	}
}

std::optional<ZoneFile> ZoneFile::Read(std::span<u8> bytes, const std::string& name)
{
	size_t sizeBytes = bytes.size_bytes();
	u8* start = bytes.data();
	u8* end = start + sizeBytes;

	//Take ownership of buffer
	ZoneFile zoneFile;
	zoneFile.Name = name;
	zoneFile._data = std::make_unique<u8[]>(sizeBytes);
	memcpy(zoneFile._data.get(), start, sizeBytes);

	//Read header
	memcpy(&zoneFile.Header, start, sizeof(ZoneFileHeader));
	u8* pos = start + sizeof(ZoneFileHeader);

	//Locate object list & other data blocks
	zoneFile.HasRelationData = (zoneFile.Header.DistrictFlags & 5) == 0;
	if (zoneFile.HasRelationData)
	{
		zoneFile.RelationData = pos;
		pos += 87368;
	}
	if (zoneFile.Empty())
	{
		zoneFile.Objects = nullptr;
		zoneFile.ObjectsSize = 0;
	}
	else
	{
		zoneFile.Objects = (ZoneObject*)pos;
		zoneFile.ObjectsSize = (u32)(end - pos);
	}

	return zoneFile;
}

ZoneObject* ZoneFile::NextObject(ZoneObject* current)
{
	if (current == LastObject())
	{
		return nullptr;
	}
	else
	{
		u8* cur = (u8*)current; //Convert to u8* for byte offseting
		u8* propBlockStart = cur + sizeof(ZoneObject); //Jump to start of prop block
		u8* next = propBlockStart + current->PropBlockSize; //Skip prop block. Next object follows
		return (ZoneObject*)next;
	}
}

ZoneObject* ZoneFile::LastObject()
{
	u8* current = (u8*)Objects;
	u32 i = 0;
	while (current && i < Header.NumObjects - 1)
	{
		current += sizeof(ZoneObject) + ((ZoneObject*)current)->PropBlockSize;
		i++;
	}

	return (ZoneObject*)current;
}

string ZoneFile::DistrictName()
{
	std::optional<string> maybeName = HashGuesser::GuessHashOriginString(Header.DistrictHash);
	if (maybeName)
		return maybeName.value();
	else
		return "Unknown";
}