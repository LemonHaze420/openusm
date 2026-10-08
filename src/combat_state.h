#pragma once

#include "base_state.h"
#include "enhanced_state.h"
#include "float.hpp"
#include "vector3d.h"
#include "attack_impact_sound_event.h"
#include "mVectorBasic.h"
#include "combo_system_move.h"

#include <cstdint>

namespace ai {
struct als_inode;
struct combat_inode;
}  // namespace ai

struct anchor_storage_class;
struct from_mash_in_place_constructor;
struct vector3d;
struct entity;
struct event;
struct entity_base_vhandle;
struct script_object;
struct script_instance;
struct vm_executable;

struct combat_bone_cache {
    entity *bone;
    vector3d position;
};

struct combat_state : ai::enhanced_state {
    ai::combat_inode *field_30;
    ai::als_inode *field_34;
    vector3d field_38;
    vector3d field_44;
    vector3d facing;
    float movement_distance;
    int field_60;
    int field_64;

    vector3d field_68;
    int field_74;
    int field_78;
    int field_7C;
    float field_80;
    float field_84;
    bool field_88;
    unsigned char padding_89[3];
    script_object *field_8C;
    script_instance *field_90;
    int frame_function;
    int activate_function;
    int deactivate_function;
    attack_impact_sound_event field_A0;
    entity_base_vhandle field_B8;
    entity *field_BC;
    entity *field_C0;
    mVectorBasic<combat_bone_cache> field_C4;
    mVectorBasic<entity_base_vhandle> field_D4;
    mVectorBasic<entity_base_vhandle> field_E4;
    int field_F4;
    bool field_F8;
    bool field_F9;
    bool field_FA;
    bool field_FB;
    bool field_FC;
    bool field_FD;
    bool field_FE;
    bool field_FF;
    unsigned callback_ids[12];

    combat_state();
    //0x00471EC0
    combat_state(from_mash_in_place_constructor *a2);
    ~combat_state();
    static void *native_vtable();
    void _unmash(mash_info_struct *info, void *owner);
    void _destruct_mashed_class();
    void _activate(ai::ai_state_machine *machine, const ai::mashed_state *state, const ai::mashed_state *previous,
                   const ai::param_block *params, activate_flag_e flags);
    void _deactivate(const ai::mashed_state *next);
    ai::state_trans_messages _frame_advance(Float delta);
    void advance_movement(Float delta, const combo_system_move *move, bool update, bool start);
    bool advance_area_damage(Float delta);
    bool advance_area_damage_peds(const vector3d *points, int count, actor *owner, const vector3d &position,
                                  const combo_system_move::results &results, entity *target,
                                  struct damage_interface *damage);
    bool advance_communications(Float delta, const combo_system_move *move);
    ai::state_trans_messages check_exit(Float delta);
    void clean_for_exit();
    void call_backs_setup();
    void call_backs_cleanup();
    bool is_sane_range() const
    {
        return field_88;
    }

    //0x00487500
    //virtual
    bool find_web_hang_spot();

    static const inline string_hash default_id{static_cast<int>(to_hash("COMBAT"))};
};
struct player_combat_state : combat_state {
    player_combat_state();
    explicit player_combat_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
};
struct spidey_combat_state : player_combat_state {
    spidey_combat_state();
    explicit spidey_combat_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
};
struct parker_combat_state : spidey_combat_state {
    parker_combat_state();
    explicit parker_combat_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
};
struct cpu_combat_state : combat_state {
    int allow_facing;
    bool previous_allow_facing;
    unsigned char padding_135[3];

    cpu_combat_state();
    explicit cpu_combat_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai::ai_state_machine *machine, const ai::mashed_state *state, const ai::mashed_state *previous,
                   const ai::param_block *params, activate_flag_e flags);
    void _deactivate(const ai::mashed_state *next);
    ai::state_trans_messages _frame_advance(Float delta);
    ai::state_trans_action _process_message(Float delta, ai::state_trans_messages message) const;
    bool stays_in_combat(const ai::mashed_state *next) const;
};


//0x004474B0
extern void web_start_call_back(event *a1, entity_base_vhandle a2, void *a3);

extern void combat_state_patch();
