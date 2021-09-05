#include "HashGuesser.h"
#include "Hash.h"

std::unordered_map<u32, string> HashGuesser::hashDictionary_ = {};
std::unordered_map<u32, string> HashGuesser::hashDictionaryAlt_ = {};
bool HashGuesser::initialized_ = false;
std::mutex HashGuesser::mutex_;

string stringList[] =
{
    //District names
    "Tutorial", "Parker", "Dust", "Oasis", "Badlands", "Manufacturing", "End Game", "Eos",

    //Class names
    "rfg_mover", "shape_cutter", "object_effect", "district", "multi_object_backpack",
    "multi_object_flag", "multi_object_marker", "object_action_node", "player", "object_patrol",
    "navpoint", "cover_node", "general_mover", "constraint", "item", "npc", "object_squad",
    "ladder", "obj_zone", "object_restricted_area", "object_safehouse", "trigger_region",
    "projectile", "turret", "object_convoy", "object_vehicle_spawn_node", "object_npc_spawn_node",
    "object_squad_spawn_node", "object_turret_spawn_node", "object_spawn_region",
    "object_demolitions_master_node", "object_activity_spawn", "object_area_defense_node",
    "object_bftp_node", "object_house_arrest_node", "object_mission_start_node",
    "player_start", "object_riding_shotgun_node", "object_upgrade_node", "object_debris",
    "object_raid_node", "object_air_strike_defense_node", "object_bounding_box",
    "object_dummy", "object_convoy_end_point", "object_path_road", "object_courier_end_point",
    "object_delivery_node", "object_guard_node", "obj_light", "marauder_ambush_region",
    "object_roadblock_node", "object_ambient_behavior_region", "automobile", "flyer",
    "walker", "weapon",

    //Additional class names
    "CLOE_ONLY", "edit_obj", "obj_chunk_ref", "stitch_piece", "decal", "terrain_decal",
    "layer", "navpoint", "constraint_point", "constraint_hinge", "constraint_hindge",
    "constraint_prism", "constraint_motor", "navmesh_cutout", "invisible_wall",
    "collision_box", "note", "object_foliage", "force_field_door", "effect_streaming_node",

    //Prop names (note: props = properties)
    //Object props
    "pos", "just_pos", "orient", "display_name", "explosion_event", "op",

    //edit_obj
    "name", "game_parent", "game_parent_obj_num", "pos", "orient", "persistent", "auto_restore",
    "mp_modes", "stick_to_terrain", "terrain_morphing", "terrain_falloff", "terrain_radius",

    //Zone props
    "name", "terrain", "district", "zone_hash", "spawn_resource", "spawn_resource_data", "terrain_file_name",
    "ambient_spawn", "invisible_walls", "wind_min_speed", "wind_max_speed", "zone_flags",

    //object_foliage
    "scale", "color_lerp",

    //force_field_door
    "SCALE_Y_BOTTOM",

    //effect_streaming_node
    "no_stub",

    //Invisible wall
    "extrude_dir", "extend_downward", "play_effect", "marker_distance", "marker_density", "marker_model",

    //note
    "note_text",

    //Subzone props
    "zone", "srid", "bmin", "bmax",

    //Cref props (What's a cref? Chunk ref?)
    "display_name", "chunk", "properties", "gameplay_properties", "importance",
    "team", "squads", "squad", "display_name", "building_type", "game_destroyed_pct",
    "display_radius", "min_pa_radius", "revision", "touch_terrain",

    //Stitch piece
    "chunk", "piece_flags", "unwalkable",

    //Navmesh cutout
    "light", "remove_cells", "pf_type",

    //decal
    "bb", "decal_entry",

    //terrain_decal
    "bb", "use_diffuse", "diffuse", "normal", "specular", "spec_power",
    "spec_alpha", "rotation_restriction", "scale_restriction",

    //Object mover props
    "chunk_name", "uid", "chunk_uid", "shape_uid", "props", "gameplay_props",
    "civilian_count", "flags", "building_type_internal", "building_type",
    "force_dynamic", "casts_shadows", "chunk_flags", "dynamic_object",
    "decal_list", "casts_drop_shadows", "importance_internal", "importance",
    "team_internal", "team", "control", "lod_off_distance", "dest_checksum",

    //Rfg mover props
    "rfg_flags", "mtype", "shape", "donor_shape", "shape_state", "shape_state_size",
    "squads", "squad", "show_on_map", "destruct_uid", "damage_percent",
    "game_destroyed_pct", "fully_destroyed_percent", "regenerate", "salvage_material",
    "salvage_num_pieces", "invulnerable", "plume_on_death", "one_of_many", "mining",
    "supply_crate", "stream_out_play_time", "world_anchors", "dynamic_links", "building",

    //General mover props
    "c_obj_id", "mtype", "ctype", "gm_flags", "mass", "com", "tensor", "idx", "sh_ptr",
    "destruct_uid", "original_object", "hitpoints", "dislodge_hitpoints",

    //Constraint props
    "template", "chunk_name", "idx", "host_handle", "child_handle", "host_index",
    "child_index", "host_hk_alt_index", "child_hk_alt_index", "outer_radius",
    "point_constraint_type", "spring_length", "min_x", "max_x", "min_y", "max_y",
    "min_z", "max_z", "threshold", "limited", "max_ang", "min_ang", "friction",
    "max", "min", "angular_speed", "gain",

    //Ladder props
    "ladder_rungs", "ladder_enabled",

    //Object path road props
    "path_road_struct", "path_road_data", "path_road_flags", "next", "prev",
    "ambient", "speed_limit", "persistent", "edf_only", "marauder", "guerrilla",
    "one_way", "other_way", "special", "reinforcements", "GPS",

    //Vehicle props
    "vehicle_type", "type", "veh_flags", "pos_at_ground", "initial_speed",
    "move_at_stream_flags", "immediate_spawn",

    //Cover node props
    "dist", "angle_left", "angle_right", "def_angle_left", "def_angle_right",
    "off_angle_left", "off_angle_right", "stance", "firing_flags", "cover_type",
    "binary_flags", "covernode_data", "cnp",

    //Guard node props
    "guarded_object", "angle_left", "angle_right", "no_defensive_combat", "group_id",
    "rendermesh",

    //Squad spawn node props
    "spawn_set", "spawn_prob", "night_spawn_prob", "default_orders", "default_orders_id",
    "respawn_speed", "raid_spawn", "hands_off_raid_squad", "miner_persona", "safehouse_vip",
    "squad_def", "no_reassignment", "dead_body", "special_vehicle", "special_npc",
    "disable_ambient_parking", "player_vehicle_respawn", "radio_operator",

    //Activity spawn node props
    "activity_type",

    //House arrest node props
    "disabled", "house_arrest_type",

    //Ambient behavior region props
    "behavior", "outer_radius",

    //Dummy object props
    "dummy_type", "dummy_type_internal", "rendermesh",

    //Obj_light props
    "type_enum", //Todo: Use type enum and flags enum from rfg_zonex_format.txt
    "light_type", "clr_orig", "atten_start", "atten_end", "atten_range", "min_clip",
    "max_clip", "clip_mesh", "aspect", "hotspot_size", "hotspot_falloff_size",
    "fvec", "light_flags", "parent_name", "show_attenuation_spheres", "show_clip_box",
    "bb",

    //Navpoint props
    "navpoint_type", "nav_type", "navlinks", "ref", "link_status", "cross_zone_link",
    "cross_zone_num", "dont_follow_road", "ignore_lanes", "outer_radius", "speed_limit",
    "navpoint_data", "npp", "nav_orient", "branch_type", "links",

    //Action node props
    "links", "obj_links", "animation_type", "high_priority", "infinite_duration",
    "disabled", "run_to", "rendermesh", "tint", "outer_radius", "rotation_restriction",

    //Patrol node props
    "patroled_object", //Game spelled it that way. I know how to spell, I swear!
    "patrol_start", "looping_patrol", "convoy_patrol", "courier_patrol",
    "ASD_truck_partol", //What the fuck is this spelling rfg?
    "override_patrol", "combat_ready", "marauder_raid", "no_check_in", "orders",
    "squad_def_index", "spawn_status", "original_pos", "respawn",

    //Roadblock node props
    "squad_vehicle", "vehicle_type", "roadblock_type", "use_object_orient", "bb",

    //Squad object props
    "vehicle_ref", "squad_flags", "npc_flags", "human_flags", "vehicle_flags",
    "vehicle_hp_mult", "node_handle", "map_icon_type",

    //Convoy end node props
    "convoy_type", "convoy_disabled", "convoy_suspended", "home_district",
    "activity_completed",

    //Convoy object props
    "convoy_def", "convoy_patrol",

    //Courier end point object props
    "courier_type", "end_game_only", "courier_completed",

    //Courier object props
    "courier_def", "courier_node", "courier_patrol", "courier_start",

    //Restricted area props
    "disabled", "allow_ambient_peds", "yellow_num_points", "yellow_polygon",
    "yellow_num_triangles", "yellow_triangles", "warning_num_points", "warning_polygon",
    "warning_num_triangles", "warning_triangles", "warning_radius",
    "bb", "area_type", //Maybe they had a restricted area drawing tool?

    //Raid node props
    "raid_type", "persistent", "rendermesh", "tint",

    //Demolitions master node props
    "demolitions_master_type", "demolitions_master_best_time",
    "demolitions_master_complete", "demolitions_master_discovered",
    "demolitions_master_par_beaten", "demolitions_master_times_completed",

    //Air strike defense node props
    "air_strike_defense_type",

    //Riding shotgun node props
    "riding_shotgun_type", "riding_shotgun_completed",

    //Area defense node props
    "area_defense_type", "area_defense_completed",

    //Delivery node props
    "delivery_type", "pair_number", "start_node", "delivery_flags",
    "best_time",

    //Bftp node props
    "tag_node", "pair_number", "bftp_flags", "collected", "enabled_bomb",

    //Marauder ambush region props
    "bb", "enabled", "day_trigger_prob", "night_trigger_prob",
    "min_ambush_squads", "max_ambush_squads",

    //Trigger region props
    "outer_radius", "trigger_shape", "region_type", "region_kill_type",
    "trigger_flags",

    //Projectile object props
    "prj_owner_h", "prj_w_info", "prj_pflags", "prj_target", "prj_vel",
    "prj_cur_vel", "prj_end_pos", "prj_duration", "prj_age_timer",
    "prj_start_pos", "prj_max_vel", "prj_detonate_ts", "prj_client_nonce",
    "prj_fired_in_slew",

    //Turret props
    "tinfo", "turret_type", "attach_tag", "rotation_limit", "weapon_owner",
    "rendermesh_weapon", "gun_pos", "gun_orient",

    //Npc props
    "npc_type", "npc_flags",

    //Human props
    "persona_name", "npc_info", "importance", "priority", "manager",
    "creator", "in_vehicle", "load_out", "human_flags", "ambient_behavior",
    "h_int_exp_ts", "h_ack_ts", "h_health_ts", "h_look_at_fin_ts",
    "h_reported_ts", "h_restore_inv", "nav_cell",

    //Multi-weapon props
    "respawns",

    //Multi-team/player props
    "game_data_handle", "multi-team",

    //Item props
    "item_type", "item_info", "flags", "ctype", "mass_override",
    "deletion_timer", "item_prop_type", "respawn", "respawns",
    "preplaced",

    //Obj item props (unsure of difference between this and normal objects)
    "item_srid", "salvage_material",

    //Collision box
    "material",

    //Effect object props
    "effect_type", "looping", "host_handle", "host_tag", "position",
    "orientation", "volume", "attached_to_camera",
    "attached_to_camera_orient", "visual", "sound", "sound_alr",
    "visual_handle", "sound_handle", "sound_alr_handle", "source_handle",
    "inner_radius", "outer_radius",

    //Streamed effect props
    "streamed_effect",

    //Weapon props
    "weapon_type", "wep_info", "wep_info_name", "wep_mag_rnds",
    "wep_res_rnds", "wep_flags", "wep_refire_delay",
    "wep_reload_delay", "wep_last_fired_timer",

    //Debris object props
    "c_info",

    //Character instance props
    "render_scale", "char_scale",

    //Skybox props
    "c_info",

    //Player start props
    "indoors", "indoor", "mp_team", "initial_spawn", "respawn",
    "checkpoint_respawn", "activity_respawn", "mission_info",

    //Object spawn region props
    "bb", "team", "npc_only", "vehicle_only",

    //Bounding box object props
    "bounding_box_type",

    //OCC props (obj_occluder)
    "oc_shape", "bb",

    //Shape cutter props
    "exp_info", "source", "owner", "flags",
    "shape_cutter_type", "outer_radius", "w_info",

    //Mission start node props
    "mission_info", "enabled", "autostart",

    //Safehouse props
    "zone_serialized", "player_start", "vehicle_spawns",
    "buildings", "trigger_regions", "enabled",
    "disabled_flags", "visible", "display_name_tag",
    "display_name_hash",

    //District props
    "control", "morale", "control_max", "morale_max",
    "grid_index", "index", "grid_color", "VFX_tint",
    "liberated", "liberated_play_line", "district_flags",

    //Multi-marker props
    "bb", "marker_type", "mp_team", "priority", "backpack_type",
    "num_backpacks", "random_backpacks", "group",

    //Multi flag capture zone props
    "mp_team",

    //Ambient spawn node props
    "ped_spawn_density", "ped_spawn_group", "veh_spawn_density",
    "veh_spawn_group", "driver_spawn_group",

    //Object upgrade node props
    "upgrade_type",

    //Mp flag props
    "mp_team",

    //Backpack object props
    "backpack_type"
};

std::optional<string> HashGuesser::GuessHashOriginString(u32 hash)
{
    mutex_.lock();
    if (!initialized_)
        FillHashDictionary();

    mutex_.unlock();

    auto result = hashDictionary_.find(hash);
    if (result != hashDictionary_.end())
        return result->second;

    auto resultAlt = hashDictionaryAlt_.find(hash);
    if (resultAlt != hashDictionaryAlt_.end())
        return resultAlt->second;

    return {};
}

void HashGuesser::FillHashDictionary()
{
    for (auto& name : stringList)
    {
        hashDictionary_[Hash::HashVolitionCRC(name, 0)] = name;
        hashDictionaryAlt_[Hash::HashVolition(name)] = name;
    }
    initialized_ = true;
}
