#pragma once

#include "info_node.h"

#include <list.hpp>

struct ai_interaction_data;
struct interact_sound_entry;

namespace ai {
struct strength_velocity_filter {
    float value;
    float weight;
};
struct strength_test_inode : info_node {
    bool active;
    bool advanced;
    char field_1E[2];
    float strength;
    float advanced_strength;
    float elapsed;
    ai_interaction_data *parameters;
    unsigned short advanced_stage;
    short field_32;
    strength_velocity_filter *field_34;
    _std::list<interact_sound_entry *> *field_38;
    int sound;
    int field_40;
    int field_44;
    int current_button;
    int state;


    strength_test_inode();

    explicit strength_test_inode(from_mash_in_place_constructor *constructor);
    static void *native_vtable();
    void _frame_advance(Float time_step);
    double get_curr_button_press();
    void UI_Init();
    void UI_Update();
    void UI_Done();
    void set_new_adv_stage();
    void update_strength(float time_step);
    void update_advanced_strength(float time_step);
    void update_sounds(float time_step, float previous_strength);
    void _activate(ai_core *core);
    void _deactivate();

    static inline string_hash default_id{int(to_hash("strength_test"))};
};
}  // namespace ai
