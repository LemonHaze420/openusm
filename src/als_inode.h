#pragma once

#include "als_layer_types.h"
#include "info_node.h"

#include "variable.h"

namespace als {

struct state_machine;
struct animation_logic_system;
struct param_list;

}  // namespace als

namespace ai {

struct als_inode_render_debug;
struct ai_core;

struct als_inode : info_node {
    als::animation_logic_system *field_1C;

    als_inode_render_debug *field_20;
    float field_24;
    string_hash field_28;

    als_inode();

    als::animation_logic_system *get_system()
    {
        return this->field_1C;
    }

    void set_desired_params(als::param_list &a2, als::layer_types a3);

    bool is_layer_interruptable(als::layer_types a1);

    string_hash get_state_id(als::layer_types a3);

    bool is_cat_our_prev_cat(string_hash a2, als::layer_types a3);

    bool anim_finished(string_hash a2, als::layer_types a3);

    //0x00689BA0
    als::state_machine *get_als_layer(als::layer_types a2);

    //0x0048B120
    string_hash get_category_id(als::layer_types a3);

    //0x00689D80
    void request_category_transition(string_hash a2, als::layer_types a3, bool a4, bool a5, bool a6);

    //0x0096C0F4
    static const inline string_hash default_id{to_hash("als")};

    //0x00693770
    //virtual
    void activate(ai_core *a2);

    //virtual
    void set_known_combat_signal_time_and_category(Float a2, string_hash a3);

    //0x00689C20
    //virtual
    float get_eta_of_combat_signal(als::layer_types a2);
};

}  // namespace ai

extern void als_inode_patch();
