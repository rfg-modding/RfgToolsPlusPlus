#pragma once
#include "common/Typedefs.h"
#include "Common/timing/Timer.h"
#include "Common/filesystem/Path.h"
#include "Common/filesystem/File.h"
#include <RfgTools++\formats\meshes\MeshDataBlock.h>
#include "RfgTools++/types/Vec2.h"
#include "RfgTools++/formats/packfiles/Packfile3.h"
#include "RfgTools++/hashes/Hash.h"
#include <BinaryTools\BinaryWriter.h>

//Types used by experimental chunk file reader

struct ChunkHeader
{
    unsigned int signature = 0;
    unsigned int version = 0;
    unsigned int source_rfgchunkx_version = 0;
    unsigned int render_data_checksum = 0;
    unsigned int render_cpu_data_offset = 0;
    unsigned int render_cpu_data_size = 0;
    unsigned int collision_model_checksum = 0;
    unsigned int collision_model_data_offset = 0;
    unsigned int collision_model_data_size = 0;
    unsigned int destruction_checksum = 0;
    unsigned int destruction_offset = 0;
    unsigned int destruction_datasize = 0;
};

struct rl_material_data
{
    unsigned int shader_handle;
    unsigned int name_checksum;
    unsigned int mat_flags;
    unsigned __int16 num_textures;
    char num_constants;
    char max_constants;
    u32 textures_offset;
    u32 constant_name_checksum_offset;
    u32 constant_block_offset;
};
struct texture_desc
{
    u32 name_offset;
    u32 name_checksum;
    u32 texture_index;
};
struct material_constant
{
    float constants[4];
};
struct material_data_block
{
    rl_material_data material_data;
    std::vector<texture_desc> texture_descs;
    std::vector<u32> constant_name_checksums;
    std::vector<material_constant> constants;

    void Read(BinaryReader& reader)
    {
        u64 startPos = reader.Position();
        u32 materialDataSize = reader.ReadUint32();

        reader.ReadToMemory(&material_data, sizeof(rl_material_data));
        for (u32 i = 0; i < material_data.num_textures; i++)
        {
            auto& desc = texture_descs.emplace_back();
            reader.ReadToMemory(&desc, sizeof(texture_desc));
        }
        for (u32 i = 0; i < material_data.num_constants; i++)
        {
            constant_name_checksums.push_back(reader.ReadUint32());
        }
        reader.Align(16);
        for (u32 i = 0; i < material_data.max_constants; i++)
        {
            auto& constantData = constants.emplace_back();
            reader.ReadToMemory(&constantData, sizeof(material_constant));
        }

        if (reader.Position() - startPos != materialDataSize)
            throw std::exception("Error! Failed to read material data block. Length of read data doesn't equal expected data size.");
    }
};

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
struct matrix;
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

    Vec3 rotate_point(Vec3& pos)
    {
        Vec3 posFinal = pos;
        //posFinal.x = (rvec.x * pos.x) + (rvec.y * pos.y) + (rvec.z * pos.z);
        //posFinal.y = (uvec.x * pos.x) + (uvec.y * pos.y) + (uvec.z * pos.z);
        //posFinal.z = (fvec.x * pos.x) + (fvec.y * pos.y) + (fvec.z * pos.z);
        posFinal.x = (rvec.x * pos.x) + (uvec.x * pos.y) + (fvec.x * pos.z);
        posFinal.y = (rvec.y * pos.x) + (uvec.y * pos.y) + (fvec.y * pos.z);
        posFinal.z = (rvec.z * pos.x) + (uvec.z * pos.y) + (fvec.z * pos.z);
        return posFinal;
    }
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

struct ByteVec4
{
    uint8_t x, y, z, w;
};
static_assert(sizeof(ByteVec4) == 4);

struct ShortVec2
{
    int16_t x, y;
};
static_assert(sizeof(ShortVec2) == 4);

//Read vertex and index data
struct ChunkVertex
{
    Vec3 Position;
    ByteVec4 Normal;
    ByteVec4 Tangent;
    ShortVec2 Uv0;
    ShortVec2 Uv1;
    ShortVec2 Uv2;
};
static_assert(sizeof(ChunkVertex) == 32);