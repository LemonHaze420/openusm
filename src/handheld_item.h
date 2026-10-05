#pragma once

#include "item.h"

#include "visual_item.h"

struct entity_base;

struct handheld_item : item {
    int field_100;
    vhandle_type<visual_item> field_104;
    entity_base *field_108;
    int field_10C;
    bool field_110;

    static void *native_vtable(void **item_table);
    ~handheld_item();
    void release_mem();
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);
    void frame_advance(Float);
    void set_owner(actor *);
    void create_visual_item();
    void draw(bool);
    void holster(bool);
    void show();
    void hide();
    void set_visibility(bool);
    void detach();
};
