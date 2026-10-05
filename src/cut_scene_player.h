#pragma once

#include "entity_base_vhandle.h"
#include "float.hpp"
#include "game_button.h"
#include "mvector.h"
#include "sound_instance_id.h"

#include <vector.hpp>

struct entity_base;
struct entity;
struct cut_scene;
struct nalSceneAnimInstance;
struct cut_scene_segment;
struct region;
struct tracking_panel_anim;
namespace comic_panels {
struct panel;
struct page_camera;
}  // namespace comic_panels

struct cut_scene_panel_state {
    string_hash id;
    comic_panels::panel *panel;
    tracking_panel_anim *animation;
};

struct cut_scene_panel_node {
    cut_scene_panel_node *left;
    cut_scene_panel_node *right;
    cut_scene_panel_node *parent;
    cut_scene_panel_state *state;
    int8_t height;
};

struct cut_scene_panel_tree {
    cut_scene_panel_node *root = nullptr;
    int count = 0;
    bool owns_panels = true;

    ~cut_scene_panel_tree();
};

struct cut_scene_player {
    cut_scene *current_cut_scene;
    mVector<cut_scene_segment>::iterator current_segment;
    cut_scene_panel_tree panels;
    comic_panels::page_camera *page_camera;
    _std::vector<entity_base_vhandle> animated_entities;
    _std::vector<entity_base_vhandle> restored_entities;
    _std::vector<entity *> acquired_entities;
    _std::vector<nalSceneAnimInstance *> streams;
    _std::vector<region *> hidden_regions;
    _std::vector<entity_base_vhandle> tracked_entities[5];
    sound_instance_id sound_inst;
    _std::vector<entity_base_vhandle> field_BC;
    entity *owned_camera;
    entity *current_camera;
    bool minimap_was_shown;
    bool hero_health_was_shown;
    bool boss_health_was_shown;
    bool peds_and_traffic_overridden;
    bool m_traffic_enabled;
    bool m_peds_enabled;
    int field_DC;
    bool field_E0;
    bool field_E1;
    bool field_E2;
    game_button field_E4;
    game_button field_118;
    int field_14C;
    int field_150;
    float field_154;
    float field_158;
    float field_15C;
    float field_160;
    float field_164;

    //0x0073F170
    cut_scene_player();
    ~cut_scene_player();

    static void restore_game_play_panel();

    //0x00737F60
    void advance_lip_syncing(Float a2);

    //0x007382E0
    bool advance_panel_anims(Float a2);

    //0x0073FFB0
    void clean_up_finished_segment();

    //0x00742190
    void play(cut_scene *a2);

    //0x007414E0
    void play_current_segment();

    bool frame_advance_lite(Float a2);

    //0x00741EC0
    void frame_advance(Float a2);

    //0x00502B00
    bool is_playing();

    //0x00740660
    void stop(cut_scene *a2);
};

//0x007411C0
extern cut_scene_player *g_cut_scene_player();

extern void cut_scene_player_patch();
