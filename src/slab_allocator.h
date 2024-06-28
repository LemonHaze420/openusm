#pragma once

#include <config.h>

#include "msimpletemplates.h"

struct debug_menu;

namespace slab_allocator {

    struct slab_t {
        uint32_t begin_sentry;
        char *arena;
        uint16_t field_8;
        uint16_t m_size;
        int16_t total_object_count;
        int16_t alloc_object_count;
        void *field_10;

        struct {
            slab_t *_sl_next_element;
            slab_t *_sl_prev_element;
            simple_list<slab_t *> *_sl_list_owner;
        } simple_list_vars;
        uint32_t end_sentry;

        slab_t(char *a2);

        bool contains(uint32_t a2);

        bool is_set(int a2);

        void set(int16_t a2);

        void unset(uint16_t a2);

        int get_free_object_count();

        int get_alloc_object_count();

        void *get_obj();

        void free_object(void *object);

        int get_total_object_count();

        void sub_592D00(int16_t size);
    };

    //0x0059F5A0
    void initialize();

    //0x0059F750
    void *allocate(int size, slab_t **a2);

    //0x00439820
    uint32_t get_max_object_size();

    //0x0059DE20
    slab_allocator::slab_t *create_slab(int size);

    void create_slab_debug_menu(debug_menu *);

    //0x0059DCA0
    void deallocate(void *a1, slab_t *a2);

    //0x00592D50
    slab_t *find_slab_for_object(void *obj);

    //0x0059AF70
    void process_lists();

    void dump_debug_info();

    constexpr auto SLAB_SIZE = 4096;

    constexpr auto NUM_STATIC_SLABS = 1024;

    extern int allocated_object_count[44];
    extern int free_object_count[44];

    extern int partial_slab_count[44];
    extern int full_slab_count[44];

    extern int free_slab_count;
    extern int total_slab_count;

    struct slab_partial_list_t {
        int field_0;
        simple_list<slab_t *> field_4[44];
    };

} // namespace slab_allocator

extern void slab_allocator_patch();
