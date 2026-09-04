#pragma once

#include "convex_box.h"
#include "trigger.h"

struct box_trigger : trigger {
    entity_base_vhandle field_58;
    vector3d field_5C;
    vector3d field_68;
    convex_box box;

    //0x0053C870
    box_trigger(string_hash a2, const vector3d &a3);

    //0x0053C940
    box_trigger(string_hash a2, entity_base *a3);

    entity_base *get_box_ent() const;

    void set_box_info(const convex_box &a2);

    /* virtual */ bool triggered(const vector3d &a2);

    /* virtual */ bool _is_point_trigger();

    /* virtual */ bool _is_box_trigger();

    /* virtual */ bool _is_entity_trigger();

    /* virtual */ bool get_bounding_sphere(vector3d *a2, float *a3);

    //virtual
    void update_center();
};
