#pragma once

#include "string_hash.h"
#include "variable.h"

namespace ai {

struct put_down_state {
    put_down_state();

    static const inline string_hash default_id{static_cast<int>(to_hash("put_down"))};
};

}  // namespace ai
