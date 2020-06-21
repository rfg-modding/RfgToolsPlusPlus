#pragma once
#include "common/Typedefs.h"
#include "ZonePcHeader36.h"
#include "ZoneObject36.h"
#include "ZonePcRelationData36.h"
#include <BinaryTools/BinaryReader.h>
#include <vector>

//Zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR
class ZonePc36
{
public:
    void Read(BinaryReader& reader);

    void Cleanup() { delete objectsBuffer_; }
    const char* NameCstr() const { return name_.data(); }
    string Name() const { return name_; }
    void SetName(const string& name) { name_ = name; }
    bool HasRelationalData() const { return hasRelationalData_; }

    ZonePcHeader36 Header;
    //ZonePcRelationData36 RelationalData;

    std::vector<ZoneObject36> Objects = {};

private:
    string name_ = {};

    u8* objectsBuffer_ = nullptr;
    bool hasRelationalData_ = false;
};