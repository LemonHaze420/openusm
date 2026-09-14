#pragma once

#include "throw_state.h"

namespace ai {
struct aimed_throw_state : throw_state {
    int field_7C;
    int field_80;
    int field_84;
    int field_88;
    bool field_8C;

    static inline string_hash default_id{int(to_hash("AIMED_THROW"))};
};

}  // namespace ai
