#pragma once

#include "string_hash.h"

struct from_mash_in_place_constructor;

namespace als {

    struct dest_weight_data {
        string_hash field_0;
        float field_4;

        dest_weight_data(from_mash_in_place_constructor * a2) : field_0(a2) {}
    };
}
