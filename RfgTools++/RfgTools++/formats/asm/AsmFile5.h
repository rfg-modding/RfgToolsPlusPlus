#pragma once
#include "common/Typedefs.h"
#include "AsmContainer.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>

class AsmFile5
{
public:
    void Read(BinaryReader& reader, const string& name);
    void Write(BinaryWriter& out);
    void Write(const string& path);
    bool HasContainer(const string& name);
    AsmContainer* GetContainer(const string& name);

    string Name;
    u32 Signature = 0;
    u16 Version = 0;
    u16 ContainerCount = 0;

    std::vector<AsmContainer> Containers = {};

private:

};