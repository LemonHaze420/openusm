#pragma once

#include "combat_inode.h"

namespace ai {
struct thug_combat_inode : combat_inode {
    struct blocking_info_t {
        float field_0;
        float field_4;
        float field_8;
        float field_C;
        float field_10;

        blocking_info_t() = default;

        void sub_4458A0(ai_core *a2, const string_hash &a3, const string_hash &a4, const string_hash &a5);
    };

    blocking_info_t field_328;

    //0x00467D10
    //virtual
    void _activate(ai_core *a2);

    static inline string_hash punch_to_block_timer_hash{int(to_hash("punch_to_block_timer"))};

    static inline string_hash punch_to_block_limit_hash{int(to_hash("punch_to_block_limit"))};

    static inline string_hash punch_blocking_timer_hash{int(to_hash("punch_blocking_timer"))};
};
}  // namespace ai
