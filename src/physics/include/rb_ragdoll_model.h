#pragma once

#include "float.hpp"
#include "phys_anim_bone_array.h"

#include <cassert>

struct phys_vector3d;
struct rigid_body;

struct ragdoll_callbacks {
    void *m_calc_bone_mat_from_rb;
    void *m_calc_rb_mat_from_bone;
};

template <typename T>
struct phys_array {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    T m_data;
    int m_alloc_count;

    auto &operator[](int i)
    {
        assert(i >= 0 && i < m_alloc_count);

        return this->m_data[i];
    }
};

struct rb_ragdoll_model {
    phys_array<rigid_body **> m_list_rigid_body;
    int field_30[252];

    bool field_420;
    int field_424;
    bool field_428;
    int field_42C;
    float field_430;
    float field_434;
    float field_438;
    int field_43C;
    int field_440;
    int field_444;
    int field_448;
    int field_44C;
    int field_450;
    int field_454;
    int field_458;
    int field_45C;
    phys_anim_bone_array field_460;

    rb_ragdoll_model();

    rigid_body *add_rigid_body(int rb_id);

    void reset_state_variables();

    void sub_4ADEF0(int i, const phys_vector3d &a3);

    //0x007A09A0
    void apply_pulse(int i, const phys_vector3d &a3);

    //0x007A0300
    void get_ballistic_info(phys_vector3d *a2, phys_vector3d *a3, float *a4);

    //0x007A0810
    void update_stability(Float a2);

    void set_max_rb_index(int a2);

    //0x007A00D0
    static void set_ragdoll_callbacks(const ragdoll_callbacks &a1);
};
