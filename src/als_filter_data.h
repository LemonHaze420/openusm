#pragma once

#include <cstdint>

struct from_mash_in_place_constructor;
struct mash_info_struct;

namespace als {
struct filter_data {
    uint32_t field_0;
    float field_4;
    float field_8;

    filter_data(from_mash_in_place_constructor *) {}

    void unmash(mash_info_struct *, void *) {}
};
}  // namespace als
