#include "igofrontend.h"

#include "common.h"
#include "combo_words.h"
#include "entity_tracker_manager.h"
#include "fe_mini_map_widget.h"
#include "fe_game_credits.h"
#include "fe_crosshair.h"
#include "race_announcer.h"
#include "fe_mission_text.h"
#include "fe_hotpursuit_indicator.h"
#include "medal_award_ui.h"
#include "thug_health.h"
#include "targeting_reticle.h"
#include "fe_distance_chase.h"
#include "fe_distance_race.h"
#include "fe_health_widget.h"
#include "fe_score_widget.h"
#include "fe_timer_widget.h"
#include "fe_track_and_field.h"
#include "func_wrapper.h"
#include "threat_assessment_meters.h"
#include "tutorial_controller_gauge.h"
#include "igozoomoutmap.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(IGOFrontEnd, 0x58);
VALIDATE_OFFSET(IGOFrontEnd, field_44, 0x44);

IGOFrontEnd::IGOFrontEnd()
{
    if constexpr (0) {
        auto *mem = mem_alloc(sizeof(fe_timer_widget));
        this->field_0 = new (mem) fe_timer_widget {};

        mem = mem_alloc(sizeof(fe_mini_map_widget));
        this->field_4 = new (mem) fe_mini_map_widget {};

        mem = mem_alloc(sizeof(fe_health_widget));
        this->boss_health = new (mem) fe_health_widget {12};

        mem = mem_alloc(sizeof(fe_health_widget));
        this->hero_health = new (mem) fe_health_widget {6};

        mem = mem_alloc(sizeof(fe_health_widget));
        this->third_party_health = new (mem) fe_health_widget {3};

        mem = mem_alloc(sizeof(fe_distance_chase));
        this->field_18 = new (mem) fe_distance_chase {};

        mem = mem_alloc(sizeof(fe_distance_race));
        this->field_1C = new (mem) fe_distance_race {};

        mem = mem_alloc(sizeof(fe_mission_text));
        this->field_20 = new (mem) fe_mission_text {};

        mem = mem_alloc(sizeof(fe_track_and_field));
        this->field_14 = new (mem) fe_track_and_field {};

        mem = mem_alloc(sizeof(threat_assessment_meters));
        this->field_24 = new (mem) threat_assessment_meters {};

        mem = mem_alloc(sizeof(thug_health));
        this->field_28 = new (mem) thug_health {};

        mem = mem_alloc(sizeof(targeting_reticle));
        this->field_2C = new (mem) targeting_reticle {};

        mem = mem_alloc(sizeof(tutorial_controller_gauge));
        this->field_30 = new (mem) tutorial_controller_gauge {};

        mem = mem_alloc(sizeof(medal_award_ui));
        this->field_34 = new (mem) medal_award_ui {};

        mem = mem_alloc(sizeof(race_announcer));
        this->field_38 = new (mem) race_announcer {};

        mem = mem_alloc(sizeof(fe_crosshair));
        this->field_3C = new (mem) fe_crosshair {};

        mem = mem_alloc(sizeof(fe_game_credits));
        this->field_40 = new (mem) fe_game_credits {};

        mem = mem_alloc(sizeof(IGOZoomOutMap));
        this->field_44 = new (mem) IGOZoomOutMap {};

        mem = mem_alloc(sizeof(combo_words));
        this->field_48 = new (mem) combo_words {};

        mem = mem_alloc(sizeof(fe_hotpursuit_indicator));
        this->field_4C = new (mem) fe_hotpursuit_indicator {};

        mem = mem_alloc(sizeof(fe_score_widget));
        this->field_50 = new (mem) fe_score_widget {};

        mem = mem_alloc(sizeof(entity_tracker_manager));
        this->field_54 =  new (mem) entity_tracker_manager {};

    } else {
        THISCALL(0x00648B40, this);
    }
}

void IGOFrontEnd::UpdateInScene()
{
    if ( this->field_54 != nullptr ) {
        this->field_54->place_poi_reticles();
    }

    this->field_44->UpdateInScene();
}

void IGOFrontEnd::Draw()
{
    if constexpr(0)
    {

    }
    else
    {
        THISCALL(0x006358F0, this);
    }
}

void IGOFrontEnd::Init() {
    TRACE("IGOFrontEnd::Init");

    THISCALL(0x00647DE0, this);
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
