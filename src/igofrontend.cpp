#include "igofrontend.h"

#include "common.h"
#include "combo_words.h"
#include "entity_tracker_manager.h"
#include "fe_crosshair.h"
#include "fe_distance_chase.h"
#include "fe_distance_race.h"
#include "fe_game_credits.h"
#include "fe_health_widget.h"
#include "fe_hotpursuit_indicator.h"
#include "fe_mini_map_widget.h"
#include "fe_mission_text.h"
#include "fe_score_widget.h"
#include "fe_timer_widget.h"
#include "fe_track_and_field.h"
#include "func_wrapper.h"
#include "igozoomoutmap.h"
#include "memory.h"
#include "medal_award_ui.h"
#include "race_announcer.h"
#include "targeting_reticle.h"
#include "threat_assessment_meters.h"
#include "thug_health.h"
#include "trace.h"
#include "tutorial_controller_gauge.h"
#include "utility.h"

VALIDATE_SIZE(IGOFrontEnd, 0x58);
VALIDATE_OFFSET(IGOFrontEnd, field_44, 0x44);

IGOFrontEnd::IGOFrontEnd()
{
    if constexpr (STANDALONE_SYSTEM)
    {
        this->field_0 = new (mem_alloc(sizeof(fe_timer_widget))) fe_timer_widget {};
        this->field_4 = new (mem_alloc(sizeof(fe_mini_map_widget))) fe_mini_map_widget {};
        this->boss_health = new (mem_alloc(sizeof(fe_health_widget))) fe_health_widget {12};
        this->hero_health = new (mem_alloc(sizeof(fe_health_widget))) fe_health_widget {6};
        this->third_party_health = new (mem_alloc(sizeof(fe_health_widget))) fe_health_widget {3};
        this->field_18 = new (mem_alloc(sizeof(fe_distance_chase))) fe_distance_chase {};
        this->field_1C = new (mem_alloc(sizeof(fe_distance_race))) fe_distance_race {};
        this->field_20 = new (mem_alloc(sizeof(fe_mission_text))) fe_mission_text {};
        this->field_14 = new (mem_alloc(sizeof(fe_track_and_field))) fe_track_and_field {};
        this->field_24 = new (mem_alloc(sizeof(threat_assessment_meters))) threat_assessment_meters {};
        this->field_28 = new (mem_alloc(sizeof(thug_health))) thug_health {};
        this->field_2C = new (mem_alloc(sizeof(targeting_reticle))) targeting_reticle {};
        this->field_30 =
            new (mem_alloc(sizeof(tutorial_controller_gauge))) tutorial_controller_gauge {};
        this->field_34 = new (mem_alloc(sizeof(medal_award_ui))) medal_award_ui {};
        this->field_38 = new (mem_alloc(sizeof(race_announcer))) race_announcer {};
        this->field_3C = new (mem_alloc(sizeof(fe_crosshair))) fe_crosshair {};
        this->field_40 = new (mem_alloc(sizeof(fe_game_credits))) fe_game_credits {};
        this->field_44 = new (mem_alloc(sizeof(IGOZoomOutMap))) IGOZoomOutMap {};
        this->field_48 = new (mem_alloc(sizeof(combo_words))) combo_words {};
        this->field_4C =
            new (mem_alloc(sizeof(fe_hotpursuit_indicator))) fe_hotpursuit_indicator {};
        this->field_50 = new (mem_alloc(sizeof(fe_score_widget))) fe_score_widget {};
        this->field_54 =
            new (mem_alloc(sizeof(entity_tracker_manager))) entity_tracker_manager {};
    }
    else
    {
        THISCALL(0x00648B40, this);
    }
}

void IGOFrontEnd::UpdateInScene()
{
    if (field_54 != nullptr) {
        field_54->place_poi_reticles();
    }
    field_44->UpdateInScene();
}

void IGOFrontEnd::Draw()
{
    THISCALL(0x006358F0, this);
}

void IGOFrontEnd::Init()
{
    TRACE("IGOFrontEnd::Init");

    if constexpr (STANDALONE_SYSTEM) {
        if (field_4 != nullptr) {
            field_4->Init();
        }
        if (field_0 != nullptr) {
            field_0->Init();
        }
        if (field_14 != nullptr) {
            field_14->_Init();
        }
        if (field_20 != nullptr) {
            field_20->Init();
        }
        if (field_24 != nullptr) {
            field_24->init();
        }
        if (field_28 != nullptr) {
            field_28->init();
        }
        if (field_2C != nullptr) {
            field_2C->init();
        }
    } else {
        THISCALL(0x00647DE0, this);
    }
}

void IGOFrontEnd::Update(Float a2) {
    THISCALL(0x00641600, this, a2);
}

void IGOFrontEnd::CheckPauseUnpause() {
    THISCALL(0x00629F80, this);
}

void IGOFrontEnd_patch()
{
    {
        FUNC_ADDRESS(address, &IGOFrontEnd::Init);
        REDIRECT(0x00649085, address);
    }

    {
        FUNC_ADDRESS(address, &IGOFrontEnd::Draw);
        REDIRECT(0x00640F01, address);
    }
}
