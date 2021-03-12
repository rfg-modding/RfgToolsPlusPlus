#pragma once
#include "common/Typedefs.h"
#include "TextureFlags.h"
#include "PegFormat.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <span>

//Entry for a sub-texture of peg file
class PegEntry10
{
public:
    //Data that's really in the file (48 bytes)
    u32 DataOffset;
    u16 Width;
    u16 Height;
    PegFormat BitmapFormat;
    u16 SourceWidth;
    u16 AnimTilesWidth;
    u16 AnimTilesHeight;
    u16 NumFrames;
    TextureFlags Flags;
    u32 FilenameOffset;
    u16 SourceHeight;
    u8 Fps;
    u8 MipLevels;
    u32 FrameSize;
    u32 Next; //Runtime only, can ignore
    u32 Previous; //Runtime only, can ignore
    u32 Cache0; //Runtime only, can ignore
    u32 Cache1; //Runtime , can ignore

    //Extra data that's not in the file
    string Name;
    std::span<u8> RawData;
    bool Edited = false; 

    void Read(BinaryReader& reader)
    {
        reader.ReadToMemory(this, 48);
    }

    void Write(BinaryWriter& out)
    {
        out.WriteFromMemory(this, 48);
    }
};