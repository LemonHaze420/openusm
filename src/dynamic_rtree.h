#pragma once

#include "bitvector.h"
#include "float.hpp"
#include "rtree_root.h"
#include "variable.h"
#include "entity_base_vhandle.h"

struct vector3d;
struct rtree_node_t;
struct subdivision_visitor;
struct entity;

struct rtree_hash_entry {
    entity_base_vhandle field_0;
    rtree_node_t *entity_aabb;
    rtree_hash_entry *field_8;

    rtree_hash_entry(entity_base_vhandle a2, rtree_node_t *a3);
};

struct rtree_hash_table {
    rtree_hash_entry *field_0[256];

    rtree_hash_entry *get(const entity_base_vhandle &a2);

    //0x0055FAC0
    bool find(const entity_base_vhandle &key, rtree_hash_entry **a3);

    bool insert(const entity_base_vhandle &key, rtree_hash_entry *a3);
};

struct dynamic_rtree_root_state {
    fixed_bitvector<unsigned, 2048> occupied;
    int field_108;
    int field_10C;
    rtree_root_t field_110;
    rtree_hash_table field_140;

    char field_540[148];
    void *field_5D4;
    rtree_node_t *bottom_level;
    int field_5DC[3];

    int BOTTOM_LEVEL_SIZE;
    int field_5EC;

    //0x005709C0
    void sort();

    void occupy(int index);

    int find_free_slot() const;

    void sub_563270(int a2);
};

struct dynamic_rtree_root_t {
    dynamic_rtree_root_state *state;

    //0x005386B0
    void init(int a2, int a3);

    //0x00521500
    void traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor &a4);

    void sort();

    void term();

    //0x0052AFC0
    void update_entity(entity *ent);

    //0x0052B160
    bool remove_entity(entity *a2);
};

extern Var<dynamic_rtree_root_t> collision_dynamic_rtree;
