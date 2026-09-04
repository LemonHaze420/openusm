#pragma once

#include "string_hash.h"
#include "variable.h"

namespace ai {

struct subdued_state {
    subdued_state();

    static inline string_hash default_id{int(to_hash("subdued"))};
};

}  // namespace ai
