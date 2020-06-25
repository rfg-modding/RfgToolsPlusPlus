#include "ZonePc36.h"
#include "common/filesystem/Path.h"
#include "common/filesystem/File.h"
#include "common/string/String.h"
#include "hashes/HashGuesser.h"
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

    if (Name() == "p_terr01_03_12.rfgzone_pc")
    {
        auto a = 2;
    }

    //Read relational data if it's useful
    //Todo: Determine district name string from hash
    hasRelationalData_ = (Header.DistrictFlags & 5) == 0; //Todo: Get a better breakdown of all zone flag values the game cares about
    //Todo: Figure out how to interpret this data. Enable this if it's useful
    //reader.ReadToMemory(&RelationalData, sizeof(ZonePcRelationData36));
    if (hasRelationalData_)
        reader.Skip(87368); //For now we just skip it since it's not clear how to interpret that data yet

    //Don't read further if file has no objects
    if (Header.NumObjects == 0)
        return;

    Objects.reserve(Header.NumObjects);
    for (u32 i = 0; i < Header.NumObjects; i++)
    {
        u64 pos = reader.Position();
        ZoneObject36& object = Objects.emplace_back();
        object.Read(reader);
    }

    //Guess zone district name
    auto result = HashGuesser::GuessHashOriginString(Header.DistrictHash);
    districtName_ = result ? result.value() : "unknown";
}

void ZonePc36::GenerateObjectHierarchy()
{
    const u32 InvalidHandle = 0xFFFFFFFF; //Todo: Make this a global constant somewhere
    if (Objects.size() <= 1)
        return;

    //Register top level objects as first pass (objects with no parent). Done in two steps since it's simpler
    for (auto& object : Objects)
    {
        //If has no parent then register as a top level object
        if (object.Parent == InvalidHandle)
        {
            auto& hierarchyNode = ObjectsHierarchical.emplace_back();
            hierarchyNode.Self = &object;
        }
    }

    //Register objects to their parents
    for (auto& object : Objects)
    {
        //Skip top level objects as they were handled in the first pass
        if (object.Parent == InvalidHandle)
            continue;

        //Find parent
        ZoneObjectNode36* maybeParent = GetTopLevelObject(object.Parent);
        //Todo: Search in other zones/files for parents and siblings + move this step into a different class. Likely need to check matching p_ and non p_ files
        //Throw error if couldn't find parent. So far only seen single level object trees with parents in the same zone. This will detect things that don't fit that
        if (!maybeParent)
        {
            std::cout << "Error in \"" << Name() << "\". Object " << object.Handle << " could not find it's parent with handle " << object.Parent << "\n";
            break;
        }


        //Register with parent if not already done
        auto& parent = *maybeParent;
        parent.AddChildObjectIfUnique(&object);

        //Add siblings to parents if not already done
        ZoneObject36* currentObject = &object;
        while (currentObject->Sibling != InvalidHandle)
        {
            ZoneObject36* maybeSibling = GetObject(currentObject->Sibling);
            //Todo: Search in other zones/files for parents and siblings + move this step into a different class. Likely need to check matching p_ and non p_ files
            if (!maybeSibling)
            {
                std::cout << "Error in \"" << Name() << "\". Object " << currentObject->Handle << " could not find it's sibling with handle " << currentObject->Sibling << "\n";
                break;
            }

            currentObject = maybeSibling;
            parent.AddChildObjectIfUnique(currentObject);
        }
    }
}

bool ZonePc36::IsTopLevelObject(u32 handle)
{
    for (const auto& object : ObjectsHierarchical)
    {
        if (object.Self->Handle == handle)
            return true;
    }
    return false;
}

ZoneObjectNode36* ZonePc36::GetTopLevelObject(u32 handle)
{
    for (auto& object : ObjectsHierarchical)
    {
        if (object.Self->Handle == handle)
            return &object;
    }
    return nullptr;
}

ZoneObject36* ZonePc36::GetObject(u32 handle)
{
    for (auto& object : Objects)
    {
        if (object.Handle == handle)
            return &object;
    }
    return nullptr;
}
