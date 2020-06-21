#pragma once
#include "common/Typedefs.h"
#include "AsmContainer.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>

class AsmFile5
{
public:
    void Read(BinaryReader& reader);

    u32 Signature = 0;
    u16 Version = 0;
    u16 ContainerCount = 0;

    std::vector<AsmContainer> Containers = {};

private:

};