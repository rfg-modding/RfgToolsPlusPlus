#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include <optional>
#include <span>

class ZoneObject;
class ZoneObjectProperty;

class ZoneFileHeader
{
public:
    u32 Signature = 0;
    u32 Version = 0;
    u32 NumObjects = 0;
    u32 NumHandles = 0;
    u32 DistrictHash = 0;
    u32 DistrictFlags = 0;
};
static_assert(sizeof(ZoneFileHeader) == 24, "sizeof(ZoneFileHeader) must be 24 to match RFG zone file format.");

//Zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR.
class ZoneFile
{
public:
    ZoneFileHeader Header;
    bool HasRelationData = false;
    u8* RelationData = nullptr;
    //Start of the zone object list. You must use NextObject() for iteration since there's variable size property blocks between objects.
    ZoneObject* Objects = nullptr;
    u32 ObjectsSize = 0;
    string Name;

    //Read from a file.
    static std::optional<ZoneFile> Read(const std::string& path);
    //Read from an in memory buffer
    static std::optional<ZoneFile> Read(std::span<u8> bytes, const std::string& name);
    //Get the object after the current one. Returns nullptr if the end was reached.
    ZoneObject* NextObject(ZoneObject* current);
    //Final zone object
    ZoneObject* LastObject();
    //Returns true if Header.NumObjects == 0
    bool Empty() { return Header.NumObjects == 0; }
    //Guess district name from Header.DistrictHash
    string DistrictName();

private:
    std::unique_ptr<u8[]> _data = nullptr;
};

//Zone object used in zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR
class ZoneObject
{
public:
    u32 ClassnameHash = 0; //Hash of the objects classname string (e.g. "rfg_mover", "cover_node", etc)
    u32 Handle = 0; //Unique identifier of the object
    Vec3 Bmin = {}; //Bounding box minimum point
    Vec3 Bmax = {}; //Bounding box maximum point
    u16 Flags = 0; //Todo: Determine flag bit values
    u16 BlockSize = 0; //Size of the objects entire data block, including these values
    u32 Parent = 0; //Parent object handle
    u32 Sibling = 0; //Sibling object handle. Forms a linked list of sibling objects by following each sibling handle.
    u32 Child = 0; //Child object handle. Can access other children by checking their sibling handle.
    u32 Num = 0; //Exact purpose unknown. Some kind of identifier used in conjunction with Handle
    u16 NumProps = 0; //Number of properties describing the object that immediately follow these values in the zone file
    u16 PropBlockSize = 0; //Size of binary blob of property data

    //Attempt to get the classname from ClassnameHash using HashGuesser
    string Classname();
    //Returns the property if the object has it, and nullptr otherwise.
    ZoneObjectProperty* GetProperty(const string& propertyName);
    bool HasProperty(const string& propertyName);

    //First and last property
    ZoneObjectProperty* Properties();
    ZoneObjectProperty* NextProperty(ZoneObjectProperty* current);
};
static_assert(sizeof(ZoneObject) == 56, "sizeof(ZoneObject) must be 56 bytes to match RFG zone file format.");

//Property of a ZoneObject (e.g. position, terrain mesh name, wind speed, etc)
class ZoneObjectProperty
{
public:
    u16 Type;
    u16 Size;
    u32 NameHash;
    u8* Data(); //Property data
    std::optional<string> Name(); //Guess name from NameHash using HashGuesser
};
static_assert(sizeof(ZoneObjectProperty) == 8, "sizeof(ZoneObjectProperty) must be 8 bytes to match RFG zone file format.");