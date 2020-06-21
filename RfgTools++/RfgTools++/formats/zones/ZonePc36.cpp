#include "ZonePc36.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include <BinaryTools/BinaryReader.h>
#include <filesystem>
#include <iostream>

void ZonePc36::Read(BinaryReader& reader)
{
    //Read and validate header
    reader.ReadToMemory(&Header, sizeof(ZonePcHeader36));
    if (Header.Signature != 1162760026) //Should equal ascii value for "ZONE"
        throw std::exception(("Error! Invalid zone file signature. Expected 1162760026, detected " + std::to_string(Header.Signature)).c_str());
    if (Header.Version != 36) //Only have seen and reversed version 36
        throw std::exception(("Error! Invalid zone file version. Expected 36, detected " + std::to_string(Header.Version)).c_str());

    //Read relational data if it's useful
    //Todo: Determine district name string from hash
    hasRelationalData_ = (Header.DistrictFlags & 5) == 0; //Todo: Get a better breakdown of all zone flag values the game cares about
    //Todo: Figure out how to interpret this data. Enable this if it's useful
    //reader.ReadToMemory(&RelationalData, sizeof(ZonePcRelationData36));
    reader.Skip(87368); //For now we just skip it since it's not clear how to interpret that data yet

    Objects.reserve(Header.NumObjects);
    for (u32 i = 0; i < Header.NumObjects; i++)
    {
        ZoneObject36& object = Objects.emplace_back();
        reader.ReadToMemory(&object, sizeof(ZoneObject36));
        reader.Skip(object.PropBlockSize);
    }
}