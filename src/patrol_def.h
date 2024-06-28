#pragma once

#include "mash.h"
#include "string_hash.h"
#include "vector3d.h"

struct from_mash_in_place_constructor;

struct patrol_def {
    string_hash field_0;
    vector3d field_4;
    int neighborhood_id;
    int field_14;
    int num_nodes;
    int skill_filter;
    int script_difficulty;
    int field_24;

    patrol_def(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    void clear();

    string_hash get_id_hash() const {
        return this->field_0;
    }
};
