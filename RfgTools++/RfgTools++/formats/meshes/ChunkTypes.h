#pragma once
#include "RfgTools++/types/Vec3.h"
#include "RfgTools++/types/Mat3.h"
#include <vector>

struct ChunkHeader
{
    u32 Signature = 0;
    u32 Version = 0;
    u32 SourceVersion = 0;
    u32 RenderDataChecksum = 0;
    u32 RenderCpuDataOffset = 0;
    u32 RenderCpuDataSize = 0;
    u32 CollisionModelChecksum = 0;
    u32 CollisionModelDataOffset = 0;
    u32 CollisionModelDataSize = 0;
    u32 DestructionChecksum = 0;
    u32 DestructionOffset = 0;
    u32 DestructionDataSize = 0;
};
//TODO: Make a macro that does this + adds the struct name & expected size to the error message. No point in duplicating the string dozens of times
static_assert(sizeof(ChunkHeader) == 48, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct ChunkSnapPoint
{
    Mat3 Orient;
    Vec3 Position;
};
static_assert(sizeof(ChunkSnapPoint) == 48, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct Subpiece
{
    Vec3 Bmin;
    Vec3 Bmax;
    Vec3 Position;
    Vec3 CenterOfMass;
    f32 Mass;
    u32 DlodKey;
    u32 LinksOffset; //ushort offset
    u8 PhysicalMaterialIndex;
    u8 ShapeType;
    u8 NumLinks;
    u8 Flags;
};
static_assert(sizeof(Subpiece) == 64, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct SubpieceData
{
    u32 ShapeOffset; //havok shape offset
    u16 CollisionModel;
    u16 RenderSubpiece;
    u32 Unknown0;
};
static_assert(sizeof(SubpieceData) == 12, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct Link
{
    i32 YieldMax;
    f32 Area;
    i16 Obj[2];
    u8 Flags;
};
static_assert(sizeof(Link) == 16, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct Dlod
{
    u32 NameHash;
    Vec3 Pos;
    Mat3 Orient;
    u16 RenderSubpiece;
    u16 FirstPiece;
    u8 MaxPieces;
};
static_assert(sizeof(Dlod) == 60, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct RbbAabb
{
    i16 MinX;
    i16 MinY;
    i16 MinZ;
    i16 MaxX;
    i16 MaxY;
    i16 MaxZ;
};
static_assert(sizeof(RbbAabb) == 12, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct RbbNode
{
    int NumObjects;
    RbbAabb Aabb;
    u32 NodeDataOffset; //et_ptr_offset<unsigned char, 0> node_data;
};
static_assert(sizeof(RbbNode) == 20, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct DestroyableInstanceData
{
    unsigned int ObjectsOffset;
    unsigned int LinksOffset;
    unsigned int DlodsOffset;
    unsigned int DataSize;
    u32 BufferOffset; //et_ptr_offset<unsigned char, 0> buffer;
};
static_assert(sizeof(DestroyableInstanceData) == 20, "Structs compiled size doesn't match its size in RFG. Required for proper loading.");

struct Destroyable
{
    //TODO: Remove data we won't need after reading (like NumX which can be grabbed from vector), or which isn't set until runtime (probably most of the offsets)
    u32 AabbTreeOffset; //rfg_rbb_node offset
    u32 ObjectsOffset; //rfg_subpiece_base offset
    u32 ExtraDataOffset; //rfg_subpiece_base_extra_data offset
    int NumObjects;
    u32 BaseLinksOffset; //rfg_links_base offset
    int NumLinks;
    u32 DlodsOffset; //rfg_dlod_base offset
    int NumDlods;
    u32 InstanceDataOffset; //rfg_destroyable_base_instance_data offset
    u32 TransformBufferOffset; //unsigned char buffer offset
    f32 Mass;

    std::vector<Subpiece> Subpieces;
    std::vector<SubpieceData> SubpieceData;
    std::vector<Link> Links;
    std::vector<Dlod> Dlods;

    //Note: These aren't read by ChunkMesh::Read(). Chunk format hasn't been 100% reversed yet.
    std::vector<RbbNode> RbbNodes;
    DestroyableInstanceData InstanceData;
    
    //Additional data stored in a separate part of the chunk file
    u32 UID;
    string Name;
    u32 IsDestroyable;
    u32 NumSnapPoints;
    ChunkSnapPoint SnapPoints[10];
};