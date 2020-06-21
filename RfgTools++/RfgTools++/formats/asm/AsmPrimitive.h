#pragma once
#include "common/Typedefs.h"
#include "AsmEnums.h"
#include <BinaryTools/BinaryReader.h>

class AsmPrimitive
{
public:
    string Name = {};
    PrimitiveType Type = PrimitiveType::None;
    AllocatorType Allocator = AllocatorType::None;
    u8 Flags = 0;
    u8 SplitExtIndex = 0;
    u32 HeaderSize = 0;
    u32 DataSize = 0;

    void Read(BinaryReader& reader)
    {
        u32 nameLength = reader.ReadUint16();
        Name = reader.ReadFixedLengthString(nameLength);
        Type = (PrimitiveType)reader.ReadChar();
        Allocator = (AllocatorType)reader.ReadChar();
        Flags = reader.ReadChar();
        SplitExtIndex = reader.ReadChar();
        HeaderSize = reader.ReadUint32();
        DataSize = reader.ReadUint32();
    }
};