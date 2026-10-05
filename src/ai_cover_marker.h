#pragma once

#include "marker.h"
#include "variable.h"

struct ai_cover_marker;

struct ai_cover_marker_list {
    ai_cover_marker *head;
    ai_cover_marker *tail;
    uint32_t size;
};

struct ai_cover_marker : marker {
    ai_cover_marker *next_cover_marker;
    ai_cover_marker *previous_cover_marker;
    ai_cover_marker_list *cover_marker_list;
    string_hash cover_id;

    static Var<ai_cover_marker_list> all_cover_markers;

    ai_cover_marker(const string_hash &, uint32_t);
    ~ai_cover_marker();

    static void *native_vtable(void **entity_table);
    void release_mem();
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);

private:
    void add_to_cover_list();
    void remove_from_cover_list();
};
