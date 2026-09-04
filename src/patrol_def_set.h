#pragma once

#include "mvector.h"

struct from_mash_in_place_constructor;
struct patrol_def;

struct patrol_def_set {
    mVector<patrol_def> field_0;

    patrol_def_set(from_mash_in_place_constructor *a2);

    void unmash(mash_info_struct *a1, void *a3);

    void destruct_mashed_class();
};
