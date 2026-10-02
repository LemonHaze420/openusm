#pragma once

#include "info_node.h"

namespace ai {
struct strength_test_inode : info_node {
    static inline string_hash default_id{int(to_hash("strength_test"))};
};
}  // namespace ai
