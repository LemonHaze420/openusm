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
    struct render_data {
        color32 field_0;
        vector3d m_scale;
        float field_10;
        float field_14;

        render_data() : field_0(255, 255, 255, 255), m_scale(1.0, 1.0, 1.0), field_10(1.0), field_14(1.0) {}
    };

    coninfo_t *coninfo;
    _std::vector<vhandle_type<actor>> *ignore_col_ents;
    render_data *field_8;
    movement_info *mi;
    script_instance *my_script;

    advanced_entity_ptrs() = default;

    ~advanced_entity_ptrs();

    //0x004CFCE0
    void un_mash(generic_mash_header *header, actor *arg4, void *a4, generic_mash_data_ptrs *a5);
};
