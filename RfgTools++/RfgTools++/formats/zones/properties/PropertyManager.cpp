#include "PropertyManager.h"
#include "common/Typedefs.h"
#include "types/Vec2.h"
#include "RfgTools++/formats/zones/properties/IZoneProperty.h"
#include "RfgTools++/formats/zones/properties/primitive/StringProperty.h"
#include "RfgTools++/formats/zones/properties/primitive/BoolProperty.h"
#include "RfgTools++/formats/zones/properties/primitive/FloatProperty.h"
#include "RfgTools++/formats/zones/properties/primitive/UintProperty.h"
#include "RfgTools++/formats/zones/properties/compound/Vec3Property.h"
#include "RfgTools++/formats/zones/properties/compound/Matrix33Property.h"
#include "RfgTools++/formats/zones/properties/compound/BoundingBoxProperty.h"
#include "RfgTools++/formats/zones/properties/compound/OpProperty.h"
#include "RfgTools++/formats/zones/properties/special/DistrictFlagsProperty.h"
#include "RfgTools++/formats/zones/properties/compound/ListProperty.h"
#include "RfgTools++/formats/zones/properties/special/NavpointDataProperty.h"

std::unordered_map <u32, std::function<IZoneProperty* (BinaryReader& reader, u16 type, u16 size, u32 nameHash)>> PropertyManager::propertyTypes4_ = {};
std::unordered_map <u32, std::function<IZoneProperty* (BinaryReader& reader, u16 type, u16 size, u32 nameHash)>> PropertyManager::propertyTypes5_ = {};
std::unordered_map <u32, std::function<IZoneProperty* (BinaryReader& reader, u16 type, u16 size, u32 nameHash)>> PropertyManager::propertyTypes6_ = {};
bool PropertyManager::initialized_ = false;
std::mutex PropertyManager::typesMutex_;

void PropertyManager::ReadObjectProperties(ZoneObject36& object, BinaryReader& reader)
{
    //Ensure the properties are initialized. Mutex used to avoid thread deadlock as multiple territories can be loaded at once.
    typesMutex_.lock();
    if (!initialized_)
        Init();

    typesMutex_.unlock();

    //Read properties
    for (u32 i = 0; i < object.NumProps; i++)
    {
        //Each property is prefixed by a type identifier and the size of the property data in bytes
        u16 type = reader.ReadUint16();
        u16 size = reader.ReadUint16();
        u32 propertyNameHash = reader.ReadUint32();
        u64 propertyStartPos = reader.Position();

        //Attempt to read property data
        IZoneProperty* prop = ReadProperty(reader, type, size, propertyNameHash);

        //Skip property if read failed
        if (!prop)
        {
            reader.SeekBeg(propertyStartPos);
            reader.Skip(size);
        }
        //Todo: Make UnknownDataProperty for failed reads / unknown property types to preserve data. Will be needed when saving is added

        //Else add it to the list & align to next property
        object.Properties.push_back(prop);
        reader.Align(4);
    }
}

IZoneProperty* PropertyManager::ReadProperty(BinaryReader& reader, u16 type, u16 size, u32 nameHash)
{
    //Attempt to find property definition based on type and nameHash
    switch (type)
    {
    case 4:
    {
        auto result = propertyTypes4_.find(nameHash);
        return result == propertyTypes4_.end() ? nullptr : result->second(reader, type, size, nameHash);
    }
    case 5:
    {
        auto result = propertyTypes5_.find(nameHash);
        return result == propertyTypes5_.end() ? nullptr : result->second(reader, type, size, nameHash);
    }
    case 6:
    {
        auto result = propertyTypes6_.find(nameHash);
        return result == propertyTypes6_.end() ? nullptr : result->second(reader, type, size, nameHash);
    }
    default:
        throw std::runtime_error("Error! Encountered unknown zone object property type value(" + std::to_string(type) + "). This probably means there's a read error happening elsewhere.");
        break;
    }
    return nullptr;
}

//Macro to simplify adding property definitions
#define PropertyDefinition(nameString, ConcreteType)                               \
{                                                                                  \
    (u32)Hash::HashVolitionCRC(#nameString, 0),                                    \
    [](BinaryReader& reader, u16 type, u16 size, u32 nameHash) -> IZoneProperty*   \
    {                                                                              \
        IZoneProperty* prop = new ConcreteType;                                    \
        if (prop->Read(reader, type, size, nameHash))                              \
        {                                                                          \
            prop->Name = #nameString;                                              \
            prop->Type = type;                                                     \
            prop->NameHash = Hash::HashVolitionCRC(#nameString, 0);                \
            return prop;                                                           \
        }                                                                          \
        else                                                                       \
        {                                                                          \
            delete prop;                                                           \
            return nullptr;                                                        \
        }                                                                          \
    }                                                                              \
}                                                                                  \



void PropertyManager::Init()
{
    //Properties with the type value of 4
    propertyTypes4_ =
    {
        //All type 4 properties are string properties
        PropertyDefinition(district, StringProperty),
        PropertyDefinition(terrain_file_name, StringProperty),
        PropertyDefinition(ambient_spawn, StringProperty),
        PropertyDefinition(mission_info, StringProperty),
        PropertyDefinition(mp_team, StringProperty),
        PropertyDefinition(item_type, StringProperty),
        PropertyDefinition(default_orders, StringProperty),
        PropertyDefinition(squad_def, StringProperty),
        PropertyDefinition(respawn_speed, StringProperty),
        PropertyDefinition(vehicle_type, StringProperty),
        PropertyDefinition(spawn_set, StringProperty),
        PropertyDefinition(chunk_name, StringProperty),
        PropertyDefinition(props, StringProperty),
        PropertyDefinition(building_type, StringProperty),
        PropertyDefinition(gameplay_props, StringProperty),
        PropertyDefinition(chunk_flags, StringProperty),
        PropertyDefinition(display_name, StringProperty),
        PropertyDefinition(turret_type, StringProperty),
        PropertyDefinition(animation_type, StringProperty),
        PropertyDefinition(weapon_type, StringProperty),
        PropertyDefinition(bounding_box_type, StringProperty),
        PropertyDefinition(trigger_shape, StringProperty),
        PropertyDefinition(trigger_flags, StringProperty),
        PropertyDefinition(region_kill_type, StringProperty),
        PropertyDefinition(region_type, StringProperty),
        PropertyDefinition(convoy_type, StringProperty),
        PropertyDefinition(home_district, StringProperty),
        PropertyDefinition(raid_type, StringProperty),
        PropertyDefinition(house_arrest_type, StringProperty),
        PropertyDefinition(activity_type, StringProperty),
        PropertyDefinition(delivery_type, StringProperty),
        PropertyDefinition(courier_type, StringProperty),
        PropertyDefinition(streamed_effect, StringProperty),
        PropertyDefinition(display_name_tag, StringProperty),
        PropertyDefinition(upgrade_type, StringProperty),
        PropertyDefinition(riding_shotgun_type, StringProperty),
        PropertyDefinition(area_defense_type, StringProperty),
        PropertyDefinition(dummy_type, StringProperty),
        PropertyDefinition(demolitions_master_type, StringProperty),
        PropertyDefinition(team, StringProperty),
        PropertyDefinition(sound_alr, StringProperty),
        PropertyDefinition(sound, StringProperty),
        PropertyDefinition(visual, StringProperty),
        PropertyDefinition(behavior, StringProperty),
        PropertyDefinition(roadblock_type, StringProperty),
        PropertyDefinition(type_enum, StringProperty),
        PropertyDefinition(clip_mesh, StringProperty),
        PropertyDefinition(light_flags, StringProperty),
        PropertyDefinition(backpack_type, StringProperty),
        PropertyDefinition(marker_type, StringProperty),
        PropertyDefinition(area_type, StringProperty),
        PropertyDefinition(spawn_resource_data, StringProperty),
        PropertyDefinition(parent_name, StringProperty),
    };

    //Properties with the type value of 5
    propertyTypes5_ =
    {
        //Bool properties
        PropertyDefinition(respawn, BoolProperty),
        PropertyDefinition(respawns, BoolProperty),
        PropertyDefinition(checkpoint_respawn, BoolProperty),
        PropertyDefinition(initial_spawn, BoolProperty),
        PropertyDefinition(activity_respawn, BoolProperty),
        PropertyDefinition(special_npc, BoolProperty),
        PropertyDefinition(safehouse_vip, BoolProperty),
        PropertyDefinition(special_vehicle, BoolProperty),
        PropertyDefinition(hands_off_raid_squad, BoolProperty),
        PropertyDefinition(radio_operator, BoolProperty),
        PropertyDefinition(squad_vehicle, BoolProperty),
        PropertyDefinition(miner_persona, BoolProperty),
        PropertyDefinition(raid_spawn, BoolProperty),
        PropertyDefinition(no_reassignment, BoolProperty),
        PropertyDefinition(disable_ambient_parking, BoolProperty),
        PropertyDefinition(player_vehicle_respawn, BoolProperty),
        PropertyDefinition(no_defensive_combat, BoolProperty),
        PropertyDefinition(preplaced, BoolProperty),
        PropertyDefinition(enabled, BoolProperty),
        PropertyDefinition(indoor, BoolProperty),
        PropertyDefinition(no_stub, BoolProperty),
        PropertyDefinition(autostart, BoolProperty),
        PropertyDefinition(high_priority, BoolProperty),
        PropertyDefinition(run_to, BoolProperty),
        PropertyDefinition(infinite_duration, BoolProperty),
        PropertyDefinition(no_check_in, BoolProperty),
        PropertyDefinition(combat_ready, BoolProperty),
        PropertyDefinition(looping_patrol, BoolProperty),
        PropertyDefinition(marauder_raid, BoolProperty),
        PropertyDefinition(ASD_truck_partol, BoolProperty), //The game spelled it this way, can't change it unless we want to break the hash generated from it.
        PropertyDefinition(courier_patrol, BoolProperty),
        PropertyDefinition(override_patrol, BoolProperty),
        PropertyDefinition(allow_ambient_peds, BoolProperty),
        PropertyDefinition(disabled, BoolProperty),
        PropertyDefinition(tag_node, BoolProperty),
        PropertyDefinition(start_node, BoolProperty),
        PropertyDefinition(end_game_only, BoolProperty),
        PropertyDefinition(visible, BoolProperty),
        PropertyDefinition(vehicle_only, BoolProperty),
        PropertyDefinition(npc_only, BoolProperty),
        PropertyDefinition(dead_body, BoolProperty),
        PropertyDefinition(looping, BoolProperty),
        PropertyDefinition(use_object_orient, BoolProperty),
        PropertyDefinition(random_backpacks, BoolProperty),
        PropertyDefinition(liberated, BoolProperty),
        PropertyDefinition(liberated_play_line, BoolProperty),

        //Float properties
        PropertyDefinition(wind_min_speed, FloatProperty),
        PropertyDefinition(wind_max_speed, FloatProperty),
        PropertyDefinition(spawn_prob, FloatProperty),
        PropertyDefinition(night_spawn_prob, FloatProperty),
        PropertyDefinition(angle_left, FloatProperty),
        PropertyDefinition(angle_right, FloatProperty),
        PropertyDefinition(rotation_limit, FloatProperty),
        PropertyDefinition(game_destroyed_pct, FloatProperty),
        PropertyDefinition(outer_radius, FloatProperty),
        PropertyDefinition(night_trigger_prob, FloatProperty),
        PropertyDefinition(day_trigger_prob, FloatProperty),
        PropertyDefinition(speed_limit, FloatProperty),
        PropertyDefinition(hotspot_falloff_size, FloatProperty),
        PropertyDefinition(atten_range, FloatProperty),
        PropertyDefinition(aspect, FloatProperty),
        PropertyDefinition(hotspot_size, FloatProperty),
        PropertyDefinition(atten_start, FloatProperty),
        PropertyDefinition(control, FloatProperty),
        PropertyDefinition(control_max, FloatProperty),
        PropertyDefinition(morale, FloatProperty),
        PropertyDefinition(morale_max, FloatProperty),

        //Unsigned int properties
        PropertyDefinition(gm_flags, UintProperty),
        PropertyDefinition(dest_checksum, UintProperty),
        PropertyDefinition(uid, UintProperty),
        PropertyDefinition(next, UintProperty),
        PropertyDefinition(prev, UintProperty),
        PropertyDefinition(mtype, UintProperty),
        PropertyDefinition(group_id, UintProperty),
        PropertyDefinition(ladder_rungs, UintProperty),
        PropertyDefinition(min_ambush_squads, UintProperty),
        PropertyDefinition(max_ambush_squads, UintProperty),
        PropertyDefinition(host_index, UintProperty),
        PropertyDefinition(child_index, UintProperty),
        PropertyDefinition(child_alt_hk_body_index, UintProperty),
        PropertyDefinition(host_alt_hk_body_index, UintProperty),
        PropertyDefinition(host_handle, UintProperty),
        PropertyDefinition(child_handle, UintProperty),
        PropertyDefinition(path_road_flags, UintProperty),
        PropertyDefinition(patrol_start, UintProperty),
        PropertyDefinition(yellow_num_points, UintProperty),
        PropertyDefinition(yellow_num_triangles, UintProperty),
        PropertyDefinition(warning_num_points, UintProperty),
        PropertyDefinition(warning_num_triangles, UintProperty),
        PropertyDefinition(pair_number, UintProperty),
        PropertyDefinition(group, UintProperty),
        PropertyDefinition(priority, UintProperty),
        PropertyDefinition(num_backpacks, UintProperty),

        //Vec3 property
        PropertyDefinition(just_pos, Vec3Property),
        PropertyDefinition(min_clip, Vec3Property),
        PropertyDefinition(max_clip, Vec3Property),
        PropertyDefinition(clr_orig, Vec3Property),

        //Matrix33 property
        PropertyDefinition(nav_orient, Matrix33Property),

        //Bounding box property
        PropertyDefinition(bb, BoundingBoxProperty),

        //Op property. op = orientation & position
        PropertyDefinition(op, OpProperty),

        //District flags property
        PropertyDefinition(district_flags, DistrictFlagsProperty),

        //Todo: Support this property type
        //Constraint template property
        //PropertyDefinition(template, ConstraintTemplateProperty),
        //{
        //    (u32)Hash::HashVolitionCRC("template", 0),
        //    [](BinaryReader& reader, u16 type, u16 size, u32 nameHash) -> IZoneProperty*
        //    {
        //        IZoneProperty* prop = new ConstraintTemplateProperty;
        //        if (prop->Read(reader, type, size, nameHash))
        //            return prop;
        //        else
        //        {
        //            delete prop;
        //            return nullptr;
        //        }
        //    }
        //}
    };

    //Properties with the type value of 6
    propertyTypes6_ =
    {
        //Various value list properties
        PropertyDefinition(obj_links, ListProperty<u32>),
        PropertyDefinition(world_anchors, ListProperty<u16>),
        PropertyDefinition(covernode_data, ListProperty<u32>),
        PropertyDefinition(dynamic_links, ListProperty<u32>),
        PropertyDefinition(path_road_struct, ListProperty<u32>),
        PropertyDefinition(path_road_data, ListProperty<u16>),
        PropertyDefinition(yellow_triangles, ListProperty<u16>),
        PropertyDefinition(warning_triangles, ListProperty<u16>),
        PropertyDefinition(yellow_polygon, ListProperty<Vec2>),
        PropertyDefinition(warning_polygon, ListProperty<Vec2>),

        //Navpoint data property
        PropertyDefinition(navpoint_data, NavpointDataProperty),
    };
}
