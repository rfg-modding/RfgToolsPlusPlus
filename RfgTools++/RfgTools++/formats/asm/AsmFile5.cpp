#include "AsmFile5.h"
#include <BinaryTools/BinaryWriter.h>
#include "common/string/String.h"
#include "RfgTools++/formats/packfiles/Packfile3.h"
#include "common/filesystem/Path.h"

void AsmFile5::Read(BinaryReader& reader, const string& name)
{
    Name = name;
    Signature = reader.ReadUint32();
    Version = reader.ReadUint16();
    ContainerCount = reader.ReadUint16();

    if (Signature != 3203399405)
        throw std::exception(("Error! Invalid asm file signature. Expected 3203399405, detected " + std::to_string(Signature)).c_str());
    if (Version != 5) //Only have seen and reversed version 36
        throw std::exception(("Error! Invalid asm file version. Expected 5, detected " + std::to_string(Version)).c_str());

    for (u32 i = 0; i < ContainerCount; i++)
    {
        AsmContainer& container = Containers.emplace_back();
        container.Read(reader);
    }
}

void AsmFile5::Write(BinaryWriter& out)
{
    //Write header
    out.WriteUint32(Signature);
    out.WriteUint16(Version);
    out.WriteUint16(ContainerCount);

    //Write container data
    for (auto& container : Containers)
        container.Write(out);
}

void AsmFile5::Write(const string& path)
{
    BinaryWriter out(path);
    Write(out);
}

bool AsmFile5::HasContainer(const string& name)
{
    string nameLower = String::ToLower(name);
    for (auto& container : Containers)
        if (nameLower == String::ToLower(container.Name))
            return true;

    return false;
}

AsmContainer* AsmFile5::GetContainer(const string& name)
{
    string nameLower = String::ToLower(name);
    for (auto& container : Containers)
        if (nameLower == String::ToLower(container.Name))
            return &container;

    return nullptr;
}

bool AsmFile5::UpdateContainer(const Packfile3& str2)
{
    const string str2NameNoExt = Path::GetFileNameNoExtension(str2.Name());

    //Get container
    AsmContainer* container = GetContainer(Path::GetFileNameNoExtension(str2.Name()));
    if (!container)
        return false;

    //Update container + primitive sizes
    container->CompressedSize = str2.Header.CompressedDataSize;
    for (u32 i = 0; i < container->PrimitiveSizes.size(); i++)
        container->PrimitiveSizes[i] = str2.Entries[i].DataSize; //Uncompressed size

    //Update primitive values
    u32 primIndex = 0;
    u32 primSizeIndex = 0;
    while (primIndex < container->Primitives.size())
    {
        AsmPrimitive& curPrim = container->Primitives[primIndex];

        //If DataSize = 0 assume this primitive has no gpu file
        //Todo: This assumption blocks support of adding/remove files from str2s or reordering them
        if (curPrim.DataSize == 0)
        {
            curPrim.HeaderSize = str2.Entries[primSizeIndex].DataSize; //Uncompressed size
            primIndex++;
            primSizeIndex++;
        }
        else //Otherwise assume primitive has cpu and gpu file
        {
            curPrim.HeaderSize = str2.Entries[primSizeIndex].DataSize; //Cpu file uncompressed size
            curPrim.DataSize = str2.Entries[primSizeIndex + 1].DataSize; //Gpu file uncompressed size
            primIndex++;
            primSizeIndex += 2;
        }
    }

    return true;
}