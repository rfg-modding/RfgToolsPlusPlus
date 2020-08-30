#pragma once
#include "common/Typedefs.h"
#include "ZonePcHeader36.h"
#include "ZoneObject36.h"
#include "ZonePcRelationData36.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>
#include <optional>

//Todo: Move this + hierarchy code out of ZonePc36 into it's own class for managing zones & zone data
class ZoneObjectNode36
{
public:
    void AddChildObject(ZoneObject36* childPtr)
    {
        Children.push_back(ZoneObjectNode36{ childPtr, this, {} });
    }
    
    void AddChildObjectIfUnique(ZoneObject36* childPtr)
    {
        //Make sure the child object hasn't already been registered
        for (auto& child : Children)
        {
            if (child.Self == childPtr)
                return;
        }

        //If unique then add it
        Children.push_back(ZoneObjectNode36{ childPtr, this, {} });
    }
    
    ZoneObject36* Self = nullptr;
    ZoneObjectNode36* Parent = nullptr;
    std::vector<ZoneObjectNode36> Children = {};
    bool Selected = false;
};

//Zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR
class ZonePc36
{
public:
    void Read(BinaryReader& reader);

    //Todo: The hierarchy will need to be continually updated if the number of zone objects changes since that possibly resizes the vector thus invalidating the hierarchy pointers
    //Generate a hierarchical view of the zone objects. Has pointers to parent objects and their children 
    //to reduce the number of object lookups and make it easier to iterate objects and their relatives.
    void GenerateObjectHierarchy();
    //Checks if an object has no parents and is registered in the object hierarchy
    bool IsTopLevelObject(u32 handle);
    //Get top level zone object from hierarchical view
    ZoneObjectNode36* GetTopLevelObject(u32 handle);

    void Cleanup() { delete objectsBuffer_; }
    const char* NameCstr() const { return name_.c_str(); }
    string Name() const { return name_; }
    void SetName(const string& name) { name_ = name; }
    const char* DistrictNameCstr() { return districtName_.c_str(); }
    string DistrictName() { return districtName_; }
    bool HasRelationalData() const { return hasRelationalData_; }
    //Get an object from it's handle. Does not look in the hierarchical view. Just looks at flat list of objects
    ZoneObject36* GetObject(u32 handle);
    //Get first object of the provided type.
    ZoneObject36* GetSingleObject(const string& typeString);

    ZonePcHeader36 Header;
    //ZonePcRelationData36 RelationalData;

    std::vector<ZoneObject36> Objects = {};
    std::vector<ZoneObjectNode36> ObjectsHierarchical = {};

private:

    string name_;
    string districtName_;

    u8* objectsBuffer_ = nullptr;
    bool hasRelationalData_ = false;
};