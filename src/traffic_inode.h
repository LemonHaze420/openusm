#pragma once

#include "info_node.h"

namespace ai {

struct traffic_inode : info_node {
    int field_1C[44];

    traffic_inode();

    static inline string_hash default_id{int(to_hash("TRAFFIC"))};
};

}  // namespace ai
