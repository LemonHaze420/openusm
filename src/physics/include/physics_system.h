#pragma once

#include "environment_rigid_body.h"
#include "float.hpp"
#include "variable.h"
#include "phys_memory_pool_base.h"
#include "rbc_def_distance.h"

#include <cstdint>

struct phys_mem_info;
struct phys_memory_heap;
struct rigid_body;
struct user_rigid_body;
struct rigid_body_constraint_contact;
struct rigid_body_constraint_distance;

struct physics_contact_array {
    contact_point_info *points;
    int capacity;
    int count;
};


template <unsigned Size>
struct physics_constraint_storage {
    rigid_body *b1;
    rigid_body *b2;
    char data[Size - 8];
};

struct phys_sys {
    //0x007A1220
    static void phys_init(const phys_mem_info &a1);

    //0x007A11F0
    static void set_collision_callback(void (*a1)());

    //0x007A1130
    static void set_v_tol(int a1, int a2, Float a3);

    //0x007A1190
    static void set_vp_tol(int a1, int a2, Float a3);

    //0x007A1770
    static rigid_body *create_rigid_body();

    static rigid_body_constraint_distance *create_rbc_dist(rigid_body *a1, rigid_body *a2);

    //0x007A1BD0
    static user_rigid_body *create_user_rigid_body();

    static environment_rigid_body *get_environment_rigid_body();

    //0x007A1200
    static void phys_frame_advance(Float a1);

    //0x007A1D60
    static rigid_body_constraint_contact *create_no_error_rbc_contact(rigid_body *a1, rigid_body *a2);

    static void destroy(rigid_body_constraint_distance *a1);

    static void destroy(user_rigid_body *a1);

    static void destroy(rigid_body *body);
    static void destroy_all_constraint(rigid_body *body);

    static void phys_shutdown();
};

struct physics_system {
    int field_0;
    int field_4;
    int field_8;
    void (*m_callback)();
    float field_10;
    int field_14;
    int field_18;
    float field_1C;
    int field_20;
    int field_24;
    float field_28;
    physics_contact_array *field_2C;
    physics_contact_array *field_30;
    environment_rigid_body field_34;
    int field_1A0;
    int field_1A4;
    int field_1A8;
    int field_1AC;
    int field_1B0;
    int field_1B4;
    char *field_1B8;
    int field_1BC;
    int field_1C0;
    list_user_rigid_body field_1C4;
    list_rigid_body field_1D8;
    list_rigid_body_constraint_contact field_1EC;
    contact_point_info *field_200;
    int field_204;
    int field_208;
    contact_point_info *field_20C;
    int field_210;
    int field_214;
    phys_memory_pool<physics_constraint_storage<72>> field_218;
    phys_memory_pool<physics_constraint_storage<228>> field_22C;
    phys_memory_pool<rigid_body_constraint_distance> field_240;
    phys_memory_pool<physics_constraint_storage<320>> field_254;
    phys_memory_pool<physics_constraint_storage<208>> field_268;
    phys_memory_pool<physics_constraint_storage<192>> field_27C;
    char *field_290;
    char *field_294;
    char *field_298;
    int field_29C;
    int field_2A0;
    int field_2A4;
    int field_2A8;
    int field_2AC;
    int field_2B0;
    int field_2B4;
    int field_2B8;
    int field_2BC;
    int field_2C0;
    int field_2C4;
    int field_2C8;
    int field_2CC;
    int field_2D0;
    int field_2D4;

    //0x007AB3B0
    physics_system();

    //0x007AB170
    void frame_advance(Float a2);
    void time_step(float elapsed, bool final_step);

    //0x007AB870
    static void create_inst(const phys_mem_info &a1);

    static void destroy_inst();

    //0x007AADD0
    static uint32_t get_buffer_size(const phys_mem_info &a1);

    static uint32_t get_buffer_alignment();

    //0x007AB5E0
    static physics_system *allocate_buffer(const phys_mem_info &a1, phys_memory_heap &a2);
};

extern physics_system *&g_physics_system;

//0x0059F4D0
extern void physics_system_init();

extern void physics_system_shutdown();

extern void physics_system_patch();
