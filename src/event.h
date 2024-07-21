#pragma once

#include "string_hash.h"

#include <cstdint>

struct event
{
    std::intptr_t m_vtbl;
    string_hash field_4;
    int field_8;

    //0x0048ABA0
    event(string_hash a2);

    void _finalize(bool a2);

#define create_string_hash(name)           \
    static inline const string_hash name { \
        static_cast<int>(to_hash(#name))   \
    }

    create_string_hash(ANIM_ACTION);

    create_string_hash(ATTACK_BEGIN);

    create_string_hash(ATTACK_END);

    create_string_hash(CMBT_CHAIN);

    create_string_hash(ATTACK);

    create_string_hash(SWAP_PROP_A);

    create_string_hash(SWAP_PROP_B);

    create_string_hash(FOOTSTEP_L);

    create_string_hash(FOOTSTEP_R);

    create_string_hash(TERRAIN_FX);

    create_string_hash(CAPTURE_FRAME);

    create_string_hash(HIDE);

    create_string_hash(SOUND_GROUP);

    create_string_hash(ATTACK_WHOOSH_SOUND);

    create_string_hash(ATTACK_IMPACT_SOUND);

    create_string_hash(STORED_ATTACK_IMPACT_SOUND);

    create_string_hash(SAY_FILE);

    create_string_hash(SAY_SOUND_GROUP);

    create_string_hash(SAY_GAB);

    create_string_hash(FX_SMALL);

    create_string_hash(FX_BIG);

    create_string_hash(WEB_START);

    create_string_hash(WEB_END);

    create_string_hash(MO_START);

    create_string_hash(MO_END);

    create_string_hash(INTERACTION_FINISHED);

    create_string_hash(GRAB_START);

    create_string_hash(GRAB_END);

    create_string_hash(THROW_START);

    create_string_hash(THROW_END);

    create_string_hash(WEAPON_SHOW);

    create_string_hash(WEAPON_HIDE);

    create_string_hash(ACTIVATED_BY_CHARACTER);

    create_string_hash(DAMAGED);

    create_string_hash(DESTROYED);

    create_string_hash(FED_UPON);

    create_string_hash(USE_ITEM);

    create_string_hash(RUMBLE_LOW);

    create_string_hash(RUMBLE_MEDIUM);

    create_string_hash(RUMBLE_HIGH);

    create_string_hash(RUMBLE_STOP);

    create_string_hash(GENERIC_SIGNAL_5);

    create_string_hash(AI_STATE_IDLE);

    create_string_hash(AI_STATE_ALERTED);

    create_string_hash(AI_STATE_COMBAT);

    create_string_hash(AI_STATE_INCREMENT);

    create_string_hash(AI_STATE_DECREMENT);

    create_string_hash(FADED_OUT);

    create_string_hash(SCENE_ANIM_FINISHED);

    create_string_hash(SCENE_ANIM_ENDING);

    create_string_hash(ANIM_DEST_REACHED);

    create_string_hash(META_ANIM_UTILITY);

    create_string_hash(BOUNCED);

    create_string_hash(AI_NEW_THREAT);

    create_string_hash(AI_LOST_THREAT);

    create_string_hash(BEGIN_PICK_UP);

    create_string_hash(PICKED_UP);

    create_string_hash(ENTER_LIMBO);

    create_string_hash(EXIT_LIMBO);

    create_string_hash(HIDE_FINGERS_OF_GOD);

    create_string_hash(SHOW_FINGERS_OF_GOD);

    create_string_hash(CREDITS_FINISHED);

    create_string_hash(BEGIN_PUT_DOWN);

    create_string_hash(PUT_DOWN);

    create_string_hash(BEING_WEBBED);

    create_string_hash(HEAL_START);

    create_string_hash(HEAL_DONE);

    create_string_hash(RESPAWNED_THIS_FRAME);

    create_string_hash(FINISHED_TRAFFIC_GOTO);

    create_string_hash(TRAFFIC_CAR_IS_COMBATIVE);

    create_string_hash(TRAFFIC_CAR_ENTER_COMBAT);

    create_string_hash(TRAFFIC_CAR_EXIT_COMBAT);

    create_string_hash(TRAFFIC_CAR_KILLED);

    create_string_hash(TGT_ARRESTED);

    create_string_hash(TGT_FOLLOW_CAUGHT);

    create_string_hash(TGT_DRAINED);

    create_string_hash(IN_LIGHT_POLE);

    create_string_hash(OUT_LIGHT_POLE);

    create_string_hash(AI_CROWD_RESPONSE_CHEER);

    create_string_hash(AI_CROWD_RESPONSE_FLEE);

    create_string_hash(AI_GET_IN_CAR_FINISHED);

    create_string_hash(AI_GET_OUT_OF_CAR_FINISHED);

    create_string_hash(CAR_COMBAT_ENGAGED);

    create_string_hash(CAR_COMBAT_DISENGAGED);

    create_string_hash(CAR_COMBAT_NODE_CHANGED);

    create_string_hash(CAR_COMBAT_LEFT_ATTACK);

    create_string_hash(CAR_COMBAT_RIGHT_ATTACK);

    create_string_hash(CAR_COMBAT_DODGED);

    create_string_hash(DELETED);

    create_string_hash(SUBDUED);

    create_string_hash(ACTIVATE_SPIDEY_SENSE);

    create_string_hash(PROP_PHYSICS_START);

    create_string_hash(PROP_PHYSICS_STOP);

    create_string_hash(COLLIDED_WITH_SWING_WEB);

    create_string_hash(TRICK_WALL_JUMP);

    create_string_hash(TRICK_POLE_SWING_REVOLUTION);

    create_string_hash(AI_PUPPET_1);

    create_string_hash(AI_PUPPET_2);

    create_string_hash(AI_PUPPET_3);

    create_string_hash(AI_PUPPET_4);

    create_string_hash(AI_PUPPET_5);

    create_string_hash(KILL_SPAWNED_LAYER);

    create_string_hash(FORCE_INTERACTION_SUCCESS);

    create_string_hash(FORCE_INTERACTION_FAILURE);

    create_string_hash(INTERACTION_PRE_ANIMS_PHASE);

    create_string_hash(INTERACTION_PRE_INTERACT_PHASE);

    create_string_hash(INTERACTION_INTERACT_PHASE);

    create_string_hash(INTERACTION_POST_INTERACT_PHASE);

    create_string_hash(INTERACTION_POST_FAIL_INTERACT_PHASE);

    create_string_hash(INTERACTION_SUCCESS);

    create_string_hash(INTERACTION_FAILURE);

    create_string_hash(ATTACH_PRE_ANIMS_PHASE);

    create_string_hash(ATTACH_PRE_INTERACT_PHASE);

    create_string_hash(ATTACH_INTERACT_PHASE);

    create_string_hash(ATTACH_POST_INTERACT_PHASE);

    create_string_hash(ENTER);

    create_string_hash(LEAVE);

    create_string_hash(PICKUP);

    create_string_hash(USE);

    create_string_hash(SCHWING);

    create_string_hash(DETONATE);

    create_string_hash(ARMED);

    create_string_hash(EXPLODE);

    create_string_hash(SWITCH_TOGGLE);

    create_string_hash(SWITCH_ON);

    create_string_hash(SWITCH_OFF);

    create_string_hash(MUSIC_TRACK_SWITCH);

    create_string_hash(MUSIC_FINISHED);

    create_string_hash(TIME_MINUTE_INC);

    create_string_hash(TIME_HOUR_INC);

    create_string_hash(TIME_DAY_INC);

    create_string_hash(X_PRESSED);

    create_string_hash(X_RELEASED);

    create_string_hash(SQUARE_PRESSED);

    create_string_hash(SQUARE_RELEASED);

    create_string_hash(TRIANGLE_PRESSED);

    create_string_hash(TRIANGLE_RELEASED);

    create_string_hash(CIRCLE_PRESSED);

    create_string_hash(CIRCLE_RELEASED);

    create_string_hash(L1_PRESSED);

    create_string_hash(L1_RELEASED);

    create_string_hash(L2_PRESSED);

    create_string_hash(L2_RELEASED);

    create_string_hash(R1_PRESSED);

    create_string_hash(R1_RELEASED);

    create_string_hash(R2_PRESSED);

    create_string_hash(R2_RELEASED);

    create_string_hash(LEFT_PRESSED);

    create_string_hash(LEFT_RELEASED);

    create_string_hash(RIGHT_PRESSED);

    create_string_hash(RIGHT_RELEASED);

    create_string_hash(UP_PRESSED);

    create_string_hash(UP_RELEASED);

    create_string_hash(DOWN_PRESSED);

    create_string_hash(DOWN_RELEASED);

    create_string_hash(RSTICK_LEFT_PRESSED);

    create_string_hash(RSTICK_LEFT_RELEASED);

    create_string_hash(RSTICK_RIGHT_PRESSED);

    create_string_hash(RSTICK_RIGHT_RELEASED);

    create_string_hash(RSTICK_UP_PRESSED);

    create_string_hash(RSTICK_UP_RELEASED);

    create_string_hash(RSTICK_DOWN_PRESSED);

    create_string_hash(RSTICK_DOWN_RELEASED);

    create_string_hash(LSTICK_LEFT_PRESSED);

    create_string_hash(LSTICK_LEFT_RELEASED);

    create_string_hash(LSTICK_RIGHT_PRESSED);

    create_string_hash(LSTICK_RIGHT_RELEASED);

    create_string_hash(LSTICK_UP_PRESSED);

    create_string_hash(LSTICK_UP_RELEASED);

    create_string_hash(LSTICK_DOWN_PRESSED);

    create_string_hash(LSTICK_DOWN_RELEASED);

    create_string_hash(START_PRESSED);

    create_string_hash(START_RELEASED);

    create_string_hash(SELECT_PRESSED);

    create_string_hash(SELECT_RELEASED);

    create_string_hash(BEETLE_DEVICE_EXPLODE);

    create_string_hash(BEETLE_SHOCKWAVE_ATTACK);

    create_string_hash(BEETLE_SMOKE_GRENADE_ATTACK);

    create_string_hash(BEETLE_PROXIMITY_GRENADE_ATTACK);

    create_string_hash(BEETLE_PROXIMITY_BARRIER_ATTACK);

    create_string_hash(BEETLE_ACTIVATE_LASER);

    create_string_hash(BEETLE_DE_ACTIVATE_LASER);

    create_string_hash(BEETLE_LASER_ATTACK);

    create_string_hash(BEETLE_LASER_VOLLEY_ATTACK);

    create_string_hash(BEETLE_BUMRUSH_ATTACK);

    create_string_hash(BEETLE_LEAP_ATTACK);

    create_string_hash(BEETLE_LASER_DRAG_ATTACK);

    create_string_hash(BEETLE_LASER_DRAG_2PASS_ATTACK);

    create_string_hash(BEETLE_LASER_DRAG_3PASS_ATTACK);

    create_string_hash(RHINO_WINDOW_OPEN);

    create_string_hash(RHINO_WINDOW_CLOSE);

    create_string_hash(SABLE_BLOCKS);

    create_string_hash(SABLE_RETALIATES);

    create_string_hash(HERO_LEAVES_PLAY_AREA);

    create_string_hash(HERO_ENTERS_PLAY_AREA);

    create_string_hash(COLLISION_EVENT);

    create_string_hash(THROW_EVENT);

    create_string_hash(BOUNCED_GROUND);

    create_string_hash(BOUNCED_UNKNOWN);

    create_string_hash(STORED_BOUNCED_TARGET);

    create_string_hash(PARTICLE_EMISSION);

    create_string_hash(VENOM_CANCEL_MODE);

    create_string_hash(PREVIEW_TRANS_JUMP_MSG);

    create_string_hash(PREVIEW_TRANS_WEBZIP_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_1_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_2_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_3_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_4_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_5_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_6_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_7_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_8_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_9_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_10_MSG);

    create_string_hash(PREVIEW_TRANS_MELEE_11_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_1_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_2_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_3_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_4_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_5_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_6_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_7_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_8_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_9_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_10_MSG);

    create_string_hash(PREVIEW_TRANS_RANGED_11_MSG);

    create_string_hash(PREVIEW_TRANS_FLEE_NEAR_MSG);

    create_string_hash(PREVIEW_TRANS_FLEE_NEAR_1_MSG);

    create_string_hash(PREVIEW_TRANS_FLEE_NEAR_2_MSG);

    create_string_hash(PREVIEW_TRANS_FLEE_NEAR_3_MSG);

    create_string_hash(PREVIEW_TRANS_FLEE_FAR_MSG);
};
