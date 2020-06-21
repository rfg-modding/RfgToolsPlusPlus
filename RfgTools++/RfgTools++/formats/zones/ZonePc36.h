#pragma once
#include "common/Typedefs.h"
#include "ZonePcHeader36.h"
#include "ZoneObject36.h"
#include "ZonePcRelationData36.h"
#include <vector>

//Zone format version 36. Found in rfgzone_pc and layer_pc files in RFG and RFGR
class ZonePc36
{
public:
    ZonePc36(const string& path);

    void Read();
    void Cleanup() { delete objectsBuffer_; }
    const char* NameCstr() const { return name_.data(); }
    string Name() const { return name_; }
    bool HasRelationalData() const { return hasRelationalData_; }

    ZonePcHeader36 Header;
    //ZonePcRelationData36 RelationalData;

    std::vector<ZoneObject36> Objects = {};

private:
    string path_ = {};
    string name_ = {};

    u8* objectsBuffer_ = nullptr;
    bool hasRelationalData_ = false;
};