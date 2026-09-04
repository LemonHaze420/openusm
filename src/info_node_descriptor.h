#pragma once

#include "string_hash.h"

namespace ai {

struct info_node_descriptor {
    string_hash field_0;
    int field_4;

    info_node_descriptor(string_hash a1, int a2);
};

}  // namespace ai
