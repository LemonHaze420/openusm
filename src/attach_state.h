#pragma once

#include "string_hash.h"
#include "variable.h"

namespace ai {

struct attach_state {
    attach_state();

    static const inline string_hash default_id{static_cast<int>(to_hash("ATTACH"))};
};

}  // namespace ai
