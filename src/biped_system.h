#pragma once

#include "physical_interface.h"
#include "rb_ragdoll_model.h"
#include "rb_sphere_list.h"

struct conglomerate;

struct alignas(16) biped_system {
    rb_ragdoll_model field_0;
    char field_1564[12];
    conglomerate *owner;
    char field_1574[12];
    rigid_body_sphere_list sphere_slots[10];
    rigid_body_sphere_list *spheres[10];
    int sphere_indices[10];
    rigid_body_sphere_list *sphere_storage;
    int sphere_count;
    char field_2078[8];
    biped_sphere_pair pair_slots[7];
    biped_sphere_pair *pairs[7];
    int pair_indices[7];
    biped_sphere_pair *pair_storage;
    int pair_count;
    char field_20F8[8];
    bool contact;
    char field_2101[3];
    vector3d contact_normal;
    vector3d contact_position;
    int field_211C;
    vector3d restored_position;
    float collision_diameter;
    int field_2130;
    float field_2134;
    float field_2138;
    bool field_213C;
    bool field_213D;

    biped_system();

    //0x005A2260
    void create_bps(conglomerate *a2, int a3, physical_interface::biped_physics_body_types arg4a);

    //0x005A0CB0
    void setup_physics(conglomerate *a2, physical_interface::biped_physics_body_types arg4);

    void prolog_frame_advance(Float a2);
    void initialize_storage();
    void epilog_frame_advance(Float elapsed);
    rigid_body_sphere_list *find_spheres(int body_index);
    static void process_biped_physics(Float elapsed);
    void gather_collisions();
    static void collision_callback();
};

struct biped_system_pool {
    biped_system slots[9];
    biped_system *allocated[9];
    int indices[9];
    biped_system *slot_array;
    int count;

    void destroy_member(biped_system *biped);
    biped_system *create_member();
};

extern biped_system_pool *&g_biped_system_pool;
void destroy_biped_ragdoll(biped_system *biped);
biped_system *create_biped_ragdoll(conglomerate *owner, int flags, physical_interface::biped_physics_body_types type);
