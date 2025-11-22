#pragma once

#include "line_info.h"

namespace ai {

struct corner_info {
    line_info field_0;
    int field_5C[19];

    corner_info();

    //0x006B7590
    corner_info(const corner_info &a2);

    void clear();
};
} // namespace ai
