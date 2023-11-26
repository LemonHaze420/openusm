#pragma once

#include "actor.h"
#include "oldmath_po.h"
#include "vector3d.h"

#include <vector.hpp>

struct script_instance;
struct generic_mash_header;
struct generic_mash_data_ptrs;

struct movement_info {
    po field_0;
    float field_40;
    vector3d field_44;
    int field_50;
    bool field_54;
    bool field_55;
};

struct coninfo_t;

struct advanced_entity_ptrs {
    coninfo_t *coninfo;
    _std::vector<vhandle_type<actor>> *ignore_col_ents;
    void *field_8;
    movement_info *mi;
    script_instance *my_script;

    advanced_entity_ptrs();

    ~advanced_entity_ptrs();

    //0x004CFCE0
    void un_mash(generic_mash_header *header, actor *arg4, void *a4, generic_mash_data_ptrs *a5);
};
