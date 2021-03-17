#pragma once
#include "common/Typedefs.h"
#include "types/Vec3.h"
#include "formats/zones/properties/IZoneProperty.h"
#include <BinaryTools/BinaryReader.h>
#include <memory>

//Placeholder used when a property is read that isn't supported or recognized
class UnknownTypeProperty : public IZoneProperty
{
public:
    //Raw data buffer used to preserve property data. Allows editing and resaving zone data without supporting all property types.
    std::unique_ptr<u8[]> buffer_ = nullptr;

    bool Read(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
    {
        Type = type;
        NameHash = nameHash;
        DataType = ZonePropertyType::Unknown;
        
        //Guess name with name hash
        auto result = HashGuesser::GuessHashOriginString(nameHash);
        Name = result ? result.value() : "unknown";
        
        //Read property data to buffer to conserve it
        buffer_ = std::make_unique<u8[]>(size);
        reader.ReadToMemory(buffer_.get(), size);

        return true;
    }
};