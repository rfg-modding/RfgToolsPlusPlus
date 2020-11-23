#pragma once
#include "common/Typedefs.h"
#include "MeshHeaderShared.h"
#include "SubmeshData.h"
#include "RenderBlock.h"
#include "MeshEnums.h"
#include "MaterialBlock.h"
#include "VertexBufferData.h"
#include "IndexBufferData.h"
#include <BinaryTools/BinaryReader.h>
#include <BinaryTools/BinaryWriter.h>
#include <filesystem>
#include <optional>
#include <vector>
#include <span>

enum material_shape_type
{
    INVALID_MATERIAL_EVENT_SHAPE_TYPE = 0xFFFFFFFF,
    MATERIAL_EVENT_SHAPE_SOLID = 0x0,
    MATERIAL_EVENT_SHAPE_SHEET = 0x1,
    MATERIAL_EVENT_SHAPE_POLE = 0x2,
    MATERIAL_EVENT_SHAPE_FENCE = 0x3,
    MATERIAL_EVENT_SHAPE_CATWALK = 0x4,
    NUM_MATERIAL_EVENT_SHAPE_TYPES = 0x5,
};
struct rl_color_float
{
    float red;
    float green;
    float blue;
    float alpha;
};
struct base_event_candidate
{
    void* vfptr;//base_event_candidateVtbl* vfptr;
    float event_size[3];
};
struct __declspec(align(4)) gust_effect_info
{
    unsigned int effect;
    float fade_time;
    float percent_chance;
    bool near_far;
};
struct rfg_effect_material
{
    char name[32];
    material_shape_type default_shape_type;
    rl_color_float effect_tint;
    bool effect_tint_enabled;
    base_event_candidate* effect_candidate[13];
    int blast_decal;
    unsigned int trail_effects[2];
    int num_gust_effects;
    gust_effect_info gust_effects[10];
    bool driving_do_tracks;
    unsigned int driving_front_effect;
    unsigned int driving_rear_effect;
};
struct rfg_physical_material
{
    char name[32];
    unsigned int name_checksum;
    char suffix[3];
    unsigned int suffix_checksum;
    u32 base_props_name_ptr; //char* base_props_name;
    float density;
    float brittleness;
    float restitution;
    float friction;
    unsigned int link_strength;
    u32 effect_material_ptr; //rfg_effect_material* effect_material;
    unsigned int flags;
};
struct vector
{
    f32 x;
    f32 y;
    f32 z;
};
struct matrix
{
    vector rvec;
    vector uvec;
    vector fvec;
};
struct destroyable_base
{
    u32 aabb_tree_offset; //rfg_rbb_node
    u32 base_objects_offset; //rfg_subpiece_base
    u32 base_extra_data_offset; //rfg_subpiece_base_extra_data
    int num_objects;
    u32 base_links_offset; //base_links
    int num_links;
    u32 base_dlods_offset; //base_dlods
    int num_dlods;
    u32 inst_data_offset; //rfg_destroyable_base_instance_data
    u32 transform_buffer_offset; //unsigned char
    float base_structural_mass;
};
struct subpiece_base
{
    vector prel_bmin;
    vector prel_bmax;
    vector pos;
    vector center_of_mass;
    f32 mass;
    u32 dlod_key;
    u32 links_offset; //ushort
    u8 physical_material_index;
    u8 shape_type;
    u8 num_links;
    u8 flags;
};
struct subpiece_base_data
{
    u32 shape_offset; //hkpShape
    u16 collision_model;
    u16 render_subpiece;
    u32 h;
};
struct link_base
{
    i32 yield_max;
    f32 area;
    i16 obj[2];
    u8 flags;
    u8 padding[3];
};
struct dlod_base
{
    u32 name_hash;
    vector pos;
    matrix orient;
    u16 render_subpiece;
    u16 first_piece;
    u8 max_pieces;
    u8 padding[3];
};
struct fp_aabb
{
    __int16 min_x;
    __int16 min_y;
    __int16 min_z;
    __int16 max_x;
    __int16 max_y;
    __int16 max_z;
};
struct rfg_rbb_node
{
    void Read(BinaryReader& reader)
    {
        reader.ReadToMemory(this, sizeof(rfg_rbb_node));
    }
    int num_objects;
    fp_aabb aabb;
    u32 node_data_offset; //et_ptr_offset<unsigned char, 0> node_data;
};
struct destroyable_instance_data
{
    unsigned int objects_offset;
    unsigned int links_offset;
    unsigned int dlods_offset;
    unsigned int data_size;
    u32 buffer_offset; //et_ptr_offset<unsigned char, 0> buffer;
};
struct destroyable
{
    destroyable_base base;
    std::vector<subpiece_base> subpieces;
    std::vector<subpiece_base_data> subpieces_data;
    std::vector<link_base> links;
    std::vector<dlod_base> dlods;
    std::vector<rfg_rbb_node> rbb_nodes;
    destroyable_instance_data instance_data;
};

struct MeshInstanceData
{
    std::span<u8> VertexBuffer;
    std::span<u8> IndexBuffer;
};

class StaticMesh
{
public:
    //Read header data from cpu file
    void Read(BinaryReader& reader, const string& name, u32 signature, u32 version, bool isChunkFile = false);
    //Read raw data of a submesh. Must call Read to get data from cpu file first
    std::optional<MeshInstanceData> ReadSubmeshData(BinaryReader& gpuFile, u32 index, bool isChunkFile = false);
    void Write(BinaryWriter& out);
    void Write(const string& path);
    void WriteToObj(const string& gpuFilePath, const string& outputFolderPath, const string& diffuseTexturePath = "", const string& specularTexturePath = "", const string& normalTexturePath = "");

    string Name;
    //Shared mesh header used by all mesh types
    MeshHeaderShared Header;
    //Static mesh specific header data (may be similar to other mesh types data)
    u32 NumLods;
    u32 LodSubmeshIdOffset;
    u32 MeshTagsOffset;
    u32 MeshTagsNumTags;
    u32 MeshTagsInternalOffset;
    u32 CmIndex;

    //Todo: Use MeshDataBlock here. It is exactly this layout so no point in making two separate definitions
    //Mesh data
    u32 MeshVersion;
    u32 MeshSimpleCrc; //Hash used several times in cpu and gpu file to validate mesh
    u32 CpuDataSize; //Size of data section from MeshOffset to material map data (minus alignment padding at end) in bytes
    u32 GpuDataSize; //Size of gpu file in bytes
    u32 NumSubmeshes;
    u32 SubmeshesOffset; //Seems to be a pointer set at runtime
    VertexBufferData VertexBufferConfig;
    IndexBufferData IndexBufferConfig;

    //Submeshes
    std::vector<SubmeshData> SubMeshes;

    //Render blocks
    std::vector<RenderBlock> RenderBlocks;

    //Material data block
    MaterialBlock MaterialBlock;

    //Texture names
    std::vector<string> TextureNames;

    //Todo: Move into specific class. Only added to this class for easy testing of chunk reading attempt. Should really be in it's own class
    //Chunk specific data (cchk_pc)
    std::vector<u32> MaterialIds;
    std::vector<RfgMaterial> ChunkMaterials;
    std::vector<destroyable> Destroyables;

private:
    bool readHeader_ = false;
};