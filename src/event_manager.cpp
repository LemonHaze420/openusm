#include "event_manager.h"

#include "binary_search_array_cmp.h"
#include "event.h"
#include "event_type.h"
#include "event_recipient_entry.h"
#include "func_wrapper.h"
#include "memory.h"
#include "slab_allocator.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

void event_manager::clear()
{
    if constexpr (0)
    {}
    else
    {
        CDECL_CALL(0x004EE7A0);
    }
}

bool event_manager::callback_exists(int id)
{
    if ( id == 0 ) {
        return false;
    }

    for ( auto &v1 : event_types  )
    {
        if (v1->callback_exists(id) )  {
            return true;
        }
    }

    return false;
}

void event_manager::delete_inst() {
    clear();
}

void event_manager::create_inst()
{
    TRACE("event_manager::create_inst");
    if constexpr (0)
    {
        event_manager::clear();

        event_manager::register_event_type(event::ANIM_ACTION, true);

        event_manager::register_event_type(event::ATTACK_BEGIN, true);

        event_manager::register_event_type(event::ATTACK_END, true);

        event_manager::register_event_type(event::CMBT_CHAIN, true);

        event_manager::register_event_type(event::ATTACK, true);

        event_manager::register_event_type(event::SWAP_PROP_A, true);

        event_manager::register_event_type(event::SWAP_PROP_B, true);

        event_manager::register_event_type(event::FOOTSTEP_L, true);

        event_manager::register_event_type(event::FOOTSTEP_R, true);

        event_manager::register_event_type(event::TERRAIN_FX, true);

        event_manager::register_event_type(event::CAPTURE_FRAME, true);

        event_manager::register_event_type(event::HIDE, true);

        event_manager::register_event_type(event::SOUND_GROUP, true);

        event_manager::register_event_type(event::ATTACK_WHOOSH_SOUND, true);

        event_manager::register_event_type(event::ATTACK_IMPACT_SOUND, true);

        event_manager::register_event_type(event::STORED_ATTACK_IMPACT_SOUND, true);

        event_manager::register_event_type(event::SAY_FILE, true);

        event_manager::register_event_type(event::SAY_SOUND_GROUP, true);

        event_manager::register_event_type(event::SAY_GAB, true);

        event_manager::register_event_type(event::FX_SMALL, true);

        event_manager::register_event_type(event::FX_BIG, true);

        event_manager::register_event_type(event::WEB_START, true);

        event_manager::register_event_type(event::WEB_END, true);

        event_manager::register_event_type(event::MO_START, true);

        event_manager::register_event_type(event::MO_END, true);

        event_manager::register_event_type(event::INTERACTION_FINISHED, true);

        event_manager::register_event_type(event::GRAB_START, true);

        event_manager::register_event_type(event::GRAB_END, true);

        event_manager::register_event_type(event::THROW_START, true);

        event_manager::register_event_type(event::THROW_END, true);

        event_manager::register_event_type(event::WEAPON_SHOW, true);

        event_manager::register_event_type(event::WEAPON_HIDE, true);

        event_manager::register_event_type(event::ACTIVATED_BY_CHARACTER, false);

        event_manager::register_event_type(event::DAMAGED, true);

        event_manager::register_event_type(event::DESTROYED, false);

        event_manager::register_event_type(event::FED_UPON, false);

        event_manager::register_event_type(event::USE_ITEM, false);

        event_manager::register_event_type(event::RUMBLE_LOW, true);

        event_manager::register_event_type(event::RUMBLE_MEDIUM, true);

        event_manager::register_event_type(event::RUMBLE_HIGH, true);

        event_manager::register_event_type(event::RUMBLE_STOP, true);

        event_manager::register_event_type(event::GENERIC_SIGNAL_5, false);

        event_manager::register_event_type(event::AI_STATE_IDLE, false);

        event_manager::register_event_type(event::AI_STATE_ALERTED, false);

        event_manager::register_event_type(event::AI_STATE_COMBAT, false);

        event_manager::register_event_type(event::AI_STATE_INCREMENT, false);

        event_manager::register_event_type(event::AI_STATE_DECREMENT, false);

        event_manager::register_event_type(event::FADED_OUT, false);

        event_manager::register_event_type(event::SCENE_ANIM_FINISHED, false);

        event_manager::register_event_type(event::SCENE_ANIM_ENDING, false);

        event_manager::register_event_type(event::ANIM_DEST_REACHED, true);

        event_manager::register_event_type(event::META_ANIM_UTILITY, true);

        event_manager::register_event_type(event::BOUNCED, false);

        event_manager::register_event_type(event::AI_NEW_THREAT, false);

        event_manager::register_event_type(event::AI_LOST_THREAT, false);

        event_manager::register_event_type(event::BEGIN_PICK_UP, false);

        event_manager::register_event_type(event::PICKED_UP, false);

        event_manager::register_event_type(event::ENTER_LIMBO, false);

        event_manager::register_event_type(event::EXIT_LIMBO, false);

        event_manager::register_event_type(event::HIDE_FINGERS_OF_GOD, false);

        event_manager::register_event_type(event::SHOW_FINGERS_OF_GOD, false);

        event_manager::register_event_type(event::CREDITS_FINISHED, false);

        event_manager::register_event_type(event::BEGIN_PUT_DOWN, false);

        event_manager::register_event_type(event::PUT_DOWN, false);

        event_manager::register_event_type(event::BEING_WEBBED, false);

        event_manager::register_event_type(event::HEAL_START, false);

        event_manager::register_event_type(event::HEAL_DONE, false);

        event_manager::register_event_type(event::RESPAWNED_THIS_FRAME, false);

        event_manager::register_event_type(event::FINISHED_TRAFFIC_GOTO, false);

        event_manager::register_event_type(event::TRAFFIC_CAR_IS_COMBATIVE, false);

        event_manager::register_event_type(event::TRAFFIC_CAR_ENTER_COMBAT, false);

        event_manager::register_event_type(event::TRAFFIC_CAR_EXIT_COMBAT, false);

        event_manager::register_event_type(event::TRAFFIC_CAR_KILLED, false);

        event_manager::register_event_type(event::TGT_ARRESTED, false);

        event_manager::register_event_type(event::TGT_FOLLOW_CAUGHT, false);

        event_manager::register_event_type(event::TGT_DRAINED, true);

        event_manager::register_event_type(event::IN_LIGHT_POLE, true);

        event_manager::register_event_type(event::OUT_LIGHT_POLE, true);

        event_manager::register_event_type(event::AI_CROWD_RESPONSE_CHEER, false);

        event_manager::register_event_type(event::AI_CROWD_RESPONSE_FLEE, false);

        event_manager::register_event_type(event::AI_GET_IN_CAR_FINISHED, false);

        event_manager::register_event_type(event::AI_GET_OUT_OF_CAR_FINISHED, false);

        event_manager::register_event_type(event::CAR_COMBAT_ENGAGED, false);

        event_manager::register_event_type(event::CAR_COMBAT_DISENGAGED, false);

        event_manager::register_event_type(event::CAR_COMBAT_NODE_CHANGED, false);

        event_manager::register_event_type(event::CAR_COMBAT_LEFT_ATTACK, false);

        event_manager::register_event_type(event::CAR_COMBAT_RIGHT_ATTACK, false);

        event_manager::register_event_type(event::CAR_COMBAT_DODGED, false);

        event_manager::register_event_type(event::DELETED, false);

        event_manager::register_event_type(event::SUBDUED, false);

        event_manager::register_event_type(event::ACTIVATE_SPIDEY_SENSE, false);

        event_manager::register_event_type(event::PROP_PHYSICS_START, false);

        event_manager::register_event_type(event::PROP_PHYSICS_STOP, false);

        event_manager::register_event_type(event::COLLIDED_WITH_SWING_WEB, false);

        event_manager::register_event_type(event::TRICK_WALL_JUMP, false);

        event_manager::register_event_type(event::TRICK_POLE_SWING_REVOLUTION, false);

        event_manager::register_event_type(event::AI_PUPPET_1, false);

        event_manager::register_event_type(event::AI_PUPPET_2, false);

        event_manager::register_event_type(event::AI_PUPPET_3, false);

        event_manager::register_event_type(event::AI_PUPPET_4, false);

        event_manager::register_event_type(event::AI_PUPPET_5, false);

        event_manager::register_event_type(event::KILL_SPAWNED_LAYER, false);

        event_manager::register_event_type(event::FORCE_INTERACTION_SUCCESS, true);

        event_manager::register_event_type(event::FORCE_INTERACTION_FAILURE, true);

        event_manager::register_event_type(event::INTERACTION_PRE_ANIMS_PHASE, false);

        event_manager::register_event_type(event::INTERACTION_PRE_INTERACT_PHASE, false);

        event_manager::register_event_type(event::INTERACTION_INTERACT_PHASE, false);

        event_manager::register_event_type(event::INTERACTION_POST_INTERACT_PHASE, false);

        event_manager::register_event_type(event::INTERACTION_POST_FAIL_INTERACT_PHASE, false);

        event_manager::register_event_type(event::INTERACTION_SUCCESS, false);

        event_manager::register_event_type(event::INTERACTION_FAILURE, false);

        event_manager::register_event_type(event::ATTACH_PRE_ANIMS_PHASE, false);

        event_manager::register_event_type(event::ATTACH_PRE_INTERACT_PHASE, false);

        event_manager::register_event_type(event::ATTACH_INTERACT_PHASE, false);

        event_manager::register_event_type(event::ATTACH_POST_INTERACT_PHASE, false);

        event_manager::register_event_type(event::ENTER, false);

        event_manager::register_event_type(event::LEAVE, false);

        event_manager::register_event_type(event::PICKUP, false);

        event_manager::register_event_type(event::USE, false);

        event_manager::register_event_type(event::SCHWING, false);

        event_manager::register_event_type(event::DETONATE, false);

        event_manager::register_event_type(event::ARMED, false);

        event_manager::register_event_type(event::EXPLODE, false);

        event_manager::register_event_type(event::SWITCH_TOGGLE, false);

        event_manager::register_event_type(event::SWITCH_ON, false);

        event_manager::register_event_type(event::SWITCH_OFF, false);

        event_manager::register_event_type(event::MUSIC_TRACK_SWITCH, false);

        event_manager::register_event_type(event::MUSIC_FINISHED, false);

        event_manager::register_event_type(event::TIME_MINUTE_INC, false);

        event_manager::register_event_type(event::TIME_HOUR_INC, false);

        event_manager::register_event_type(event::TIME_DAY_INC, false);

        event_manager::register_event_type(event::X_PRESSED, false);

        event_manager::register_event_type(event::X_RELEASED, false);

        event_manager::register_event_type(event::SQUARE_PRESSED, false);

        event_manager::register_event_type(event::SQUARE_RELEASED, false);

        event_manager::register_event_type(event::TRIANGLE_PRESSED, false);

        event_manager::register_event_type(event::TRIANGLE_RELEASED, false);

        event_manager::register_event_type(event::CIRCLE_PRESSED, false);

        event_manager::register_event_type(event::CIRCLE_RELEASED, false);

        event_manager::register_event_type(event::L1_PRESSED, false);

        event_manager::register_event_type(event::L1_RELEASED, false);

        event_manager::register_event_type(event::L2_PRESSED, false);

        event_manager::register_event_type(event::L2_RELEASED, false);

        event_manager::register_event_type(event::R1_PRESSED, false);

        event_manager::register_event_type(event::R1_RELEASED, false);

        event_manager::register_event_type(event::R2_PRESSED, false);

        event_manager::register_event_type(event::R2_RELEASED, false);

        event_manager::register_event_type(event::LEFT_PRESSED, false);

        event_manager::register_event_type(event::LEFT_RELEASED, false);

        event_manager::register_event_type(event::RIGHT_PRESSED, false);

        event_manager::register_event_type(event::RIGHT_RELEASED, false);

        event_manager::register_event_type(event::UP_PRESSED, false);

        event_manager::register_event_type(event::UP_RELEASED, false);

        event_manager::register_event_type(event::DOWN_PRESSED, false);

        event_manager::register_event_type(event::DOWN_RELEASED, false);

        event_manager::register_event_type(event::RSTICK_LEFT_PRESSED, false);

        event_manager::register_event_type(event::RSTICK_LEFT_RELEASED, false);

        event_manager::register_event_type(event::RSTICK_RIGHT_PRESSED, false);

        event_manager::register_event_type(event::RSTICK_RIGHT_RELEASED, false);

        event_manager::register_event_type(event::RSTICK_UP_PRESSED, false);

        event_manager::register_event_type(event::RSTICK_UP_RELEASED, false);

        event_manager::register_event_type(event::RSTICK_DOWN_PRESSED, false);

        event_manager::register_event_type(event::RSTICK_DOWN_RELEASED, false);

        event_manager::register_event_type(event::LSTICK_LEFT_PRESSED, false);

        event_manager::register_event_type(event::LSTICK_LEFT_RELEASED, false);

        event_manager::register_event_type(event::LSTICK_RIGHT_PRESSED, false);

        event_manager::register_event_type(event::LSTICK_RIGHT_RELEASED, false);

        event_manager::register_event_type(event::LSTICK_UP_PRESSED, false);

        event_manager::register_event_type(event::LSTICK_UP_RELEASED, false);

        event_manager::register_event_type(event::LSTICK_DOWN_PRESSED, false);

        event_manager::register_event_type(event::LSTICK_DOWN_RELEASED, false);

        event_manager::register_event_type(event::START_PRESSED, false);

        event_manager::register_event_type(event::START_RELEASED, false);

        event_manager::register_event_type(event::SELECT_PRESSED, false);

        event_manager::register_event_type(event::SELECT_RELEASED, false);

        event_manager::register_event_type(event::BEETLE_DEVICE_EXPLODE, true);

        event_manager::register_event_type(event::BEETLE_SHOCKWAVE_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_SMOKE_GRENADE_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_PROXIMITY_GRENADE_ATTACK, false);

        event_manager::register_event_type(event::BEETLE_PROXIMITY_BARRIER_ATTACK, false);

        event_manager::register_event_type(event::BEETLE_ACTIVATE_LASER, true);

        event_manager::register_event_type(event::BEETLE_DE_ACTIVATE_LASER, true);

        event_manager::register_event_type(event::BEETLE_LASER_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_LASER_VOLLEY_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_BUMRUSH_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_LEAP_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_LASER_DRAG_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_LASER_DRAG_2PASS_ATTACK, true);

        event_manager::register_event_type(event::BEETLE_LASER_DRAG_3PASS_ATTACK, true);

        event_manager::register_event_type(event::RHINO_WINDOW_OPEN, false);

        event_manager::register_event_type(event::RHINO_WINDOW_CLOSE, false);

        event_manager::register_event_type(event::SABLE_BLOCKS, false);

        event_manager::register_event_type(event::SABLE_RETALIATES, false);

        event_manager::register_event_type(event::HERO_LEAVES_PLAY_AREA, false);

        event_manager::register_event_type(event::HERO_ENTERS_PLAY_AREA, false);

        event_manager::register_event_type(event::COLLISION_EVENT, false);

        event_manager::register_event_type(event::THROW_EVENT, false);

        event_manager::register_event_type(event::BOUNCED_GROUND, false);

        event_manager::register_event_type(event::BOUNCED_UNKNOWN, false);

        event_manager::register_event_type(event::STORED_BOUNCED_TARGET, false);

        event_manager::register_event_type(event::PARTICLE_EMISSION, false);

        event_manager::register_event_type(event::VENOM_CANCEL_MODE, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_JUMP_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_WEBZIP_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_1_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_2_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_3_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_4_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_5_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_6_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_7_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_8_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_9_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_10_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_MELEE_11_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_1_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_2_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_3_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_4_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_5_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_6_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_7_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_8_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_9_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_10_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_RANGED_11_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_1_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_2_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_3_MSG, true);

        event_manager::register_event_type(event::PREVIEW_TRANS_FLEE_FAR_MSG, true);
    }
    else
    {
        CDECL_CALL(0x004F3BE0);
    }
}

int sub_4D1F40(char a1) {
    return CDECL_CALL(0x004D1F40, a1);
}

bool event_manager::does_script_have_callbacks(const script_executable *a1) {
    return (bool) CDECL_CALL(0x004D2000, a1);
}

void event_manager::raise_event(string_hash a1, entity_base_vhandle a2)
{
    if constexpr (1) {
        auto *v2 = event_manager::get_event_type(a1);
        if (v2 != nullptr) {
            v2->raise_event(a2, nullptr);
        }
    } else {
        CDECL_CALL(0x004EE9F0, a1, a2);
    }
}

void event_manager::garbage_collect()
{
    TRACE("event_manager::garbage_collect");

    if constexpr (0)
    {
        if ( event_types.empty()
            || garbage_index >= event_types.size() )
        {
            garbage_index = 0;
        }
        else
        {
            auto it = event_types.begin() + garbage_index;
            auto *v1 = (*it);
            if ( v1->garbage_collect() )
            {
                if ( v1 != nullptr )
                {
                    v1->~event_type();
                    mem_dealloc(v1, sizeof(event_type));

                }

                event_types.erase(it);
            }
            else
            {
                ++garbage_index;
            }
        }
    }
    else
    {
        CDECL_CALL(0x004E1B00);
    }
}

event_type *event_manager::get_event_type(string_hash a1)
{
    if constexpr (1)
    {
        auto index = -1;

        auto size = event_types.size();

        event_type *result = nullptr;

        if (binary_search_array_cmp<string_hash, event_type *>(&a1,
                                                            &event_types.front(),
                                                            0,
                                                            size,
                                                            &index,
                                                            compare_deref<string_hash, event_type *>))
        {
            result = event_types.at(index);
        }

        return result;

    } else {
        return (event_type *) CDECL_CALL(0x004D1F40, a1);
    }
}

string_hash event_manager::register_script_event_type(const char *a2, const char **a3)
{
    auto *v3 = strrchr(a2, ':');
    auto *v4 = ( v3 != nullptr ? v3 + 1 : a2 );

    string_hash v7 {v4};
    if ( a3 != nullptr ) {
        *a3 = v4;
    }

    return v7;
}

event_type *event_manager::register_event_type(string_hash a1, bool a2) {
    if constexpr (1) {
        auto *v2 = event_manager::get_event_type(a1);
        if (v2 != nullptr) {
            if (!v2->field_28 && a2) {
                auto str = a1.to_string();

                sp_log("Attempting to re-register non-pollable event type (%s,0x%x) as pollable",
                       str,
                       a1.source_hash_code);
            }
        } else {
            auto *mem = mem_alloc(sizeof(event_type));
            event_type *the_type = new (mem) event_type{a1, a2};

            assert(the_type != nullptr &&
                   "Need to increase the fixed pool on events (increase MAX_EVENT_TYPES)!!!");

            void (__fastcall *push_back)(void *, void *, void *) = CAST(push_back, 0x005E7330);

            int (*compare)(const void *, const void *) = CAST(compare, 0x005034D0);

            push_back(&event_types, nullptr, &the_type);
            if (event_types.size() > 1) {
                qsort(event_types.m_first, event_types.size(), 4u, compare);
            }
        }

        return v2;
    } else {
        return (event_type *) CDECL_CALL(0x004E19F0, a1, a2);
    }
}

void event_manager::clear_script_callbacks(entity_base_vhandle a1, script_executable *a2)
{
    if constexpr (0)
    {
        for ( auto &v2 : event_types ) {
            v2->clear_script_callbacks(a1, a2);
        }
    }
    else
    {
        CDECL_CALL(0x004D4380, a1, a2);
    }
}

event_recipient_entry * event_manager::create_event_recipient(string_hash arg0, entity_base_vhandle a2)
{
    auto *v2 = event_manager::register_event_type(arg0, 0);
    if ( v2 != nullptr )
        return v2->create_recipient_entry(a2);
    else
        return nullptr;
}

int event_manager::add_callback(
        string_hash a1,
        entity_base_vhandle a2,
        void (*cb)(event *, entity_base_vhandle, void *),
        void *a4,
        bool a5)
{
    auto *entry = create_event_recipient(a1, a2);
    if (entry != nullptr)
    {
        return entry->add_callback(cb, a4, a5);
    }
    else
    {
        assert(0 && "this might be indicative of an error-condition.");
        return 0;
    }
}

void event_manager::remove_callback(unsigned int a1, string_hash a2, entity_base_vhandle a3)
{
    auto *v4 = get_event_type(a2);
    if ( v4 != nullptr )
    {
        v4->remove_default_callback(a1);
        auto *v5 = v4->find_recipient_entry(a3);
        if ( v5 != nullptr ) {
            v5->remove_callback(a1);
        }
    }
}


void event_manager_patch()
{
    REDIRECT(0x005E1118, event_manager::create_inst);

    SET_JUMP(0x004EE9F0, event_manager::raise_event);

    REDIRECT(0x005D707A, event_manager::garbage_collect);
}

