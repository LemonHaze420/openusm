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

namespace event_manager {
#if STANDALONE_SYSTEM
namespace {
int garbage_index_storage{};
_std::vector<event_type *> event_types_storage{};
}
int &garbage_index = garbage_index_storage;
_std::vector<event_type *> &event_types = event_types_storage;
#else
int &garbage_index = var<int>(0x0095A6DC);
_std::vector<event_type *> &event_types = var<_std::vector<event_type *>>(0x0095BA48);
#endif
}  // namespace event_manager

void event_manager::clear()
{
#if STANDALONE_SYSTEM
    for (auto *type : event_types) {
        delete type;
    }
    event_types.clear();
    garbage_index = 0;
#else
    CDECL_CALL(0x004EE7A0);
#endif
}

bool event_manager::callback_exists(int id)
{
    if (id == 0) {
        return false;
    }

    for (auto &v1 : event_types) {
        if (v1->callback_exists(id)) {
            return true;
        }
    }

    return false;
}

void event_manager::delete_inst()
{
    clear();
}

void event_manager::create_inst()
{
    TRACE("event_manager::create_inst");
    if constexpr (STANDALONE_SYSTEM) {
        clear();

        register_event_type(event::ANIM_ACTION, true);

        register_event_type(event::ATTACK_BEGIN, true);

        register_event_type(event::ATTACK_END, true);

        register_event_type(event::CMBT_CHAIN, true);

        register_event_type(event::ATTACK, true);

        register_event_type(event::SWAP_PROP_A, true);

        register_event_type(event::SWAP_PROP_B, true);

        register_event_type(event::FOOTSTEP_L, true);

        register_event_type(event::FOOTSTEP_R, true);

        register_event_type(event::TERRAIN_FX, true);

        register_event_type(event::CAPTURE_FRAME, true);

        register_event_type(event::HIDE, true);

        register_event_type(event::SOUND_GROUP, true);

        register_event_type(event::ATTACK_WHOOSH_SOUND, true);

        register_event_type(event::ATTACK_IMPACT_SOUND, true);

        register_event_type(event::STORED_ATTACK_IMPACT_SOUND, true);

        register_event_type(event::SAY_FILE, true);

        register_event_type(event::SAY_SOUND_GROUP, true);

        register_event_type(event::SAY_GAB, true);

        register_event_type(event::FX_SMALL, true);

        register_event_type(event::FX_BIG, true);

        register_event_type(event::WEB_START, true);

        register_event_type(event::WEB_END, true);

        register_event_type(event::MO_START, true);

        register_event_type(event::MO_END, true);

        register_event_type(event::INTERACTION_FINISHED, true);

        register_event_type(event::GRAB_START, true);

        register_event_type(event::GRAB_END, true);

        register_event_type(event::THROW_START, true);

        register_event_type(event::THROW_END, true);

        register_event_type(event::WEAPON_SHOW, true);

        register_event_type(event::WEAPON_HIDE, true);

        register_event_type(event::ACTIVATED_BY_CHARACTER, false);

        register_event_type(event::DAMAGED, true);

        register_event_type(event::DESTROYED, false);

        register_event_type(event::FED_UPON, false);

        register_event_type(event::USE_ITEM, false);

        register_event_type(event::RUMBLE_LOW, true);

        register_event_type(event::RUMBLE_MEDIUM, true);

        register_event_type(event::RUMBLE_HIGH, true);

        register_event_type(event::RUMBLE_STOP, true);

        register_event_type(event::GENERIC_SIGNAL_5, false);

        register_event_type(event::AI_STATE_IDLE, false);

        register_event_type(event::AI_STATE_ALERTED, false);

        register_event_type(event::AI_STATE_COMBAT, false);

        register_event_type(event::AI_STATE_INCREMENT, false);

        register_event_type(event::AI_STATE_DECREMENT, false);

        register_event_type(event::FADED_OUT, false);

        register_event_type(event::SCENE_ANIM_FINISHED, false);

        register_event_type(event::SCENE_ANIM_ENDING, false);

        register_event_type(event::ANIM_DEST_REACHED, true);

        register_event_type(event::META_ANIM_UTILITY, true);

        register_event_type(event::BOUNCED, false);

        register_event_type(event::AI_NEW_THREAT, false);

        register_event_type(event::AI_LOST_THREAT, false);

        register_event_type(event::BEGIN_PICK_UP, false);

        register_event_type(event::PICKED_UP, false);

        register_event_type(event::ENTER_LIMBO, false);

        register_event_type(event::EXIT_LIMBO, false);

        register_event_type(event::HIDE_FINGERS_OF_GOD, false);

        register_event_type(event::SHOW_FINGERS_OF_GOD, false);

        register_event_type(event::CREDITS_FINISHED, false);

        register_event_type(event::BEGIN_PUT_DOWN, false);

        register_event_type(event::PUT_DOWN, false);

        register_event_type(event::BEING_WEBBED, false);

        register_event_type(event::HEAL_START, false);

        register_event_type(event::HEAL_DONE, false);

        register_event_type(event::RESPAWNED_THIS_FRAME, false);

        register_event_type(event::FINISHED_TRAFFIC_GOTO, false);

        register_event_type(event::TRAFFIC_CAR_IS_COMBATIVE, false);

        register_event_type(event::TRAFFIC_CAR_ENTER_COMBAT, false);

        register_event_type(event::TRAFFIC_CAR_EXIT_COMBAT, false);

        register_event_type(event::TRAFFIC_CAR_KILLED, false);

        register_event_type(event::TGT_ARRESTED, false);

        register_event_type(event::TGT_FOLLOW_CAUGHT, false);

        register_event_type(event::TGT_DRAINED, true);

        register_event_type(event::IN_LIGHT_POLE, true);

        register_event_type(event::OUT_LIGHT_POLE, true);

        register_event_type(event::AI_CROWD_RESPONSE_CHEER, false);

        register_event_type(event::AI_CROWD_RESPONSE_FLEE, false);

        register_event_type(event::AI_GET_IN_CAR_FINISHED, false);

        register_event_type(event::AI_GET_OUT_OF_CAR_FINISHED, false);

        register_event_type(event::CAR_COMBAT_ENGAGED, false);

        register_event_type(event::CAR_COMBAT_DISENGAGED, false);

        register_event_type(event::CAR_COMBAT_NODE_CHANGED, false);

        register_event_type(event::CAR_COMBAT_LEFT_ATTACK, false);

        register_event_type(event::CAR_COMBAT_RIGHT_ATTACK, false);

        register_event_type(event::CAR_COMBAT_DODGED, false);

        register_event_type(event::DELETED, false);

        register_event_type(event::SUBDUED, false);

        register_event_type(event::ACTIVATE_SPIDEY_SENSE, false);

        register_event_type(event::PROP_PHYSICS_START, false);

        register_event_type(event::PROP_PHYSICS_STOP, false);

        register_event_type(event::COLLIDED_WITH_SWING_WEB, false);

        register_event_type(event::TRICK_WALL_JUMP, false);

        register_event_type(event::TRICK_POLE_SWING_REVOLUTION, false);

        register_event_type(event::AI_PUPPET_1, false);

        register_event_type(event::AI_PUPPET_2, false);

        register_event_type(event::AI_PUPPET_3, false);

        register_event_type(event::AI_PUPPET_4, false);

        register_event_type(event::AI_PUPPET_5, false);

        register_event_type(event::KILL_SPAWNED_LAYER, false);

        register_event_type(event::FORCE_INTERACTION_SUCCESS, true);

        register_event_type(event::FORCE_INTERACTION_FAILURE, true);

        register_event_type(event::INTERACTION_PRE_ANIMS_PHASE, false);

        register_event_type(event::INTERACTION_PRE_INTERACT_PHASE, false);

        register_event_type(event::INTERACTION_INTERACT_PHASE, false);

        register_event_type(event::INTERACTION_POST_INTERACT_PHASE, false);

        register_event_type(event::INTERACTION_POST_FAIL_INTERACT_PHASE, false);

        register_event_type(event::INTERACTION_SUCCESS, false);

        register_event_type(event::INTERACTION_FAILURE, false);

        register_event_type(event::ATTACH_PRE_ANIMS_PHASE, false);

        register_event_type(event::ATTACH_PRE_INTERACT_PHASE, false);

        register_event_type(event::ATTACH_INTERACT_PHASE, false);

        register_event_type(event::ATTACH_POST_INTERACT_PHASE, false);

        register_event_type(event::ENTER, false);

        register_event_type(event::LEAVE, false);

        register_event_type(event::PICKUP, false);

        register_event_type(event::USE, false);

        register_event_type(event::SCHWING, false);

        register_event_type(event::DETONATE, false);

        register_event_type(event::ARMED, false);

        register_event_type(event::EXPLODE, false);

        register_event_type(event::SWITCH_TOGGLE, false);

        register_event_type(event::SWITCH_ON, false);

        register_event_type(event::SWITCH_OFF, false);

        register_event_type(event::MUSIC_TRACK_SWITCH, false);

        register_event_type(event::MUSIC_FINISHED, false);

        register_event_type(event::TIME_MINUTE_INC, false);

        register_event_type(event::TIME_HOUR_INC, false);

        register_event_type(event::TIME_DAY_INC, false);

        register_event_type(event::X_PRESSED, false);

        register_event_type(event::X_RELEASED, false);

        register_event_type(event::SQUARE_PRESSED, false);

        register_event_type(event::SQUARE_RELEASED, false);

        register_event_type(event::TRIANGLE_PRESSED, false);

        register_event_type(event::TRIANGLE_RELEASED, false);

        register_event_type(event::CIRCLE_PRESSED, false);

        register_event_type(event::CIRCLE_RELEASED, false);

        register_event_type(event::L1_PRESSED, false);

        register_event_type(event::L1_RELEASED, false);

        register_event_type(event::L2_PRESSED, false);

        register_event_type(event::L2_RELEASED, false);

        register_event_type(event::R1_PRESSED, false);

        register_event_type(event::R1_RELEASED, false);

        register_event_type(event::R2_PRESSED, false);

        register_event_type(event::R2_RELEASED, false);

        register_event_type(event::LEFT_PRESSED, false);

        register_event_type(event::LEFT_RELEASED, false);

        register_event_type(event::RIGHT_PRESSED, false);

        register_event_type(event::RIGHT_RELEASED, false);

        register_event_type(event::UP_PRESSED, false);

        register_event_type(event::UP_RELEASED, false);

        register_event_type(event::DOWN_PRESSED, false);

        register_event_type(event::DOWN_RELEASED, false);

        register_event_type(event::RSTICK_LEFT_PRESSED, false);

        register_event_type(event::RSTICK_LEFT_RELEASED, false);

        register_event_type(event::RSTICK_RIGHT_PRESSED, false);

        register_event_type(event::RSTICK_RIGHT_RELEASED, false);

        register_event_type(event::RSTICK_UP_PRESSED, false);

        register_event_type(event::RSTICK_UP_RELEASED, false);

        register_event_type(event::RSTICK_DOWN_PRESSED, false);

        register_event_type(event::RSTICK_DOWN_RELEASED, false);

        register_event_type(event::LSTICK_LEFT_PRESSED, false);

        register_event_type(event::LSTICK_LEFT_RELEASED, false);

        register_event_type(event::LSTICK_RIGHT_PRESSED, false);

        register_event_type(event::LSTICK_RIGHT_RELEASED, false);

        register_event_type(event::LSTICK_UP_PRESSED, false);

        register_event_type(event::LSTICK_UP_RELEASED, false);

        register_event_type(event::LSTICK_DOWN_PRESSED, false);

        register_event_type(event::LSTICK_DOWN_RELEASED, false);

        register_event_type(event::START_PRESSED, false);

        register_event_type(event::START_RELEASED, false);

        register_event_type(event::SELECT_PRESSED, false);

        register_event_type(event::SELECT_RELEASED, false);

        register_event_type(event::BEETLE_DEVICE_EXPLODE, true);

        register_event_type(event::BEETLE_SHOCKWAVE_ATTACK, true);

        register_event_type(event::BEETLE_SMOKE_GRENADE_ATTACK, true);

        register_event_type(event::BEETLE_PROXIMITY_GRENADE_ATTACK, false);

        register_event_type(event::BEETLE_PROXIMITY_BARRIER_ATTACK, false);

        register_event_type(event::BEETLE_ACTIVATE_LASER, true);

        register_event_type(event::BEETLE_DE_ACTIVATE_LASER, true);

        register_event_type(event::BEETLE_LASER_ATTACK, true);

        register_event_type(event::BEETLE_LASER_VOLLEY_ATTACK, true);

        register_event_type(event::BEETLE_BUMRUSH_ATTACK, true);

        register_event_type(event::BEETLE_LEAP_ATTACK, true);

        register_event_type(event::BEETLE_LASER_DRAG_ATTACK, true);

        register_event_type(event::BEETLE_LASER_DRAG_2PASS_ATTACK, true);

        register_event_type(event::BEETLE_LASER_DRAG_3PASS_ATTACK, true);

        register_event_type(event::RHINO_WINDOW_OPEN, false);

        register_event_type(event::RHINO_WINDOW_CLOSE, false);

        register_event_type(event::SABLE_BLOCKS, false);

        register_event_type(event::SABLE_RETALIATES, false);

        register_event_type(event::HERO_LEAVES_PLAY_AREA, false);

        register_event_type(event::HERO_ENTERS_PLAY_AREA, false);

        register_event_type(event::COLLISION_EVENT, false);

        register_event_type(event::THROW_EVENT, false);

        register_event_type(event::BOUNCED_GROUND, false);

        register_event_type(event::BOUNCED_UNKNOWN, false);

        register_event_type(event::STORED_BOUNCED_TARGET, false);

        register_event_type(event::PARTICLE_EMISSION, false);

        register_event_type(event::VENOM_CANCEL_MODE, true);

        register_event_type(event::PREVIEW_TRANS_JUMP_MSG, true);

        register_event_type(event::PREVIEW_TRANS_WEBZIP_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_1_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_2_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_3_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_4_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_5_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_6_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_7_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_8_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_9_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_10_MSG, true);

        register_event_type(event::PREVIEW_TRANS_MELEE_11_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_1_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_2_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_3_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_4_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_5_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_6_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_7_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_8_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_9_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_10_MSG, true);

        register_event_type(event::PREVIEW_TRANS_RANGED_11_MSG, true);

        register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_MSG, true);

        register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_1_MSG, true);

        register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_2_MSG, true);

        register_event_type(event::PREVIEW_TRANS_FLEE_NEAR_3_MSG, true);

        register_event_type(event::PREVIEW_TRANS_FLEE_FAR_MSG, true);
    } else {
        CDECL_CALL(0x004F3BE0);
    }
}

int sub_4D1F40(char a1)
{
    return CDECL_CALL(0x004D1F40, a1);
}

bool event_manager::does_script_have_callbacks(const script_executable *a1)
{
    return (bool)CDECL_CALL(0x004D2000, a1);
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

void event_manager::raise_event(event *event_to_raise, entity_base_vhandle a2)
{
    assert(event_to_raise != nullptr);

    CDECL_CALL(0x004EEA20, event_to_raise, a2);
}

void event_manager::garbage_collect()
{
    TRACE("event_manager::garbage_collect");

    if constexpr (1) {
        if (event_types.empty() || garbage_index >= int(event_types.size())) {
            garbage_index = 0;
        } else {
            auto it = event_types.begin() + garbage_index;
            auto *v1 = (*it);
            if (v1->garbage_collect()) {
                if (v1 != nullptr) {
                    delete v1;
                }

                event_types.erase(it);
            } else {
                ++garbage_index;
            }
        }
    } else {
        CDECL_CALL(0x004E1B00);
    }
}

event_type *event_manager::get_event_type(string_hash a1)
{
    if constexpr (1) {
        auto index = -1;

        auto size = event_types.size();

        event_type *result = nullptr;

        if (binary_search_array_cmp<string_hash, event_type *>(
                &a1, &event_types.front(), 0, size, &index, compare_deref<string_hash, event_type *>)) {
            result = event_types.at(index);
        }

        return result;

    } else {
        return (event_type *)CDECL_CALL(0x004D1F40, a1);
    }
}

string_hash event_manager::register_script_event_type(const char *a2, const char **a3)
{
    auto *v3 = strrchr(a2, ':');
    auto *v4 = (v3 != nullptr ? v3 + 1 : a2);

    string_hash v7{v4};
    if (a3 != nullptr) {
        *a3 = v4;
    }

    return v7;
}

event_type *event_manager::register_event_type(string_hash a1, bool a2)
{
    TRACE("event_manager::register_event_type");

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
            event_type *the_type = new event_type{a1, a2};

            assert(the_type != nullptr && "Need to increase the fixed pool on events (increase MAX_EVENT_TYPES)!!!");

#if STANDALONE_SYSTEM
            event_types.push_back(the_type);
            std::sort(event_types.begin(), event_types.end(), [](const event_type *lhs, const event_type *rhs) {
                return lhs->field_0.source_hash_code < rhs->field_0.source_hash_code;
            });
#else
            void(__fastcall * push_back)(void *, void *, void *) = CAST(push_back, 0x005E7330);
            int (*compare)(const void *, const void *) = CAST(compare, 0x005034D0);

            event_types.push_back(the_type);
            if (event_types.size() > 1) {
                qsort(event_types.m_first, event_types.size(), 4u, compare);
            }
#endif
        }

        return v2;
    } else {
        return (event_type *)CDECL_CALL(0x004E19F0, a1, a2);
    }
}

void event_manager::clear_script_callbacks(entity_base_vhandle a1, script_executable *a2)
{
    if constexpr (0) {
        for (auto &v2 : event_types) {
            v2->clear_script_callbacks(a1, a2);
        }
    } else {
        CDECL_CALL(0x004D4380, a1, a2);
    }
}

event_recipient_entry *event_manager::create_event_recipient(string_hash arg0, entity_base_vhandle a2)
{
    auto *v2 = event_manager::register_event_type(arg0, 0);
    if (v2 != nullptr) {
        return v2->create_recipient_entry(a2);
    } else {
        return nullptr;
    }
}

int event_manager::add_callback(string_hash a1, entity_base_vhandle a2,
                                void (*cb)(event *, entity_base_vhandle, void *), void *a4, bool a5)
{
    auto *entry = create_event_recipient(a1, a2);
    if (entry != nullptr) {
        return entry->add_callback(cb, a4, a5);
    } else {
        assert(0 && "this might be indicative of an error-condition.");
        return 0;
    }
}

void event_manager::remove_callback(unsigned int a1, string_hash a2, entity_base_vhandle a3)
{
    auto *v4 = get_event_type(a2);
    if (v4 != nullptr) {
        v4->remove_default_callback(a1);
        auto *v5 = v4->find_recipient_entry(a3);
        if (v5 != nullptr) {
            v5->remove_callback(a1);
        }
    }
}


void event_manager_patch()
{
    REDIRECT(0x005E1118, event_manager::create_inst);

    //SET_JUMP(0x004EE9F0, event_manager::raise_event);

    SET_JUMP(0x004E19F0, event_manager::register_event_type);

    REDIRECT(0x005D707A, event_manager::garbage_collect);
}
