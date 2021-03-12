#pragma once
#include "common/Typedefs.h"
#include "AsmEnums.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>

//Data recorded about a str2_pc subfile. Stored in asm_pc files.
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

    void Write(BinaryWriter& out)
    {
        //Write primitive data
        out.WriteUint16((u16)Name.size());
        out.WriteFixedLengthString(Name); //Doesn't write a null terminator
        out.WriteUint8((u8)Type);
        out.WriteUint8((u8)Allocator);
        out.WriteUint8(Flags);
        out.WriteUint8(SplitExtIndex);
        out.WriteUint32(HeaderSize);
        out.WriteUint32(DataSize);
    }
};