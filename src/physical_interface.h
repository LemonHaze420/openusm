#pragma once

#include "entity_base_vhandle.h"
#include "entity.h"
#include "float.hpp"
#include "mstring.h"
#include "string_hash.h"
#include "variable.h"
#include "vector3d.h"

#include <vector.hpp>

struct actor;
struct biped_system;
struct pendulum;
struct resource_key;
struct signaller;
struct generic_mash_header;
struct generic_mash_data_ptrs;
struct entity_base_vhandle;
struct rocket_guidance_sys;

extern inline constexpr auto PHYS_IFC_MAX_PENDULUM_CONSTRAINTS = 5;

struct physical_interface {
    enum biped_physics_body_types {
    };

    enum force_type {
    };

    std::intptr_t m_vtbl;
    actor *field_4;
    bool dynamic;
    char empty0[3];
    uint32_t field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    vector3d m_velocity;
    vector3d field_2C;
    vector3d field_38;
    vector3d field_44;
    vector3d field_50;
    vector3d field_5C;
    vector3d field_68;
    vector3d field_74;
    int field_80;
    vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>> field_84;
    string_hash field_88;
    float field_8C;
    float field_90;
    float field_94;
    int field_98;
    float field_9C;
    float field_A0;
    int field_A4;
    vector3d field_A8;
    vector3d field_B4;
    int field_C0;
    int field_C4;
    float field_C8;
    float field_CC;
    float m_gravity_multiplier;
    float field_D4;
    int field_D8;
    int field_DC;
    int field_E0;
    float field_E4;
    rocket_guidance_sys *field_E8;
    float field_EC;
    float field_F0;
    int field_F4;
    int field_F8;
    int field_FC;
    vector3d field_100;
    float field_10C;
    pendulum *field_110[PHYS_IFC_MAX_PENDULUM_CONSTRAINTS];
    int field_124;
    int field_128;
    int field_12C;
    int field_130;
    int field_134;
    int field_138;
    int field_13C;
    int field_140;
    int field_144;
    int field_148;
    int field_14C;
    int field_150;
    int field_154;
    bool field_158;
    float field_15C;
    float field_160;
    float field_164;
    float field_168;
    float field_16C;
    int field_170;
    int *field_174;
    biped_system *m_bp_sys;
    char field_17C;
    char field_17D;
    float field_180;
    bool field_184;
    char field_185;
    char field_186;
    mString field_188;
    mString field_198;
    int field_1A8;
    float field_1AC;

    //0x004DF020
    physical_interface(actor *a2);

    pendulum *get_pendulum(int num);

    bool is_flag(uint32_t a2) const {
        return (a2 & this->field_C) != 0;
    }

    actor * get_actor() {
        return this->field_4;
    }

    bool is_enabled() const;

    int get_num_active_pendulums() const;

    //0x004C93E0
    void get_parent_terrain_type(string_hash *a2);

    //0x004CEDA0
    float cancel_all_velocity();

    //0x004BDE00
    bool is_effectively_standing();

    void manage_standing(bool a2);

    //0x004C9500
    bool set_ifc_num(const resource_key &a2, Float a3, bool a4);

    //0x004BD160
    bool get_ifc_num(const resource_key &a2, float &a3, bool a4);

    bool is_biped_physics_running() const;

    bool is_prop_physics_running() const
    {
        return this->field_174 != nullptr;
    }

    biped_system *get_biped_system();

    //0x004DA210
    void add_to_phys_ifc_list();

    //0x004CA0D0
    vector3d get_velocity() const;
 
    //0x004D19E0
    void set_pendulum(int a2, pendulum *a3);

    //0x004BD060
    void set_gravity(bool a2);

    //0x004BDE90
    void set_current_gravity_vector(const vector3d &a2);

    //0x004BCC50
    float get_floor_offset();

    //0x004BDE70
    void suspend(bool a2);

    //0x004BDCB0
    static vector3d calculate_perfect_force_vector(const vector3d &start,
                                                   const vector3d &target,
                                                   Float max_y,
                                                   Float gravity_multiplier);

    //0x004BD9E0
    static void calculate_force_vector(Float a1, Float a2, float *a3, float *a4, Float a5);

    //0x004BDE50
    void enable(bool a2);

    //0x004CA1C0
    void set_velocity(const vector3d &new_velocity, bool a3);

    //0x004ECC10
    void set_control_parent(entity *a2);

    //0x004F2460
    void start_biped_physics(physical_interface::biped_physics_body_types a2);

    //0x004BCC20
    bool allow_manage_standing();

    //0x004BCBF0
    void set_allow_manage_standing(bool a2);

    //0x004F2700
    void stop_biped_physics(bool a2);

    //0x004DF4A0
    void un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5);

    vector3d apply_positional_constraints(
        Float a3,
        const vector3d &a4,
        bool a5);

    //0x004C9430
    void apply_force_increment_in_biped_physics_mode(const vector3d &a2,
                                                     force_type a3,
                                                     const vector3d &a4,
                                                     int a5);

    void stop_prop_physics(bool a2);

    void remove_from_phys_ifc_list();

    //virtual
    void release_ifc();


    //0x004ECFF0
    //virtual
    void apply_force_increment(const vector3d &a2, force_type arg4, const vector3d &a4, int a5);

    //0x004FB1D0
    static void frame_advance_all_phys_interfaces(Float a1);

    //0x004C9E60
    static vector3d calculate_force_vector_2(const vector3d *a2,
                                             const vector3d *a3,
                                             Float a4,
                                             Float a5);

    //0x004BDEB0
    static void clear_static_lists();

    static inline auto & all_phys_interfaces = var<_std::vector<physical_interface *> *>(0x0095A6B0);

    static std::reference_wrapper<int[512]> rotators ;
    static int & rotators_num;
};

static inline float & g_gravity = var<float>(0x00921E3C);

extern void physical_interface_patch();
