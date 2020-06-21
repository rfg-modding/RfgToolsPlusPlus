#include "AsmFile5.h"

void AsmFile5::Read(BinaryReader& reader)
{
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