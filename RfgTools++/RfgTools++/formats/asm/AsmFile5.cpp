#include "AsmFile5.h"
#include <BinaryTools/BinaryWriter.h>
#include "common/string/String.h"

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
