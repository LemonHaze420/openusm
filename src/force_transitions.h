#pragma once

#include "string_hash.h"
#include "mvector.h"

struct from_mash_in_place_constructor;

namespace als {
struct alter_conditions;

struct force_transitions {
    string_hash field_0;
    mVector<als::alter_conditions> field_4;

    force_transitions();

    force_transitions(from_mash_in_place_constructor *a2);

    void unmash(mash_info_struct *a1, void *);
};
}  // namespace als
