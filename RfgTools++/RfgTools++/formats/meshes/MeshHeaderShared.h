#pragma once
#include "common/Typedefs.h"
#include <BinaryTools/BinaryReader.h>

//Common mesh header used by static and character meshes (csmesh_pc and ccmesh_pc)
class MeshHeaderShared
{
public:
    u32 Signature;
    u32 Version;
    u32 MeshOffset;
    u32 MaterialMapOffset;
    u32 MaterialsOffset;
    u32 NumMaterials;
    u32 TextureNamesOffset;

    void Read(BinaryReader& cpuFile, u32 expectedSignature, u32 expectedVersion)
    {
        Signature = cpuFile.ReadUint32();
        if (Signature != expectedSignature)
            throw std::runtime_error("Error! Invalid static mesh signature. Expected value is " + std::to_string(expectedSignature) + ". The detected signature is " + std::to_string(Signature));

        Version = cpuFile.ReadUint32();
        if (Version != 4 && Version != 5)
            throw std::runtime_error("Error! Invalid static mesh version. Expected versions 4 or 5. The detected version is " + std::to_string(Version));

        //Read header
        //Shared mesh header
        if (Version == 4) //Used by RFG steam edition
        {
            MeshOffset = cpuFile.ReadUint32();
            MaterialMapOffset = cpuFile.ReadUint32();
            MaterialsOffset = cpuFile.ReadUint32();
            NumMaterials = cpuFile.ReadUint32();
            TextureNamesOffset = cpuFile.ReadUint32();
        }
        else if (Version == 5) //Used by RFGR
        {
            MeshOffset = cpuFile.ReadUint32();
            cpuFile.Skip(4);
            MaterialMapOffset = cpuFile.ReadUint32();
            cpuFile.Skip(4);
            MaterialsOffset = cpuFile.ReadUint32();
            cpuFile.Skip(4);
            NumMaterials = cpuFile.ReadUint32();
            cpuFile.Skip(4);
            TextureNamesOffset = cpuFile.ReadUint32();
            cpuFile.Skip(4);
        }
    }
};