#pragma once

#include "actor.h"
#include "resource_key.h"

struct conglomerate_clone : actor {
    entity_base_vhandle field_C0;
    resource_key *field_C4;
    int *field_C8;
    entity_base_vhandle *field_CC;

    conglomerate_clone(string_hash const &a2, unsigned a3);
    ~conglomerate_clone();
    void common_destruct();
    void release_mem();
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);
    float _get_visual_radius();
    vector3d _get_visual_center();
    bool _is_renderable();
    void _render(Float);
    static void *native_vtable(void **actor_table);
};
