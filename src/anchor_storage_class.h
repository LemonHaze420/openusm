#pragma once

#include "entity_base_vhandle.h"
#include "entity.h"
#include "vector3d.h"

struct signaller;

struct anchor_storage_class {
    vhandle_type<entity> field_0;
    int field_4;

    anchor_storage_class();

    anchor_storage_class(vhandle_type<entity> a2, entity_base_vhandle a3);

    bool is_valid() const;

    vector3d get_origin() const;

    //0x00464370
    vector3d get_target() const;
};
