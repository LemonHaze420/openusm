#pragma once

#include <cstdint>

#include "float.hpp"

struct fe_health_widget;
struct entity_tracker_manager;
struct IGOZoomOutMap;
struct fe_mini_map_widget;
struct fe_track_and_field;
struct fe_distance_chase;
struct fe_distance_race;
struct fe_mission_text;
struct combo_words;
struct fe_hotpursuit_indicator;
struct fe_score_widget;
struct threat_assessment_meters;
struct thug_health;
struct targeting_reticle;
struct tutorial_controller_gauge;
struct medal_award_ui;
struct race_announcer;
struct fe_crosshair;
struct fe_game_credits;
struct fe_timer_widget;

struct IGOFrontEnd {
    fe_timer_widget *m_fe_timer_widget;
    fe_mini_map_widget *m_fe_mini_map_widget;
    fe_health_widget *m_boss_health;
    fe_health_widget *m_hero_health;
    fe_health_widget *m_third_party_health;
    fe_track_and_field *m_fe_track_and_field;
    fe_distance_chase *m_fe_distance_chase;
    fe_distance_race *m_fe_distance_race;
    fe_mission_text *m_fe_mission_text;
    threat_assessment_meters *m_threat_assessment_meters;
    thug_health *m_thug_health;
    targeting_reticle *m_targeting_reticle;
    tutorial_controller_gauge *m_tutorial_controller_gauge;
    medal_award_ui *m_medal_award_ui;
    race_announcer *m_race_announcer;
    fe_crosshair *m_fe_crosshair;
    fe_game_credits *m_fe_game_credits;
    IGOZoomOutMap *m_igo_zoom_out_map;
    combo_words *m_combo_words;
    fe_hotpursuit_indicator *m_fe_hotpursuit_indicator;
    fe_score_widget *m_fe_score_widget;
    entity_tracker_manager *m_entity_tracker_manager;

    //0x00648B40


    IGOFrontEnd();
    ~IGOFrontEnd();

    void UpdateInScene();

    //0x00647DE0
    void Init();

    //0x006358F0
    void Draw();

    //0x00629F80
    void CheckPauseUnpause();

    //0x00641600
    void Update(Float a2);
};

extern void IGOFrontEnd_patch();
