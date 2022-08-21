# Packfile documentation
This page documents the packfile format used by Red Faction Guerrilla. The same format version is used by all versions of the game. RFA uses the same extension, but it's a newer incompatible version of the format. RFG packfiles can have two extensions, .vpp_pc and .str2_pc. They are exactly the same, however the .str2_pc files are only found within .vpp_pc files and always have their data compressed & condensed (see the header section for more info).

The files are divided into 4 sections:

- [Header](https://github.com/Moneyl/RfgToolsPlusPlus/blob/master/Documentation/Packfile.md#header)
- [Entries](https://github.com/Moneyl/RfgToolsPlusPlus/blob/master/Documentation/Packfile.md#entries)
- [Entry names](https://github.com/Moneyl/RfgToolsPlusPlus/blob/master/Documentation/Packfile.md#entry-names)
- [Entry data](https://github.com/Moneyl/RfgToolsPlusPlus/blob/master/Documentation/Packfile.md#entry-data)

## Relevant info

Before describing these sections here's some notes on the terminology used within:

- Most of the data types are a letter and a number. The letter denotes if it's unsigned, signed, or floating point. The number denotes the number of bits it is. Unsigned values start with `u`, signed values start with `i`, and floating point values start with `f`. For example, `u32` is an unsigned 32 bit integer, and `i8` is a signed 8 bit integer. 
- Assume that all strings in RFG formats are ASCII (1 byte per character) unless specified otherwise. ASCII characters will be specified with `char` in this file.
- Data sections often have padding bytes so the next section aligns to a specific byte offset. For example, `Align(2048)` means that enough padding bytes were added to align to the next integer multiple of 2048. So if your current offset were 8452 then Align(2048) would add 1788 padding bytes. Since the next integer multiple of 2048 is 10240 (5 * 2048) and 8452 + 1788 is 10240

## Header
This section describes the format version, number of files it contains, data size, data format, and more. Here's its data layout:
```
u32 Signature
u32 Version
char[65] ShortName
char[256] PathName
u8[3] Padding
u32 Flags
u8[4] Padding
u32 NumberOfEntries
u32 FileSize
u32 EntryBlockSize
u32 NameBlockSize
u32 DataSize
u32 CompressedDataSize
Align(2048)
```

* `Signature` - Signature or "magic number" for this data type. Used as a sanity check. If this doesn't equal 1367935694 the file is probably corrupt..
* `Version` - Should always be 3 for RFG.
* `ShortName` - The game seems to ignore this. It's always null.
* `PathName` - Same as ShortName. Some packers like Gibbeds write a little message in here. Might be a good spot to write packer version info for debugging.
* `Flags` - Bitflags used to specify the data format for entries. Only the first two bits are used by RFG in vanilla files. Haven't checked if setting the other bits breaks anything in the game. Bit 0 = Compressed. Bit 1 = Condensed. See the [Entry data](https://github.com/Moneyl/RfgToolsPlusPlus/blob/master/Documentation/Packfile.md#entry-data) section for info on what these do.
* `NumberOfEntries` - The number of files in the packfile.
* `FileSize` - The total size of the packfile in bytes
* `EntryBlockSize` - The size of the entry block in bytes. Doesn't include the padding bytes at the end.
* `NameBlockSize` - The size of the name block in bytes. Doesn't include the padding bytes at the end.
* `DataSize` - The size of the entry data block in bytes. Includes padding between entry data.
* `CompressedDataSize` - Size of the compressed entry data in bytes. Includes padding between entry data.


## Entries
This section has metadata for each entry in the packfile. Each entry represents a single file. This section has `NumberOfEntries` (from the header) entries. This is the data layout of a single entry:
```
u32 NameOffset
u8[4] Padding
u32 DataOffset
u32 NameHash
u32 DataSize
u32 CompressedDataSize
u8[4] Padding
```

* `NameOffset` - The byte offset of the entry name relative to the start of the entry names block

* `DataOffset` - The byte offset of the entry data relative to the start of the entry data block. When the packfile is compressed *and* condensed this is the offset after decompressing the entire entry data block. This value can be wrong in RFGR since it has packfiles larger than 4GB. If writing a packer/unpacker you should disregard this and recalculate it yourself using the data sizes of each entry.

* `NameHash` - A CRC32 hash of the entry name.

* `DataSize` - The size of the entry data in bytes. If the file is compressed this is the size after decompression.

* `CompressedDataSize` - The size of the entries compressed data. If the packfile isn't compressed this should be `0xFFFFFFFF`.

This block ends with an `Align(2048)`.

## Entry names
This block is a list of null terminated ASCII strings. It ends with `Align(2048)`.

## Entry data
This block has the actual data for each entry. The format of the data depends on the header flags. 

#### Default (no flags)
First we'll describe the data layout when neither flag is used (neither compressed nor condensed). The data for each entry is listed in the same order they're found in the entry block. After each entries data is `Align(2048)`. For example, lets say there's a vpp_pc that contains the following files:
- weapons.xtbl
- characters.xtbl
- rfg_zonex_format.txt

The entry data block starts with the contents of weapons.xtbl followed by an `Align(2048)`. Next is the contents of characters.xtbl and another `Align(2048)`. Finally is the contents of rfg_zonex_format.txt. There's no alignment padding after the last entry. Next the effect of each flag on the data layout will be described.

#### Compressed only
When this flag is used alone each entries data is compressed using default ZLIB compression. Each entry is still followed by padding bytes. The padding bytes aren't compressed in this mode. Each entry is compressed separately.

#### Condensed only
When this flag is used alone there are no padding bytes between entries.

#### Compressed & Condensed
When both flags are enabled a unique data layout is used. All the entries are compressed together into a single binary blob. To extract one file you must decompress the entire entry data block and get the file using its data offset. The decompressed data is all the entry data listed sequentially with 16 padding bytes after each entry.
