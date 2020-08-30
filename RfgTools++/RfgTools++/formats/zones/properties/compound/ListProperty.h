#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>

template <class T>
class ListProperty : public IZoneProperty
{
public:
    std::vector<T> Data = {};

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        DataType = ZonePropertyType::List;

        const u32 typeSize = sizeof(T);
        if (size % typeSize != 0)
            throw std::runtime_error("Size of ListProperty<T> is not divisible by the size of it's type " + string(typeid(T).name()));

        const u32 elementCount = size / typeSize;
        for (u32 i = 0; i < elementCount; i++)
        {
            T temp;
            reader.ReadToMemory(&temp, sizeof(T));
            Data.push_back(temp);
        }
        return true;
    }
};