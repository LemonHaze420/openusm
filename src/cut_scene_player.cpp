#include "cut_scene_player.h"

#include "ambient_audio_manager.h"
#include "app.h"
#include "comic_panels.h"
#include "cut_scene.h"
#include "cut_scene_segment.h"
#include "entity.h"
#include "entity_class_entry.h"
#include "event.h"
#include "event_manager.h"
#include "fe_health_widget.h"
#include "fe_mini_map_widget.h"
#include "femanager.h"
#include "game.h"
#include "glass_house_manager.h"
#include "igofrontend.h"
#include "igozoomoutmap.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "spiderman_camera.h"
#include "traffic.h"
#include "wds.h"
#include "common.h"
#include "func_wrapper.h"
#include "input_mgr.h"
#include "nal_system.h"
#include "resource_pack_standalone.h"
#include "stream_scene_anim.h"
#include "trace.h"
#include "utility.h"

#include <cstdlib>
#include <algorithm>
#include <cmath>

VALIDATE_SIZE(cut_scene_player, 360u);
VALIDATE_OFFSET(cut_scene_player, panels, 0x8);
VALIDATE_OFFSET(cut_scene_player, animated_entities, 0x18);
VALIDATE_OFFSET(cut_scene_player, streams, 0x48);
VALIDATE_OFFSET(cut_scene_player, hidden_regions, 0x58);
VALIDATE_OFFSET(cut_scene_player, tracked_entities, 0x68);
VALIDATE_OFFSET(cut_scene_player, sound_inst, 0xB8);
VALIDATE_OFFSET(cut_scene_player, owned_camera, 0xCC);
VALIDATE_OFFSET(cut_scene_player, minimap_was_shown, 0xD4);
VALIDATE_OFFSET(cut_scene_player, peds_and_traffic_overridden, 0xD7);
VALIDATE_OFFSET(cut_scene_player, field_E4, 0xE4);
VALIDATE_SIZE(cut_scene_panel_state, 0xC);
VALIDATE_SIZE(cut_scene_panel_node, 0x14);
VALIDATE_SIZE(cut_scene_panel_tree, 0xC);

namespace {
Var<bool> fade_legos{0x009391EC};
Var<bool> saved_physics_enabled{0x009391ED};
Var<bool> saved_glass_house_enabled{0x009391EE};
Var<bool> cut_scene_camera_active{0x0096F7C0};
Var<bool> cut_scene_camera_transition{0x0096F7C1};
Var<float> frame_time_limit{0x00936414};

#if STANDALONE_SYSTEM
[[maybe_unused]] const bool retail_globals_initialized = [] {
    fade_legos() = true;
    saved_physics_enabled() = true;
    saved_glass_house_enabled() = true;
    frame_time_limit() = 0.06673340499401093f;
    return true;
}();
#endif

po restored_pose(const entity_class_entry &entry)
{
    po result{identity_matrix};
    constexpr float radians_per_degree = 0.017453292f;
    const vector3d angles = entry.field_34 * radians_per_degree;
    for (int axis = 0; axis != 3; ++axis) {
        if (std::fabs(angles[axis]) <= EPSILON)
            continue;
        po rotation{identity_matrix};
        if (axis == 0)
            rotation.set_rotate_x(Float{angles.x});
        else if (axis == 1)
            rotation.set_rotate_y(Float{angles.y});
        else
            rotation.set_rotate_z(Float{angles.z});
        ptr_to_po product{&result.m, &rotation.m};
        result.set_from_ptr_to_po_world(product);
    }
    result.set_position(entry.field_28);
    return result;
}
}  // namespace

cut_scene_player::cut_scene_player()
    : sound_inst(0), peds_and_traffic_overridden(false), field_E1(false), field_E4(static_cast<game_control_t>(96)),
      field_118(static_cast<game_control_t>(114)), field_14C(-1), field_154(-1.0f), field_158(-1.0f), field_164(0.0f)
{}

cut_scene_player::~cut_scene_player() = default;

cut_scene_panel_tree::~cut_scene_panel_tree()
{
    if (root) {
        if (owns_panels)
            THISCALL(0x00746480, this, &root, 1);
        else
            THISCALL(0x00747030, this);
    }
}

void cut_scene_player::restore_game_play_panel()
{
    auto *panel = comic_panels::game_play_panel();
    if (panel == nullptr)
        return;
    panel->field_67 = false;
    panel->field_4[3][0] = 320.0f;
    panel->field_4[3][1] = 240.0f;
    panel->field_4[3][2] = 0.0f;
    panel->m_size = vector2d{640.0f, 480.0f};
    for (auto *component = panel->field_60; component; component = component->field_4) {
        if (component->m_vtbl == 0x008AA32C)
            reinterpret_cast<comic_panels::panel_component_camera *>(component)->field_2C = 0;
        else if (component->m_vtbl == 0x008A9DF0)
            component->field_8 = 1.0f;
    }
    panel->field_50 = 1.0f;
}


void cut_scene_player::advance_lip_syncing(Float a2)
{
    THISCALL(0x00737F60, this, a2);
}

bool cut_scene_player::advance_panel_anims(Float a2)
{
    return (bool)THISCALL(0x007382E0, this, a2);
}

void cut_scene_player::clean_up_finished_segment()
{
    THISCALL(0x0073FFB0, this);
}


void cut_scene_player::play(cut_scene *a2)
{
    THISCALL(0x00742190, this, a2);
}

void cut_scene_player::play_current_segment()
{
    THISCALL(0x007414E0, this);
}

void cut_scene_player::frame_advance([[maybe_unused]] Float a2)
{
    TRACE("cut_scene_player::frame_advance");

#if STANDALONE_SYSTEM
    if (!is_playing()) {
        return;
    }
#else
    THISCALL(0x00741EC0, this, a2);
#endif
}

bool cut_scene_player::frame_advance_lite(Float a2)
{
    if constexpr (0) {
    } else {
        bool(__fastcall * func)(void *, void *, Float) = CAST(func, 0x00741220);
        return func(this, nullptr, a2);
    }
}

bool cut_scene_player::is_playing()
{
    return this->field_E1 || this->field_E2;
}

cut_scene_player *g_cut_scene_player()
{
#if STANDALONE_SYSTEM
    static cut_scene_player player;
    return &player;
#else
    static Var<cut_scene_player> player{0x0096FEB8};
    return &player();
#endif
}

void cut_scene_player::stop(cut_scene *a2)
{
    fade_legos() = true;
    cut_scene_camera_active() = false;
    glass_house_manager::enabled = saved_glass_house_enabled();
    cut_scene_camera_transition() = false;
    if (a2 != nullptr && a2 != current_cut_scene)
        return;

    if (field_E1) {
        if (*current_segment && current_segment != current_cut_scene->segments.end())
            clean_up_finished_segment();
        auto *node = current_cut_scene->field_0.root();
        while (node && node->m_left)
            node = node->m_left;
        for (mAvlTree<entity_class_entry>::iterator it{node}; it.field_0; ++it) {
            auto *entry = (*it).m_key;
            auto *ent = g_world_ptr->ent_mgr.get_entity(entry->field_0);
            if (ent && entry->field_25)
                entity_set_abs_po(ent, restored_pose(*entry));
        }
        if (current_cut_scene->sync_camera.m_hash != string_hash{}) {
            auto *ent = g_world_ptr->ent_mgr.get_entity(current_cut_scene->sync_camera.m_hash);
            if (ent && ent->is_a_camera())
                g_spiderman_camera_ptr()->sync(*static_cast<camera *>(ent));
        }
        if (peds_and_traffic_overridden) {
            os_developer_options::instance->set_flag(static_cast<os_developer_options::flags_t>(136), m_peds_enabled);
            traffic::enable_traffic(m_traffic_enabled, false);
            peds_and_traffic_overridden = false;
        }
        if (current_cut_scene->recording)
            app::instance->field_4.end_screen_recording();
        if (auto *sound = sound_inst.get_sound_instance_ptr()) {
            sound->stop();
            ambient_audio_manager::set_max_playing_tracks(2);
        }
    }
    restore_game_play_panel();
    field_E1 = false;
    if (current_cut_scene) {
        if (current_cut_scene->physics_overridden) {
            g_game_ptr->enable_physics(saved_physics_enabled());
            saved_physics_enabled() = true;
        }
        if (current_cut_scene->ui_hidden) {
            if (hero_health_was_shown)
                g_femanager.IGO->m_hero_health->SetShown(hero_health_was_shown);
            if (minimap_was_shown)
                g_femanager.IGO->m_fe_mini_map_widget->SetShown(minimap_was_shown);
            if (boss_health_was_shown)
                g_femanager.IGO->m_boss_health->SetShown(boss_health_was_shown);
            g_femanager.IGO->m_igo_zoom_out_map->field_5C6 = false;
            g_game_ptr->zoomInactive = false;
        }
        if (current_cut_scene->tokens_hidden) {
            event_manager::raise_event(event::SHOW_FINGERS_OF_GOD, entity_base_vhandle{0});
            g_world_ptr->field_188.mark_invisible_by_id(true);
        }
    }
    if (owned_camera) {
        g_world_ptr->ent_mgr.release_entity(owned_camera);
        owned_camera = nullptr;
    }
    current_camera = nullptr;
    current_cut_scene = nullptr;
    frame_time_limit() = std::min(frame_time_limit(), 1.0f / 15.0f);
}

void cut_scene_player_patch()
{
    {
        FUNC_ADDRESS(address, &cut_scene_player::play_current_segment);
        REDIRECT(0x00742419, address);
    }

    {
        FUNC_ADDRESS(address, &cut_scene_player::frame_advance);
        REDIRECT(0x0054FC7A, address);
        REDIRECT(0x0054FC8D, address);
    }

    //cut_scene_player::play_current_segment
    {
        {
            FUNC_ADDRESS(address, &resource_pack_standalone::get_nfl_file_handle);
            REDIRECT(0x007416A8, address);
        }

        REDIRECT(0x007416AE, create_stream_instance);
    }
}
