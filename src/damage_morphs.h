#pragma once

#include <cstdint>

struct damage_morph_memory_pool {
    struct allocation {
        void *memory;
        bool live;
        allocation *next;
    };
    allocation *allocation_list;
    allocation *list_end;
    int field_8;
    void *memory_pool;
    uintptr_t field_10;
    uintptr_t field_14;
    uintptr_t field_18;

    explicit damage_morph_memory_pool(int size);
    void init();
    void *memalloc(int alignment, int size);
    uintptr_t memfree(void *memory);
};

struct balanced_tree {
    struct tree_node {
        int key;
        int value;
        tree_node *parent;
        tree_node *left;
        tree_node *right;
        tree_node *next;
        tree_node *previous;
        int height;
    };

    tree_node *root;
    tree_node *oldest;
    tree_node *newest;

    bool retrieve(int key, int *value);
    void add(int key, int value);
    bool remove(int key);
};

struct actor;

struct damage_morphs {
    static void init_memory_pools();
    static bool intercepting_allocations();
    static void *memalloc(int alignment, int size, bool write_combine);
    static void memfree(void *memory);
    static bool is_subject_off_screen(actor *subject);
    static int register_mesh_copy(actor *subject);
    static bool unregister_mesh_copy(int registration_id);
    static void instance_frame_advance(actor *subject);

    static int &allocations_intercept_reference_count;
    static damage_morph_memory_pool &write_combine_pool;
    static damage_morph_memory_pool &normal_pool;
    static balanced_tree &registration_tree;
};
