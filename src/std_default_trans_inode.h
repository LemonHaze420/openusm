#pragma once

#include "info_node.h"
#include "base_state.h"

#include "variable.h"

namespace ai {

struct std_default_trans_inode : info_node {
    bool enabled;
    bool hit_react_enabled;
    bool hit_avoid_enabled;
    char field_1F;
    float unconscious_trigger;
    bool field_24;
    bool field_25;
    bool field_26;
    int field_28;
    int field_2C;
    void *field_30;

    static void *native_vtable();
    std_default_trans_inode();
    explicit std_default_trans_inode(from_mash_in_place_constructor *constructor);

    void _activate(ai_core *core);
    void _deactivate();
    void _frame_advance(Float time_step);
    void refresh_parameters();

    void set_enabled(bool a2);

    static const inline string_hash default_id{static_cast<int>(to_hash("std_default_trans_inode"))};
};

struct std_default_state_set_base : base_state {
    static constexpr unsigned virtual_type = 370;
    std_default_state_set_base();
    explicit std_default_state_set_base(from_mash_in_place_constructor *constructor);
    static void *native_vtable();
};

}  // namespace ai
