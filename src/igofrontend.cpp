#include "igofrontend.h"

#include "common.h"
#include "combo_words.h"
#include "entity_tracker_manager.h"
#include "fe_controller_disconnect.h"
#include "fe_crosshair.h"
#include "fe_distance_chase.h"
#include "fe_distance_race.h"
#include "fe_game_credits.h"
#include "fe_health_widget.h"
#include "femanager.h"
#include "fe_hotpursuit_indicator.h"
#include "fe_mini_map_widget.h"
#include "fe_mission_text.h"
#include "fe_score_widget.h"
#include "fe_timer_widget.h"
#include "fe_track_and_field.h"
#include "func_wrapper.h"
#include "game.h"
#include "igozoomoutmap.h"
#include "memory.h"
#include "input_mgr.h"
#include "medal_award_ui.h"
#include "panelfile.h"
#include "panelanimfile.h"
#include "pausemenusystem.h"
#include "race_announcer.h"
#include "targeting_reticle.h"
#include "threat_assessment_meters.h"
#include "thug_health.h"
#include "trace.h"
#include "tutorial_controller_gauge.h"
#include "utility.h"

VALIDATE_SIZE(IGOFrontEnd, 0x58);
VALIDATE_OFFSET(IGOFrontEnd, m_igo_zoom_out_map, 0x44);

IGOFrontEnd::IGOFrontEnd()
{
    if constexpr (STANDALONE_SYSTEM) {
        this->m_fe_timer_widget = new (mem_alloc(sizeof(fe_timer_widget))) fe_timer_widget{};
        this->m_fe_mini_map_widget = new (mem_alloc(sizeof(fe_mini_map_widget))) fe_mini_map_widget{};
        this->m_boss_health = new (mem_alloc(sizeof(fe_health_widget))) fe_health_widget{12};
        this->m_hero_health = new (mem_alloc(sizeof(fe_health_widget))) fe_health_widget{6};
        this->m_third_party_health = new (mem_alloc(sizeof(fe_health_widget))) fe_health_widget{3};
        this->m_fe_distance_chase = new (mem_alloc(sizeof(fe_distance_chase))) fe_distance_chase{};
        this->m_fe_distance_race = new (mem_alloc(sizeof(fe_distance_race))) fe_distance_race{};
        this->m_fe_mission_text = new (mem_alloc(sizeof(fe_mission_text))) fe_mission_text{};
        this->m_fe_track_and_field = new (mem_alloc(sizeof(fe_track_and_field))) fe_track_and_field{};
        this->m_threat_assessment_meters = new (mem_alloc(sizeof(threat_assessment_meters))) threat_assessment_meters{};
        this->m_thug_health = new (mem_alloc(sizeof(thug_health))) thug_health{};
        this->m_targeting_reticle = new (mem_alloc(sizeof(targeting_reticle))) targeting_reticle{};
        this->m_tutorial_controller_gauge =
            new (mem_alloc(sizeof(tutorial_controller_gauge))) tutorial_controller_gauge{};
        this->m_medal_award_ui = new (mem_alloc(sizeof(medal_award_ui))) medal_award_ui{};
        this->m_race_announcer = new (mem_alloc(sizeof(race_announcer))) race_announcer{};
        this->m_fe_crosshair = new (mem_alloc(sizeof(fe_crosshair))) fe_crosshair{};
        this->m_fe_game_credits = new (mem_alloc(sizeof(fe_game_credits))) fe_game_credits{};
        this->m_igo_zoom_out_map = new (mem_alloc(sizeof(IGOZoomOutMap))) IGOZoomOutMap{};
        this->m_combo_words = new (mem_alloc(sizeof(combo_words))) combo_words{};
        this->m_fe_hotpursuit_indicator = new (mem_alloc(sizeof(fe_hotpursuit_indicator))) fe_hotpursuit_indicator{};
        this->m_fe_score_widget = new (mem_alloc(sizeof(fe_score_widget))) fe_score_widget{};
        this->m_entity_tracker_manager = new (mem_alloc(sizeof(entity_tracker_manager))) entity_tracker_manager{};
    } else {
        THISCALL(0x00648B40, this);
    }
}


IGOFrontEnd::~IGOFrontEnd()
{
    delete m_fe_timer_widget;
    delete m_fe_mini_map_widget;
    delete m_boss_health;
    delete m_hero_health;
    delete m_third_party_health;
    delete m_fe_track_and_field;
    delete m_fe_distance_chase;
    delete m_fe_distance_race;
    delete m_fe_mission_text;
    delete m_threat_assessment_meters;
    delete m_thug_health;
    delete m_targeting_reticle;
    delete m_tutorial_controller_gauge;
    delete m_medal_award_ui;
    delete m_race_announcer;
    delete m_fe_crosshair;
    delete m_fe_hotpursuit_indicator;
    delete m_fe_score_widget;
    delete m_fe_game_credits;
    delete m_igo_zoom_out_map;
    delete m_combo_words;
    delete m_entity_tracker_manager;
}

void IGOFrontEnd::UpdateInScene()
{
    if (m_entity_tracker_manager != nullptr) {
        m_entity_tracker_manager->place_poi_reticles();
    }
    m_igo_zoom_out_map->UpdateInScene();
}

void IGOFrontEnd::Draw()
{
    THISCALL(0x006358F0, this);
}

void IGOFrontEnd::Init()
{
    TRACE("IGOFrontEnd::Init");

    if constexpr (STANDALONE_SYSTEM) {
        if (m_fe_mini_map_widget != nullptr) {
            m_fe_mini_map_widget->Init();
            m_fe_mini_map_widget->SetShown(true);
        }
        if (m_fe_timer_widget != nullptr) {
            m_fe_timer_widget->Init();
        }
        if (m_fe_track_and_field != nullptr) {
            m_fe_track_and_field->_Init();
        }
        if (m_fe_mission_text != nullptr) {
            m_fe_mission_text->Init();
        }
        if (m_threat_assessment_meters != nullptr) {
            m_threat_assessment_meters->init();
        }
        if (m_thug_health != nullptr) {
            m_thug_health->init();
        }
        if (m_targeting_reticle != nullptr) {
            m_targeting_reticle->init();
        }
    } else {
        THISCALL(0x00647DE0, this);
    }
}

void IGOFrontEnd::Update(Float a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        CheckPauseUnpause();

        if (m_fe_timer_widget != nullptr)
            m_fe_timer_widget->Update(a2);

        const auto update_health = [a2](fe_health_widget *health) {
            if (health == nullptr || health->field_38 < 0 || health->field_38 >= health->number_of_types)
                return;
            PanelFile *panel = health->panels[health->field_38];
            if (panel == nullptr || panel->field_28.empty())
                return;
            if (health->field_54 || panel->field_28.at(0)->field_2D) {
                panel->Update(a2);
                health->UpdateMasking();
            }
        };
        update_health(m_boss_health);
        update_health(m_hero_health);
        update_health(m_third_party_health);

        if (m_fe_distance_chase != nullptr)
            m_fe_distance_chase->Update(a2);
        if (m_fe_distance_race != nullptr)
            m_fe_distance_race->Update(a2);
        if (m_fe_mission_text != nullptr && m_fe_mission_text->panel != nullptr &&
            (m_fe_mission_text->shown || (m_fe_mission_text->anim != nullptr && m_fe_mission_text->anim->field_2D)))
            m_fe_mission_text->panel->Update(a2);
        if (m_fe_track_and_field != nullptr)
            m_fe_track_and_field->Update(a2);
        if (m_fe_mini_map_widget != nullptr)
            m_fe_mini_map_widget->Update(a2);
        if (m_igo_zoom_out_map != nullptr)
            m_igo_zoom_out_map->Update(a2);
    } else {
        THISCALL(0x00641600, this, a2);
    }
}

void IGOFrontEnd::CheckPauseUnpause()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (!fe_controller_disconnect::get_currently_plugged_in() || input_mgr::instance == nullptr ||
            input_mgr::instance->get_control_delta(54, input_mgr::instance->field_58) < AXIS_MAX ||
            m_igo_zoom_out_map == nullptr || m_igo_zoom_out_map->field_5C4 || m_igo_zoom_out_map->field_5C3)
            return;

        PauseMenuSystem *pause_menu = g_femanager.m_pause_menu_system;
        if (g_game_ptr->is_paused()) {
            if (pause_menu != nullptr && pause_menu->m_index >= 0) {
                g_game_ptr->unpause();
                pause_menu->Deactivate();
            }
            return;
        }
        if (g_game_ptr->field_165 || g_game_ptr->field_166 ||
            (m_fe_game_credits != nullptr && m_fe_game_credits->field_0))
            return;

        g_game_ptr->pause();
        if (pause_menu != nullptr) {
            pause_menu->SetTransition(0);
            pause_menu->Activate(1, false);
        }
    } else {
        THISCALL(0x00629F80, this);
    }
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
