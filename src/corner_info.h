#pragma once

#include "line_info.h"

namespace ai {

struct corner_info {
    line_info field_0;
    int field_5C[19];

    corner_info();

    explicit corner_info(from_mash_in_place_constructor *constructor);

    //0x006B7590
    corner_info(const corner_info &a2);

    corner_info &operator=(const corner_info &source);

    void clear();
};
}  // namespace ai
