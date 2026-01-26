#pragma once

#include "entity.h"

#include "animation_controller.h"

#include "float.hpp"
#include "spline.h"
#include "variable.h"

#include <list>

struct collision_free_state;
struct nalAnyPose;

template<typename T>
struct nalAnimClass;

namespace ai {
struct ai_core;
}

struct traffic_light_interface;
struct string_hash;
struct physical_interface;
struct ai_player_controller;
struct generic_mash_header;
struct generic_mash_data_ptrs;
struct nglMesh;
struct resource_pack_slot;
struct item;
struct signaller;
struct advanced_entity_ptrs;
struct damage_interface;
struct interactable_interface;
struct facial_expression_interface;
struct generic_anim_controller;
struct base_ai_data;
struct nalBaseSkeleton;
struct lego_map_root_node;
struct movement_info;
struct nal_anim_controller;
struct nalBaseSkeleton;
struct web_interface;

struct actor : entity {
    using base_type = vhandle_type<entity>;

    struct mesh_buffers {
        nglMesh **field_0;
        uint8_t field_4;
        uint8_t field_5;
        uint16_t field_6;
        int active_client_count;
        uint8_t field_C[4];

        void set_mesh(nglMesh *);
    };

    damage_interface *m_damage_interface;
    physical_interface *m_physical_interface;
    nalBaseSkeleton *m_skeleton;
    nal_anim_controller *anim_ctrl;
    advanced_entity_ptrs *adv_ptrs;
    base_ai_data *field_7C;
    interactable_interface *m_interactable_ifc;
    facial_expression_interface *m_facial_expression_interface;
    web_interface *field_88;
    ai_player_controller *m_player_controller;
    mesh_buffers field_90;
    traffic_light_interface *m_traffic_light_interface;
    int16_t field_A4;
    int16_t field_A6;
    int16_t field_A8[1];
    int16_t field_AA;
    vector3d field_AC;

    int field_B8;
    resource_pack_slot *m_resource_context;

    //0x004F92D0
    actor(const string_hash &a2, uint32_t a3);

    //0x0050A3F0
    actor(int);

    ~actor();

    void common_construct();

    resource_pack_slot * get_resource_context() {
        return this->m_resource_context;
    }

    collision_free_state *get_last_collision_free_state() const;

    void set_colgeom(collision_geometry *a2);

    int get_entity_size();

    void release_mem();

    vector3d get_velocity();

    bool has_traffic_light_ifc();

    void init_traffic_light_interface();

    traffic_light_interface *traffic_light_ifc();

    bool has_skeleton_ifc() const;

    //virtual
    color32 _get_render_color() const;

    //0x004B8D30
    //virtual
    void _set_render_alpha_mod(Float a2);

    //virtual
    float _get_render_alpha_mod() const;

	//virtual
	void set_render_scale(const vector3d &s);

	//virtual
	vector3d get_render_scale() const;

    //virtual
    void ifl_play();

    //virtual
    void ifl_lock(int a2);

    //0x004E2C10
    void add_collision_ignorance(entity_base_vhandle a2);

    void allocate_anim_controller(unsigned int a2, nalBaseSkeleton *a3);

    //0x004CC470
    nal_anim_controller *select_and_new_anim_controller(
        nalBaseSkeleton *a2,
        unsigned int a3);

    animation_controller::anim_ctrl_handle play_anim(const string_hash &a3);

    void bind_to_scene_anim();

    void unbind_from_scene_anim(string_hash a3, string_hash a4);

    float get_floor_offset();

    bool anim_finished(int);

    //virtual
    void invalidate_frame_delta();

    inline ai_player_controller *get_player_controller() {
        return this->m_player_controller;
    }

    //0x004D6B60
    void set_frame_delta_no_update(const po &a2, Float a3);

    void set_allow_tunnelling_into_next_frame(bool a2);

    //0x004CC940
    bool get_allow_tunnelling_into_next_frame();

    //0x004D2100
    void *find_like_item(
        vhandle_type<
            item,
            vhandle_type<actor, vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>>>>
            a2);

    //0x004E2670
    void create_damage_ifc();

    void destroy_damage_ifc();

    //0x004D6AA0
    void create_web_ifc();

    //0x004F5720
    void common_destruct();

    //0x004E3970
    void cancel_animated_movement(const vector3d &a2, Float a3);

    //0x004E2EE0
    void get_velocity(vector3d *a2);

    //0x004FB960
    void process_extra_scene_flags(unsigned int a2);

    //0x004B8B40
    physical_interface *physical_ifc();

	void create_adv_ptrs();

    //0x004C0E60
    bool has_camera_collision() const;

    //0x004C0E30
    bool has_entity_collision();

    //0x004CC740
    void kill_interact_anim();

    //0x004E26E0
    void create_physical_ifc();

    void destroy_physical_ifc();

    //0x004FBD40
    //virtual
    void _un_mash(generic_mash_header *a3, void *a4, generic_mash_data_ptrs *a5);

    //0x004C0CD0
    void create_player_controller(int a2);

    //0x004C0D40
    void destroy_player_controller();

    bool has_vertical_obb();

    vector3d *get_cached_visual_bounding_sphere_center();

    nglMesh **sub_4B8BCA();

    damage_interface *damage_ifc();

    lego_map_root_node *get_lego_map_root();

    movement_info *get_movement_info();

    //virtual
    float get_colgeom_radius() const;

    //0x004B8FC0
    //virtual
    bool is_frame_delta_valid() const;

    //0x004B9000
    //virtual
    po *get_frame_delta() const;

    //virtual
    void set_frame_delta(const po &a2, Float a3);

    //virtual
    void set_frame_delta_trans(const vector3d &a2, Float a3);

    //0x004E31F0
    vector3d _get_visual_center();

    //0x004B8BB0
    nglMesh *_get_mesh();

    //0x004C0F80
    ai::ai_core *_get_ai_core();

    //virtual
    vector3d get_colgeom_center() const;

    //virtual
    void radius_changed(bool );

    //0x004E33B0
    void _render(Float a2);

    //virtual
    bool add_item(int a4, bool a6);

    //0x004D2060
    static void swap_all_mesh_buffers();

    static void get_animations(actor *a1, std::list<nalAnimClass<nalAnyPose> *> &a2);
};

inline actor *& global_transfer_variable_the_actor = var<actor *>(0x0096C978);

namespace ai {

//0x0068A440
extern void setup_hero_capsule(actor *act);
} // namespace ai

extern void actor_patch();
