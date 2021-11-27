#pragma once
#include "common/Typedefs.h"
#include "AsmContainer.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>

class Packfile3;

//Version 5 of asm_pc files used by RFG and RFGR.
//These files are used as a table of contents for str2_pc files.
//Any time a str2_pc file is edited the asm_pc file which references it must also be updated.
//The game uses asm_pc files to determine the contents of str2_pc files despite them having metadata about their contents.
class AsmFile5
{
public:
    void Read(BinaryReader& reader, const string& name);
    void Write(BinaryWriter& out);
    void Write(const string& path);
    bool HasContainer(const string& name);
    AsmContainer* GetContainer(const string& name);
    //Update container with the same name as the provided packfile. Used to update asm_pc primitive + container sizes when a str2_pc it references changes.
    //Note: Does not support adding/removing/reordering containers yet.
    bool UpdateContainer(const Packfile3& str2);

    string Name;
    u32 Signature = 0;
    u16 Version = 0;
    u16 ContainerCount = 0;

    std::vector<AsmContainer> Containers = {};

private:

};