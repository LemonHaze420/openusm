#pragma once

#include "string_hash.h"
#include "variable.h"

namespace ai {

struct throw_state {
    throw_state();

    static const inline string_hash default_id{to_hash("THROW")};
};

}  // namespace ai
