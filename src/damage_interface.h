#pragma once

#include "generic_interface.h"

#include "cached_special_effect.h"
#include "float.hpp"
#include "vector3d.h"

#include <list.hpp>
#include <vector.hpp>

struct actor;
struct resource_key;

template<typename T>
struct bounded_variable {
    T field_0[4];

    void sub_48BFB0(const T &a2);
};

struct damage_info {
    string_hash field_0;
    vector3d field_4;
    vector3d field_8;
    vector3d field_C;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int field_38;
    bool field_3C;
    bool field_3D;
    bool field_3E;
    bool field_3F;
};

struct damage_interface : generic_interface {
    actor *field_4;
    bool dynamic;
    mString field_C;
    mString field_1C;
    mString field_2C;
    mString field_3C;
    char field_4C[0xA8];

    bool field_F4;
    float field_F8;
    resource_key field_FC;
    damage_info field_104;
    damage_info field_144;
    cached_special_effect field_184;
    float field_1C4;
    string_hash field_1C8;
    void *field_1CC;
    void *field_1D0;
    float field_1D4;
    float field_1D8;
    int field_1DC;
    int field_1E0;
    float field_1E4;
    float field_1E8;
    float field_1EC;
    float field_1F0;
    bool field_1F4;
    bool field_1F5;
    int field_1F8;
    bounded_variable<float> field_1FC;
    bounded_variable<float> field_20C;
    bounded_variable<float> field_21C;
    bounded_variable<int> field_22C;

    //0x004DE8A0
    damage_interface(actor *a2);

    //0x004D9BF0
    ~damage_interface();

    void remove_from_dmg_ifc_list();

    /* virtual */ bool get_ifc_num(const resource_key &att, float *a3, bool is_log);

    /* virtual */ bool set_ifc_num(const resource_key &att, Float a3, bool is_log);

    //virtual
    void _un_mash(
        generic_mash_header *a2,
        void *a3,
        void *a4,
        generic_mash_data_ptrs *a5);

    //virtual
    void release_ifc();

    //virtual
    void frame_advance(Float a3);

    //0x004D1990
    static void frame_advance_all_damage_ifc(Float a1);

    static inline auto & all_damage_interfaces = var<_std::vector<damage_interface *> *>(0x0095A660);

    static inline auto & found_damageable = var<_std::list<damage_interface *> *>(0x0095A5EC);
};


extern void damage_interface_patch();
