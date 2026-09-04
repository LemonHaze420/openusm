#pragma once

#include "actor.h"
#include "entity_base_vhandle.h"
#include "mvector.h"
#include "mVectorBasic.h"
#include "string_hash.h"
#include "variable.h"

namespace ai {
struct combat_target_inode {
    static inline Var<string_hash> team_hash{0x0096C470};
};

struct player_web_target_inode {
    static void add_to_web_targets_list(vhandle_type<actor> a1);

    static inline auto &web_targets_list = var<mVectorBasic<vhandle_type<actor>>>(0x0096BFD4);
};
}  // namespace ai
