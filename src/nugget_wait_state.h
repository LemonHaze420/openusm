#pragma once

#include "enhanced_state.h"

namespace ai {
struct param_block;

struct nugget_wait_state : enhanced_state {
    ai::param_block *field_30;
    string_hash field_34;
    bool field_38;
    bool field_39;

    state_trans_messages frame_advance(Float a2);

    static inline string_hash wait_anim_hash{to_hash("wait_anim")};

    static inline string_hash looping_wait_anim_hash{to_hash("looping_wait_anim")};
};

}  // namespace ai
