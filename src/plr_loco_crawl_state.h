#pragma once

#include "enhanced_state.h"

#include "float.hpp"
#include "string_hash.h"
#include "variable.h"
#include "vector3d.h"

namespace ai {
struct ai_state_machine;
struct mashed_state;
struct param_block;
}  // namespace ai

struct actor;

struct plr_loco_crawl_state : ai::enhanced_state {
    int field_30;
    float m_wallrun_deviation;
    vector3d field_38;
    int field_44;

    plr_loco_crawl_state();
    explicit plr_loco_crawl_state(from_mash_in_place_constructor *);
    static void *native_vtable();
    void map_controls(int);
    void get_info_node_list(ai::info_node_desc_list &);

    //0x0046A080
    void activate(ai::ai_state_machine *a2, const ai::mashed_state *a3, const ai::mashed_state *a4,
                  const ai::param_block *a5, ai::base_state::activate_flag_e a6);

    //virtual
    void deactivate(const ai::mashed_state *a1);

    //virtual
    ai::state_trans_messages frame_advance(Float);

    void set_player_mode(actor *a1);

    //0x0044B760
    //virtual
    void update_wallrun(Float a2);

    static const inline string_hash default_id{to_hash("crawl")};

    static const inline string_hash crawl_als_category_hash{int(to_hash("Crawling"))};
};

extern void plr_loco_crawl_state_patch();
