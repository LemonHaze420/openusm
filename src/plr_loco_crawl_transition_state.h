#pragma once

#include "enhanced_state.h"

#include "string_hash.h"
#include "variable.h"

namespace ai {
struct ai_state_machine;
struct mashed_state;
struct param_block;
}  // namespace ai

struct plr_loco_crawl_transition_state : ai::enhanced_state {
    plr_loco_crawl_transition_state();

    //virtual
    void activate(ai::ai_state_machine *a2, const ai::mashed_state *a4, const ai::mashed_state *a5,
                  const ai::param_block *a6, ai::base_state::activate_flag_e a7);

    //virtual
    void deactivate(const ai::mashed_state *a1);

    //virtual
    ai::state_trans_messages frame_advance(Float);

    static const inline string_hash default_id{to_hash("crawl_transition")};
};

extern void plr_loco_crawl_transition_state_patch();
