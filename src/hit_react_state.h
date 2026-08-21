#pragma once

#include "string_hash.h"
#include "variable.h"

namespace ai {

struct hit_react_state {
    hit_react_state();

    static const inline string_hash default_id{to_hash("hit_react")};
};

}  // namespace ai
