#pragma once

#include "entity_base_vtable.h"

#include "entity_base_vhandle.h"
#include "string_hash.h"
#include "vector3d.h"

#include "config.h"

struct po;
struct event;
struct conglomerate;
struct generic_mash_header;
struct generic_mash_data_ptrs;
struct mString;
struct time_interface;
struct sound_and_pfx_interface;
struct chunk_file;
struct damage_interface;
struct facial_expression_interface;
struct physical_interface;
struct skeleton_interface;
struct animation_interface;
struct script_data_interface;
struct decal_data_interface;
struct resource_key;
struct motion_effect_struct;

namespace ai {
struct ai_core;
}

inline constexpr auto EFLAG_EXT_SIGNALLER_ONLY = 0x20000u;

inline constexpr auto EFLAG_EXT_DOES_NOT_MANAGE_ABS_PO = 0x40000000u;

struct entity_flag_t {
    uint32_t field_0;

    operator int() {
        return field_0;
    }
};

struct entity_ext_flag_t {
    uint32_t field_0;

    operator int() {
        return field_0;
    }
};

struct entity_base : entity_base_vtable {
    uint32_t field_4;
    uint32_t field_8;
    po *my_rel_po;
    string_hash field_10;
    po *my_abs_po;
    motion_effect_struct *field_18;
    entity_base_vhandle my_handle;
    entity_base *m_parent;
    entity_base *m_child;
    entity_base *field_28;
    int16_t proximity_map_cell_reference_count;
    uint8_t m_timer;
    _std::vector<entity_base *> *adopted_children;
    conglomerate *my_conglom_root;
    sound_and_pfx_interface *my_sound_and_pfx_interface;
    int16_t field_3C;
    int16_t field_3E;
    int8_t field_40;
    int8_t field_41;
    int8_t rel_po_idx;
    int8_t proximity_map_reference_count;

    entity_base() = default;

    //0x004F32C0
    entity_base(bool a2);

    //0x004F3400
    entity_base(const string_hash &a2, uint32_t a3, bool a4);

    uint16_t get_bone_idx() const;

    float sub_57CB80();

    ////// VIRTUAL FUNCTIONS: START //////

    //0x004F8FA0
    /* virtual */ ~entity_base();

    /* virtual */ int get_entity_size();

    bool has_sound_and_pfx_ifc();

    sound_and_pfx_interface *sound_and_pfx_ifc();

    //0x004F9020
    void release_mem();

    void load(chunk_file &a2);

    bool handle_load_chunk(chunk_file &, mString &);

    void read_enx(chunk_file &a1);

    bool handle_enx_chunk(chunk_file &, mString &);

    /* virtual */ vector3d get_last_position();

    float get_visual_radius();

    /* virtual */ vector3d get_visual_center();

    vector3d get_velocity();

    //0x004C0000
    void po_changed();

    void set_ext_flag_recursive_internal(entity_ext_flag_t f, bool a3);

    void set_flag_recursive(entity_flag_t a2, bool a3);

    void set_ext_flag_recursive(entity_ext_flag_t a2, bool a3);

    void set_active(bool a2);

    void set_visible(bool a2, bool a3);

    ai::ai_core *get_ai_core();

    bool is_hero();

    bool is_alive();

    /* virtual */ int get_flavor();

    /* virtual */ bool is_an_entity_base();

    bool is_a_signaller();

    bool is_an_entity();

    bool is_an_actor() const;

    bool is_a_conglomerate_clone();

    bool is_a_camera();

    bool is_a_station_camera();

    bool is_a_game_camera();

    bool is_a_glam_camera();

    bool is_a_marky_camera();

    bool is_a_mouselook_camera();

    bool is_a_sniper_camera();

    bool is_an_ai_camera();

    bool is_a_spiderman_camera();

    bool is_a_light_source();

    bool is_a_neolight();

    bool is_a_marker();

    bool is_a_parking_marker();

    bool is_a_water_exit_marker();

    bool is_a_rectangle_marker();

    bool is_a_cube_marker();

    bool is_a_crawl_marker();

    bool is_an_anchor_marker();

    bool is_a_line_marker_base();

    bool is_a_line_anchor();

    bool is_a_pole();

    bool is_an_ai_cover_marker();

    bool is_a_lensflare();

    bool is_a_manip_obj();

    bool is_a_switch_obj();

    bool is_a_mic();

    bool is_a_pfx_entity() const;

    bool is_an_item();

    bool is_a_handheld_item();

    bool is_a_gun();

    bool is_a_thrown_item();

    bool is_a_melee_item();

    bool is_a_visual_item();

    bool is_a_grenade();

    bool is_a_rocket();

    bool is_a_sky();

    bool is_a_beam();

    bool is_a_polytube();

    bool is_a_trigger();

    bool is_material_switching();

    bool has_time_ifc();

    time_interface *time_ifc();

    bool has_damage_ifc();

    damage_interface *damage_ifc();

    bool has_facial_expression_ifc();

    facial_expression_interface *facial_expression_ifc();

    bool has_physical_ifc();

    physical_interface *physical_ifc();

    bool has_skeleton_ifc();

    skeleton_interface *skeleton_ifc();

    bool has_animation_ifc();

    animation_interface *animation_ifc();

    bool has_script_data_ifc();

    script_data_interface *script_data_ifc();

    bool has_decal_data_ifc();

    decal_data_interface *decal_data_ifc();

    bool get_ifc_num(const resource_key &, float &, bool);

    bool set_ifc_num(const resource_key &, float, bool);

    bool get_ifc_vec(const resource_key &, vector3d &, bool);

    bool set_ifc_vec(const resource_key &, const vector3d &, bool);

    bool get_ifc_str(const resource_key &, mString &, bool);

    bool set_ifc_str(const resource_key &, const mString &, bool);

    void _un_mash(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3);

    //0x004CB2F0
    /* virtual */ void un_mash(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3);

    ////// VIRTUAL FUNCTIONS: END //////

    //0x004F36B0
    void raise_event(string_hash a2);

    //0x004E0E50
    void set_parent(entity_base *parent);

    //0x004BFF60
    bool is_a_parent(entity_base *a2);

    _std::vector<entity_base *> *get_adopted_children();

    bool empty_adopted_children() const;

    po &get_rel_po();

    auto *get_parent() {
        return this->m_parent;
    }

    auto get_my_handle() {
        return this->my_handle;
    }

    bool is_flagged(uint32_t a2) const;

    bool is_ext_flagged(uint32_t a2) const;

    bool is_flagged_in_the_moved_list() const;

    bool is_dynamic() const;

    bool is_walkable() const;

    bool manage_abs_po();

    bool is_mashed_member();

    bool are_collisions_active() const;

    uint32_t is_conglom_member();

    uint32_t is_a_conglomerate();

    //0x0048AC20
    po &get_abs_po();

    bool is_rel_po_dirty() const;

    void un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4, void *a5);

    //0x004F3700
    int add_callback(string_hash a2,
                     void (*callback)(event *, entity_base_vhandle, void *),
                     void *a4,
                     bool a5);

    //0x004DB740
    void add_adopted_child(entity_base *child_arg);

    //0x004F3550
    void common_destruct();

    bool is_visible() const;

    //0x0040FCA0
    void set_fade_distance(Float a1);

    //0x0040FC80
    void on_fade_distance_changed(int value);

    //0x004CB450
    void on_fade_distance_changed_internal(int a2);

    //0x004DB590
    void update_abs_po(bool a2);

    void sub_4D3F60(entity_base *a2);

    void sub_4E0DD0();

    //0x004D3FB0
    void clear_parent(bool a1);

    //0x004D6050
    unsigned int compute_rel_po_from_model();

    entity_base_vhandle get_my_vhandle();

    //0x004F3800
    bool event_raised_last_frame(string_hash a2);

    //0x0048C5C0
    void set_abs_po(const po &the_po);

    const vector3d &get_rel_position();

    //0x0048AC00
    const vector3d &get_abs_position();

    entity_base *get_first_child();

    //0x004F3B80
    void exit_limbo();

    //0x004BFED0
    void dirty_family(bool a2);

    //0x004BFF20
    void dirty_model_po_family();

    inline auto &get_id() const {
        return this->field_10;
    }

    //0x004E09F0
    void look_at(const vector3d &a1);

    //0x00502B50
    entity_base *get_conglom_owner();

    void set_abs_position(const vector3d &pos);

    void set_timer(int new_timer);

    inline auto get_fade_group() {
        return this->field_3E;
    }
};

extern void check_po(entity_base *);

extern int DEBUG_foster_conglom_warning;

//0x004E1290
extern void entity_set_abs_parent(entity_base *me, entity_base *parent);

//0x004E1490
extern void entity_report(entity_base *a1, const mString &a2, bool a3);

//0x004E1230
extern void entity_set_abs_position(entity_base *ent, const vector3d &pos);

//0x004E1100
extern void entity_set_abs_po(entity_base *ent, const po &the_po);

extern void entity_base_patch();
