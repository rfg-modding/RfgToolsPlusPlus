#pragma once
#include "common/Typedefs.h"
#include "AsmEnums.h"
#include "AsmPrimitive.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <vector>

//Represents data about a single str2_pc file stored in an asm_pc file
class AsmContainer
{
public:
    string Name = {};
    ContainerType Type = ContainerType::None;
    ContainerFlags Flags = ContainerFlags::None;
    u16 PrimitiveCount = 0; //Number of subfiles the str2_pc has
    u32 DataOffset = 0; //Offset to the str2_pc files data block from it's start
    u32 SizeCount = 0;
    u32 CompressedSize = 0;

    //Sizes of each str2_pc subfile
    std::vector<u32> PrimitiveSizes = {};
    //Records for each of the str2_pc subfiles
    std::vector<AsmPrimitive> Primitives = {};

    void Read(BinaryReader& reader)
    {
        //Directly reading file instead of mapping to struct since it's just simpler to do it this way in this case
        //May change it to do that later if asm reading is taking too long

        //Read container metadata
        u32 nameLength = reader.ReadUint16();
        Name = reader.ReadFixedLengthString(nameLength);
        Type = (ContainerType)reader.ReadChar();
        Flags = (ContainerFlags)reader.ReadUint16();
        PrimitiveCount = reader.ReadUint16();
        DataOffset = reader.ReadUint32();
        SizeCount = reader.ReadUint32();
        CompressedSize = reader.ReadUint32();

        //Read primitive sizes
        for (u32 i = 0; i < SizeCount; i++)
            PrimitiveSizes.push_back(reader.ReadUint32());

        //Read primitive metadata
        for (u32 i = 0; i < PrimitiveCount; i++)
        {
            AsmPrimitive& primitive = Primitives.emplace_back();
            primitive.Read(reader);
        }
    }

    void Write(BinaryWriter& out)
    {
        //Write container data
        out.WriteUint16((u16)Name.size());
        out.WriteFixedLengthString(Name); //Doesn't write a null terminator to the file
        out.WriteUint8((u8)Type);
        out.WriteUint16((u16)Flags);
        out.WriteUint16(PrimitiveCount);
        out.WriteUint32(DataOffset);
        out.WriteUint32(SizeCount);
        out.WriteUint32(CompressedSize);

        //Write primitive sizes
        for (u32 size : PrimitiveSizes)
            out.WriteUint32(size);

        //Write primitive data
        for (auto& primitive : Primitives)
            primitive.Write(out);
    }
};