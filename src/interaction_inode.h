#pragma once

#include "actor.h"
#include "entity_base_vhandle.h"
#include "enum_anim_key.h"
#include "info_node.h"
#include "interaction_type_enum.h"

struct ai_interaction_data;
struct interaction;
struct from_mash_in_place_constructor;
struct anim_key;
struct generic_interaction;

namespace ai {

struct als_inode;

enum interaction_result_enum {};

struct interaction_inode : info_node {
    vhandle_type<actor> target_handle;
    als_inode *target_als;
    int curr_status;
    int field_28;
    ai_interaction_data *field_2C;
    interaction *field_30;
    anim_key *field_34;
    int field_38;
    int field_3C;
    int field_40;
    int target_interaction_type;
    bool field_48;
    bool field_49;

    //0x00483070
    interaction_inode();

    //0x004830B0
    interaction_inode(from_mash_in_place_constructor *a2);

    //0x00479850
    bool is_eligible(string_hash a2, bool a3);

    //0x004514B0
    bool is_in_master_mode();

    bool is_interacting() const
    {
        return this->curr_status == 1;
    }

    void init_interaction();

    void set_interaction(const ai_interaction_data *a2, actor *a3, interaction_type_enum a4, interaction *a5);

    void clear_interaction(interaction_result_enum a2);

    //0x004639E0
    [[nodiscard]] string_hash get_chosen_interact_state_id();

    //0x00451340
    void set_curr_anim(enum_anim_key::key_enum a2);

    void set_curr_anim(anim_key *the_anim_key);

    //0x00451500
    actor *get_target();

    //0x00463980
    void set_flag_from_param(const string_hash &a2, interaction_type_enum a3);

    //0x0046E380
    void set_scripted_start(actor *a2, generic_interaction *a3);
};
}  // namespace ai

extern void interaction_inode_patch();
