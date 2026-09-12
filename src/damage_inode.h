#pragma once

#include "info_node.h"

namespace ai {
struct damage_inode : info_node {
    static inline string_hash default_id{int(to_hash("damage_inode"))};
};
}  // namespace ai
