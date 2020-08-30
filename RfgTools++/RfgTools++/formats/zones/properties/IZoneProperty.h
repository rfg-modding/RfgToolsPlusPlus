#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>

//Todo: Try to find an alternative to this, or see if it's possible to remove the need from it. Likely only needed in a few circumstances
//Todo: but it's not been used enough at this point for that to be clear.
//Set by each concrete zone property type (implementations of IZoneProperty). Used to identify them to extract data from them.
//This is likely very error prone so you should check carefully what data type a property is before reading its data directly.
enum class ZonePropertyType
{
    None,
    String,
    Bool,
    Float,
    Uint,
    BoundingBox,
    ConstraintTemplate,
    Matrix33,
    Vec3,
    DistrictFlags,
    NavpointData,
    List,
    Op
};

class IZoneProperty
{
public:
    virtual bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash) = 0;

    //Todo: If memory usage gets out of control could store all name strings in PropertyManager instead of per property
    string Name;
    u32 Type = 0;
    u32 NameHash = 0;
    ZonePropertyType DataType = ZonePropertyType::None;
};