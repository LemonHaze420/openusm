#include "debug_menu_extra.h"

#include "actor.h"
#include "app.h"
#include "debug_menu.h"
#include "debug_render.h"
#include "entity_tracker_manager.h"
#include "os_developer_options.h"
#include "spider_monkey.h"
#include "game.h"
#include "game_settings.h"
#include "devopt.h"
#include "rumble_manager.h"
#include "ngl.h"
#include "nal_anim.h"
#include "nal_system.h"
#include "terrain.h"
#include "resource_manager.h"
#include "geometry_manager.h"
#include "region.h"
#include "femanager.h"
#include "igofrontend.h"
#include "wds.h"

#include <list>

mString ngl_render_callback(debug_menu_entry *a2)
{
    auto &v2 = a2->get_name();
    auto *v3 = v2.c_str();
    auto v4 = nglGetDebugFlag(v3);
    a2->set_ival(v4, true);
    auto result = entry_render_callback_default(a2);
    return result;
}

void ngl_handler(debug_menu_entry *a1)
{
    auto v3 = a1->get_ival();
    auto &v1 = a1->get_name();
    auto *v2 = v1.c_str();
    nglSetDebugFlag(v2, v3);
}

void create_ngl_menu(debug_menu *parent)
{
    assert(parent != nullptr);

    auto *v22 = create_menu("NGL Debug", debug_menu::sort_mode_t::undefined);

    auto *v133 = v22;
    auto *v23 = create_menu_entry(v133);

    parent->add_entry(v23);
    if (nglGetDebugFlagPtr("ShowPerfInfo")) {
        auto *v24 = create_menu_entry(mString{"ShowPerfInfo"});

        auto v1 = nglGetDebugFlag("ShowPerfInfo");
        v24->set_ival(v1);
        v24->set_min_value(0.0);
        v24->set_max_value(2.0);
        v24->set_game_flags_handler(ngl_handler);
        v24->set_render_cb(ngl_render_callback);
        v133->add_entry(v24);
    }

    if (nglGetDebugFlagPtr("ScreenShot")) {
        auto *v25 = create_menu_entry(mString{"ScreenShot"});
        auto v2 = nglGetDebugFlag("ScreenShot");
        v25->set_ival(v2);
        v25->set_min_value(0.0);
        v25->set_max_value(1.0);
        v25->set_game_flags_handler(ngl_handler);
        v25->set_render_cb(ngl_render_callback);
        v133->add_entry(v25);
    }

    if (nglGetDebugFlagPtr("DisableQuads")) {
        auto *v26 = create_menu_entry(mString{"DisableQuads"});

        auto v3 = nglGetDebugFlag("DisableQuads");
        v26->set_ival(v3);
        v26->set_min_value(0.0);
        v26->set_max_value(1.0);
        v26->set_game_flags_handler(ngl_handler);
        v26->set_render_cb(ngl_render_callback);
        v133->add_entry(v26);
    }

    if (nglGetDebugFlagPtr("DisableVSync")) {
        auto *v27 = create_menu_entry(mString{"DisableVSync"});
        auto v4 = nglGetDebugFlag("DisableVSync");
        v27->set_ival(v4);
        v27->set_min_value(0.0);
        v27->set_max_value(1.0);
        v27->set_game_flags_handler(ngl_handler);
        v27->set_render_cb(ngl_render_callback);
        v133->add_entry(v27);
    }

    if (nglGetDebugFlagPtr("DisableScratch")) {
        auto *v28 = create_menu_entry(mString{"DisableScratch"});
        auto v5 = nglGetDebugFlag("DisableScratch");
        v28->set_ival(v5);
        v28->set_min_value(0.0);
        v28->set_max_value(1.0);
        v28->set_game_flags_handler(ngl_handler);
        v28->set_render_cb(ngl_render_callback);
        v133->add_entry(v28);
    }

    if (nglGetDebugFlagPtr("DebugPrints")) {
        auto *v29 = create_menu_entry("DebugPrints");

        auto v6 = nglGetDebugFlag("DebugPrints");
        v29->set_ival(v6);
        v29->set_min_value(0.0);
        v29->set_max_value(1.0);
        v29->set_game_flags_handler(ngl_handler);
        v29->set_render_cb(ngl_render_callback);
        v133->add_entry(v29);
    }

    if (nglGetDebugFlagPtr("DumpFrameLog")) {
        auto *v30 = create_menu_entry(mString{"DumpFrameLog"});
        auto v7 = nglGetDebugFlag("DumpFrameLog");
        v30->set_ival(v7);
        v30->set_min_value(0.0);
        v30->set_max_value(1.0);
        v30->set_game_flags_handler(ngl_handler);
        v30->set_render_cb(ngl_render_callback);
        v133->add_entry(v30);
    }

    if (nglGetDebugFlagPtr("DumpSceneFile")) {
        auto *v31 = create_menu_entry("DumpSceneFile");
        auto v8 = nglGetDebugFlag("DumpSceneFile");
        v31->set_ival(v8);
        v31->set_min_value(0.0);
        v31->set_max_value(1.0);
        v31->set_game_flags_handler(ngl_handler);
        v31->set_render_cb(ngl_render_callback);
        v133->add_entry(v31);
    }

    if (nglGetDebugFlagPtr("DumpTextures")) {
        auto *v32 = create_menu_entry("DumpTextures");
        auto v9 = nglGetDebugFlag("DumpTextures");
        v32->set_ival(v9);
        v32->set_min_value(0.0);
        v32->set_max_value(1.0);
        v32->set_game_flags_handler(ngl_handler);
        v32->set_render_cb(ngl_render_callback);
        v133->add_entry(v32);
    }

    if (nglGetDebugFlagPtr("DrawLightSpheres")) {
        auto *v33 = create_menu_entry("DrawLightSpheres");
        auto v10 = nglGetDebugFlag("DrawLightSpheres");
        v33->set_ival(v10);
        v33->set_min_value(0.0);
        v33->set_max_value(1.0);
        v33->set_game_flags_handler(ngl_handler);
        v33->set_render_cb(ngl_render_callback);
        v133->add_entry(v33);
    }

    if (nglGetDebugFlagPtr("DrawMeshSpheres")) {
        auto *v34 = create_menu_entry(mString{"DrawMeshSpheres"});

        auto v11 = nglGetDebugFlag("DrawMeshSpheres");
        v34->set_ival(v11);
        v34->set_min_value(0.0);
        v34->set_max_value(1.0);
        v34->set_game_flags_handler(ngl_handler);
        v34->set_render_cb(ngl_render_callback);
        v133->add_entry(v34);
    }

    if (nglGetDebugFlagPtr("DisableDuplicateMaterialWarning")) {
        auto *v35 = create_menu_entry(mString{"DisableDuplicateMaterialWarning"});
        auto v12 = nglGetDebugFlag("DisableDuplicateMaterialWarning");
        v35->set_ival(v12);
        v35->set_min_value(0.0);
        v35->set_max_value(1.0);
        v35->set_game_flags_handler(ngl_handler);
        v35->set_render_cb(ngl_render_callback);
        v133->add_entry(v35);
    }

    if (nglGetDebugFlagPtr("DisableMissingTextureWarning")) {
        auto *v36 = create_menu_entry(mString{"DisableMissingTextureWarning"});
        auto v13 = nglGetDebugFlag("DisableMissingTextureWarning");
        v36->set_ival(v13);
        v36->set_min_value(0.0);
        v36->set_max_value(1.0);
        v36->set_game_flags_handler(ngl_handler);
        v36->set_render_cb(ngl_render_callback);
        v133->add_entry(v36);
    }

    if (nglGetDebugFlagPtr("DisableMipOpt")) {
        auto *v37 = create_menu_entry(mString{"DisableMipOpt"});

        auto v14 = nglGetDebugFlag("DisableMipOpt");
        v37->set_ival(v14);
        v37->set_min_value(0.0);
        v37->set_max_value(1.0);
        v37->set_game_flags_handler(ngl_handler);
        v37->set_render_cb(ngl_render_callback);
        v133->add_entry(v37);
    }

    if (nglGetDebugFlagPtr("DisableFSAA")) {
        auto *v38 = create_menu_entry(mString{"DisableFSAA"});

        auto v15 = nglGetDebugFlag("DisableFSAA");
        v38->set_ival(v15);
        v38->set_min_value(0.0);
        v38->set_max_value(1.0);
        v38->set_game_flags_handler(ngl_handler);
        v38->set_render_cb(ngl_render_callback);
        v133->add_entry(v38);
    }

    if (nglGetDebugFlagPtr("DrawToFrontBuffer")) {
        auto *v39 = create_menu_entry(mString{"DrawToFrontBuffer"});

        auto v16 = nglGetDebugFlag("DrawToFrontBuffer");
        v39->set_ival(v16);
        v39->set_min_value(0.0);
        v39->set_max_value(1.0);
        v39->set_game_flags_handler(ngl_handler);
        v39->set_render_cb(ngl_render_callback);
        v133->add_entry(v39);
    }

    if (nglGetDebugFlagPtr("SyncRender")) {
        auto *v40 = create_menu_entry(mString{"SyncRender"});

        auto v17 = nglGetDebugFlag("SyncRender");
        v40->set_ival(v17);
        v40->set_min_value(0.0);
        v40->set_max_value(1.0);
        v40->set_game_flags_handler(ngl_handler);
        v40->set_render_cb(ngl_render_callback);
        v133->add_entry(v40);
    }

    if (nglGetDebugFlagPtr("RenderSingleNode")) {
        auto *v41 = create_menu_entry(mString{"RenderSingleNode"});
        auto v18 = nglGetDebugFlag("RenderSingleNode");
        v41->set_ival(v18);
        v41->set_min_value(0.0);
        v41->set_max_value(1.0);
        v41->set_game_flags_handler(ngl_handler);
        v41->set_render_cb(ngl_render_callback);
        v133->add_entry(v41);
    }

    if (nglGetDebugFlagPtr("ShowPerfGraph")) {
        auto *v42 = create_menu_entry(mString{"ShowPerfGraph"});
        auto v19 = nglGetDebugFlag("ShowPerfGraph");
        v42->set_ival(v19);
        v42->set_min_value(0.0);
        v42->set_max_value(1.0);
        v42->set_game_flags_handler(ngl_handler);
        v42->set_render_cb(ngl_render_callback);
        v133->add_entry(v42);
    }

    if (nglGetDebugFlagPtr("RenderTargetShot")) {
        auto *v43 = create_menu_entry(mString{"RenderTargetShot"});

        auto v20 = nglGetDebugFlag("RenderTargetShot");
        v43->set_ival(v20);
        v43->set_min_value(0.0);
        v43->set_max_value(1.0);
        v43->set_game_flags_handler(ngl_handler);
        v43->set_render_cb(ngl_render_callback);
        v133->add_entry(v43);
    }

    if (nglGetDebugFlagPtr("WarningLevel")) {
        auto *v44 = create_menu_entry(mString{"WarningLevel"});

        auto v21 = nglGetDebugFlag("WarningLevel");
        v44->set_ival(v21);
        v44->set_min_value(0.0);
        v44->set_max_value(1.0);
        v44->set_game_flags_handler(ngl_handler);
        v44->set_render_cb(ngl_render_callback);
        v133->add_entry(v44);
    }
}

mString camera_render_callback(debug_menu_entry *a2)
{
    mString a1;
    if (a2 == nullptr) {
        a1 = entry_render_callback_default(a2);
        return a1;
    }

    switch (a2->get_ival()) {
    case 0: {
        a1 = {"Chase"};
        return a1;
    }
    case 1: {
        a1 = {"User"};
        return a1;
    }
    case 2: {
        a1 = {"SceneAnalyzer"};
        break;
    }
    default: {
        a1 = entry_render_callback_default(a2);

        break;
    }
    }

    return a1;
}

void create_camera_menu_items(debug_menu *parent)
{
    assert(parent != nullptr);

    auto *new_menu_entry = create_menu_entry(mString{"Camera"});

    new_menu_entry->set_render_cb(camera_render_callback);
    float v1[4] = {0, 2, 1, 1};
    new_menu_entry->set_fl_values(v1);
    new_menu_entry->set_game_flags_handler(game_flags_handler);
    new_menu_entry->set_id(18);
    new_menu_entry->set_ival(0);
    parent->add_entry(new_menu_entry);
    g_debug_camera_entry = new_menu_entry;
}

void populate_entity_animation_menu(debug_menu_entry *entry);

void create_entity_animation_menu(debug_menu *parent)
{
    auto *v5 = create_menu_entry(mString{"Entity Animations"});

    v5->set_submenu(nullptr);
    v5->set_game_flags_handler(populate_entity_animation_menu);
    parent->add_entry(v5);
}

static bool g_debug_menu_just_malored = false;

void warp_handler(debug_menu_entry *arg0)
{
    auto idx = arg0->get_id();
    auto *v6 = g_world_ptr->get_the_terrain();
    if (v6 != nullptr) {
        auto *v5 = v6->get_region(idx);
        if (v5->is_locked()) {
            auto v3 = v5->get_district_id();
            v6->unlock_district(v3);
        }

        auto &v2 = v5->field_A4;
        g_world_ptr->malor_point(v2, 0, false);
        g_debug_menu_just_malored = 1;
    }

    debug_menu::hide();
}

void warp_poi_handler([[maybe_unused]] debug_menu_entry *menu_entry)
{
    auto *v2 = g_femanager.IGO->field_54;
    if (v2 != nullptr) {
        vector3d a1;
        if (v2->get_the_arrow_target_pos(&a1)) {
            g_world_ptr->malor_point(a1, 0, false);
            g_debug_menu_just_malored = true;
            debug_menu::hide();
        }
    }
}

void sub_6A9FED(debug_menu_entry *entry)
{
    auto *v7 = (entity *)entry->m_data;
    if (v7 != nullptr && v7->is_an_actor()) {
        auto *v6 = (actor *)v7;
        auto *context = v6->get_resource_context();
        assert(context != nullptr);

        string_hash v4{entry->get_name().c_str()};
        resource_manager::push_resource_context(context);

        auto v3 = v6->play_anim(v4);
        resource_manager::pop_resource_context();
        v3.set_anim_speed(1.0);
        debug_menu::release_focus();
    }
}

void sub_6918AD(debug_menu_entry *entry)
{
    auto *e = (entity *)entry->m_data;
    if (e->is_an_actor()) {
        auto *a1 = create_menu(entry->get_name(), debug_menu::sort_mode_t::ascending);
        entry->set_submenu(a1);
        auto *v18 = (actor *)e;

        std::list<nalAnimClass<nalAnyPose> *> v17;
        actor::get_animations(v18, v17);
        for (auto *v15 : v17) {
            auto &v3 = v15->field_8;
            auto *v4 = v3.to_string();
            mString v14{v4};

            auto *v13 = create_menu_entry(v14);
            v13->m_data = entry->m_data;
            v13->set_game_flags_handler(sub_6A9FED);
            a1->add_entry(v13);
        }
    }
}

void populate_entity_animation_menu(debug_menu_entry *entry)
{
    auto *v26 = create_menu(entry->get_name(), debug_menu::sort_mode_t::ascending);
    entry->set_submenu(v26);
    entity::find_entities(1);
    auto &found_entities = (*entity::found_entities);
    for (auto *ent : found_entities) {
        if (ent->is_an_actor()) {
            auto *v23 = (actor *)ent;

            std::list<nalAnimClass<nalAnyPose> *> v22;
            actor::get_animations(v23, v22);
            if (!v22.empty()) {
                auto *v21 = create_menu_entry(mString{v23->get_id().to_string()});

                v21->set_game_flags_handler(sub_6918AD);
                v21->m_data = ent;
                v21->set_submenu(nullptr);
                v26->add_entry(v21);
            }
        }
    }
}

void populate_warp_menu(debug_menu_entry *entry)
{
    auto &v1 = entry->get_name();
    auto *v5 = create_menu(v1, debug_menu::sort_mode_t::ascending);

    auto *v20 = v5;
    entry->set_submenu(v5);

    auto *v19 = create_menu_entry(mString{"--WARP TO POI--"});

    v19->set_game_flags_handler(warp_poi_handler);
    v20->add_entry(v19);

    auto *v18 = g_world_ptr->get_the_terrain();
    if (v18 != nullptr) {
        for (auto idx = 0; idx < v18->total_regions; ++idx) {
            auto *v16 = v18->get_region(idx);
            if ((v16->flags & 0x4000) != 0) {
                auto &v3 = v16->get_name();
                mString v13{v3.to_string()};

                auto *v15 = create_menu_entry(v13);

                v15->set_id(idx);
                v15->set_game_flags_handler(warp_handler);
                v20->add_entry(v15);
            }
        }
    }
}

void create_warp_menu(debug_menu *parent)
{
    assert(parent != nullptr);

    auto *v5 = create_menu_entry(mString{"Warp"});

    v5->set_submenu(nullptr);
    v5->set_game_flags_handler(populate_warp_menu);
    parent->add_entry(v5);
}

void create_debug_render_menu(debug_menu *parent)
{
    assert(parent != nullptr);

    auto *v10 = create_menu("Debug Render", debug_menu::sort_mode_t::ascending);

    auto *v4 = create_menu_entry(v10);
    parent->add_entry(v4);

    for (auto i = 0u; i < 51u; ++i) {
        auto *v5 = create_menu_entry((debug_render_items_names())[i]);
        v5->set_id(i);
        v5->set_ival(debug_render_items()[i]);
        v5->set_p_ival(&debug_render_items()[i]);

        auto v1 = debug_render_get_min((debug_render_items_e)i);
        v5->set_min_value((float)v1);

        auto v2 = debug_render_get_max((debug_render_items_e)i);
        v5->set_max_value((float)v2);

        v10->add_entry(v5);
    }

    create_ngl_menu(v10);
}

void create_devopt_menu(debug_menu *parent)
{
    assert(parent != nullptr);

    auto *v22 = create_menu("Devopts", debug_menu::sort_mode_t::ascending);

    for (auto idx = 0; idx < 150; ++idx) {
        auto *v21 = get_option(idx);
        switch (v21->m_type) {
        case game_option_t::INT_OPTION: {
            auto *v20 = create_menu_entry(mString{v21->m_name});
            v20->set_p_ival(v21->m_value.p_ival);
            v20->set_min_value(-1000.0);
            v20->set_max_value(1000.0);
            v22->add_entry(v20);
            break;
        }
        case game_option_t::FLAG_OPTION: {
            auto *v19 = create_menu_entry(mString{v21->m_name});
            v19->set_pt_bval((bool *)v21->m_value.p_bval);
            v22->add_entry(v19);
            break;
        }
        case game_option_t::FLOAT_OPTION: {
            auto *v18 = create_menu_entry(mString{v21->m_name});
            v18->set_pt_fval(v21->m_value.p_fval);
            v18->set_min_value(-1000.0);
            v18->set_max_value(1000.0);
            v22->add_entry(v18);
            break;
        }
        default:
            break;
        }
    }

    auto *v5 = create_menu_entry(v22);
    parent->add_entry(v5);
}

void populate_gamefile_menu([[maybe_unused]] debug_menu_entry *entry)
{
    auto &v1 = entry->get_name();
    auto *v2 = create_menu(v1, debug_menu::sort_mode_t::ascending);

    auto *v494 = v2;
    entry->set_submenu(v2);
    if (g_game_ptr != nullptr) {
        auto *v493 = g_game_ptr->get_game_settings();

        auto *v492 = create_menu_entry(mString{"HERO_POINTS"});
        v492->set_p_ival(&v493->field_340.m_hero_points);
        v492->set_max_value(1000.0);
        v494->add_entry(v492);

        auto *v491 = create_menu_entry(mString{"UPG_MINIMAP_PTS"});
        v491->set_p_ival(&v493->field_340.field_4);
        v491->set_max_value(1000.0);
        v494->add_entry(v491);

        auto *v5 = create_menu_entry(mString{"UPG_IMPACT_WEB_PTS"});
        v5->set_p_ival(&v493->field_340.m_upg_impact_web_pts);
        v5->set_max_value(1000.0);
        v494->add_entry(v5);

        auto v6 = create_menu_entry(mString{"UPG_HERO_METER_PTS"});
        v6->set_p_ival(&v493->field_340.m_upg_hero_meter_pts);
        v6->set_max_value(1000.0);
        v2->add_entry(v6);

        auto *v7 = create_menu_entry(mString{"UPG_2ND_CHANCE_PTS"});
        v7->set_p_ival(&v493->field_340.field_10);
        v7->set_max_value(1000.0);
        v494->add_entry(v7);

        auto *v8 = create_menu_entry(mString{"UPG_SPEED_PTS"});
        v8->set_p_ival(&v493->field_340.field_14);
        v8->set_max_value(1000.0);
        v494->add_entry(v8);

        auto *v9 = create_menu_entry(mString{"UPG_UNDERDOG_PTS"});
        v9->set_p_ival(&v493->field_340.field_18);
        v9->set_max_value(1000.0);
        v494->add_entry(v9);

        auto *v10 = create_menu_entry(mString{"HERO_METER_LEVEL_1"});
        v10->set_p_ival(&v493->field_340.m_hero_meter_level_1);
        v10->set_max_value(1000.0);
        v494->add_entry(v10);

        auto *v11 = create_menu_entry(mString{"HERO_METER_LEVEL_2"});
        v11->set_p_ival(&v493->field_340.m_hero_meter_level_2);
        v11->set_max_value(1000.0);
        v494->add_entry(v11);

        auto *v12 = create_menu_entry(mString{"HERO_METER_LEVEL_3"});
        v12->set_p_ival(&v493->field_340.m_hero_meter_level_3);
        v12->set_max_value(1000.0);
        v494->add_entry(v12);

        auto *v13 = create_menu_entry(mString{"CUR_HERO_METER_LEVEL"});
        v13->set_p_ival(&v493->field_340.m_cur_hero_meter_lvl);
        v13->set_max_value(1000.0);
        v494->add_entry(v13);

        auto *v14 = create_menu_entry(mString{"CUR_HERO_METER_POINTS"});
        v14->set_p_ival(&v493->field_340.m_cur_hero_meter_pts);
        v14->set_max_value(1000.0);
        v2->add_entry(v14);

        auto *v15 = create_menu_entry(mString{"OPT_SCORE_DISPLAY"});
        v15->set_pt_bval(&v493->field_340.m_opt_score_display);
        v15->set_max_value(1000.0);
        v2->add_entry(v15);

        auto *v16 = create_menu_entry(mString{"OPT_CONT_1_RUMBLE"});
        v16->set_pt_bval(&v493->field_340.field_31);
        v16->set_max_value(1000.0);
        v2->add_entry(v16);

        auto *v17 = create_menu_entry(mString{"OPT_AUDIO_MODE"});
        v17->set_p_ival(&v493->field_340.field_34);
        v17->set_max_value(1000.0);
        v2->add_entry(v17);

        auto *v18 = create_menu_entry(mString{"OPT_LEVELS_GAME"});
        v18->set_pt_fval(&v493->field_340.field_38);
        v18->set_max_value(1000.0);
        v2->add_entry(v18);

        auto *v19 = create_menu_entry(mString{"OPT_LEVELS_MUSIC"});
        v19->set_pt_fval(&v493->field_340.field_3C);
        v19->set_max_value(1000.0);
        v2->add_entry(v19);

        auto *v20 = create_menu_entry(mString{"OPT_CONT_CONFIG"});
        v20->set_p_ival(&v493->field_340.field_40);
        v20->set_max_value(1000.0);
        v2->add_entry(v20);

        auto *v21 = create_menu_entry(mString{"MINI_MAP_ENABLED"});
        v21->set_pt_bval(&v493->field_340.m_mini_map_enabled);
        v21->set_max_value(1000.0);
        v2->add_entry(v21);

        auto *v22 = create_menu_entry(mString{"ENABLE_WEB_SHOT"});
        v22->set_pt_bval(&v493->field_340.m_enable_web_shot);
        v22->set_max_value(1000.0);
        v2->add_entry(v22);

        auto *v23 = create_menu_entry(mString{"ENABLE_WEB_DOME"});
        v23->set_pt_bval(&v493->field_340.field_46);
        v23->set_max_value(1000.0);
        v2->add_entry(v23);

        auto *v24 = create_menu_entry(mString{"ENABLE_ADV_WEB_DOME"});
        v24->set_pt_bval(&v493->field_340.field_47);
        v24->set_max_value(1000.0);
        v2->add_entry(v24);

        auto *v25 = create_menu_entry(mString{"ENABLE_WEB_GLOVES"});
        v25->set_pt_bval(&v493->field_340.m_web_gloves);
        v25->set_max_value(1000.0);
        v2->add_entry(v25);

        auto *v26 = create_menu_entry(mString{"ENABLE_ADV_WEB_GLOVES"});
        v26->set_pt_bval(&v493->field_340.m_adv_web_gloves);
        v26->set_max_value(1000.0);
        v2->add_entry(v26);

        auto *v27 = create_menu_entry(mString{"ENABLE_IMPACT_WEB"});
        v27->set_pt_bval(&v493->field_340.m_enable_impact_web);
        v27->set_max_value(1000.0);
        v2->add_entry(v27);

        auto *v28 = create_menu_entry(mString{"ENABLE_ADV_IMPACT_WEB"});
        v28->set_pt_bval(&v493->field_340.m_adv_impact_web);
        v28->set_max_value(1000.0);
        v2->add_entry(v28);

        auto *v29 = create_menu_entry(mString{"ENABLE_WEB_COWBOY"});
        v29->set_pt_bval(&v493->field_340.m_web_cowboy);
        v29->set_max_value(1000.0);
        v2->add_entry(v29);

        auto *v30 = create_menu_entry(mString{"ENABLE_WEB_YANK"});
        v30->set_pt_bval(&v493->field_340.field_4D);
        v30->set_max_value(1000.0);
        v2->add_entry(v30);

        auto *v31 = create_menu_entry(mString{"ENABLE_YANK_MANIP"});
        v31->set_pt_bval((bool *)&v493->field_340.field_4E);
        v31->set_max_value(1000.0);
        v2->add_entry(v31);

        auto *v32 = create_menu_entry(mString{"ENABLE_ZIP_WEB_ATTACK"});
        v32->set_pt_bval((bool *)&v493->field_340.field_4F);
        v32->set_max_value(1000.0);
        v2->add_entry(v32);

        auto *v33 = create_menu_entry(mString{"ENABLE_SWING_CANNONBALL"});
        v33->set_pt_bval(&v493->field_340.field_50);
        v33->set_max_value(1000.0);
        v2->add_entry(v33);

        auto *v34 = create_menu_entry(mString{"ENABLE_DIRECTIONAL_ATTACK"});
        v34->set_pt_bval(&v493->field_340.field_51);
        v34->set_max_value(1000.0);
        v2->add_entry(v34);

        auto *v35 = create_menu_entry(mString{"ENABLE_DIRECTIONAL_DODGE"});
        v35->set_pt_bval(&v493->field_340.field_52);
        v35->set_max_value(1000.0);
        v2->add_entry(v35);

        auto *v36 = create_menu_entry(mString{"DIFFICULTY"});
        v36->set_p_ival(&v493->field_340.m_difficulty);
        v36->set_max_value(1000.0);
        v2->add_entry(v36);

        auto *v37 = create_menu_entry(mString{"RUN_SENSITIVITY"});
        v37->set_pt_fval(&v493->field_340.m_run_sensitivity);
        v37->set_max_value(1000.0);
        v2->add_entry(v37);

        auto *v38 = create_menu_entry(mString{"CHAR_REL_MOVEMENT"});
        v38->set_pt_bval((bool *)&v493->field_340.field_5C);
        v38->set_max_value(1000.0);
        v2->add_entry(v38);

        auto *v39 = create_menu_entry(mString{"CRAWL_CHAR_REL_MOVEMENT"});
        v39->set_pt_bval((bool *)&v493->field_340.field_5D);
        v39->set_max_value(1000.0);
        v2->add_entry(v39);

        auto *v40 = create_menu_entry(mString{"SHOW_STYLE_SCORE"});
        v40->set_pt_bval(&v493->field_340.m_show_style_points);
        v40->set_max_value(1000.0);
        v2->add_entry(v40);

        auto *v41 = create_menu_entry(mString{"HERO_HEALTH"});
        v41->set_pt_fval(&v493->field_340.m_hero_health);
        v41->set_max_value(1000.0);
        v2->add_entry(v41);

        auto *v42 = create_menu_entry(mString{"HERO_TYPE"});
        v42->set_p_ival(&v493->field_340.m_hero_type);
        v42->set_max_value(1000.0);
        v2->add_entry(v42);

        auto *v43 = create_menu_entry(mString{"SWING_SPEED"});
        v43->set_p_ival(&v493->field_340.m_swing_speed);
        v43->set_max_value(1000.0);
        v2->add_entry(v43);

        auto *v44 = create_menu_entry(mString{"CAM_INVERTED_X"});
        v44->set_pt_bval(&v493->field_340.m_invert_camera_horz);
        v44->set_max_value(1000.0);
        v2->add_entry(v44);

        auto *v45 = create_menu_entry(mString{"CAM_INVERTED_Y"});
        v45->set_pt_bval(&v493->field_340.m_invert_camera_vert);
        v45->set_max_value(1000.0);
        v2->add_entry(v45);

        auto *v46 = create_menu_entry(mString{"SPEED_DEMON"});
        v46->set_pt_bval((bool *)&v493->field_340.field_6E);
        v46->set_max_value(1000.0);
        v2->add_entry(v46);

        auto *v47 = create_menu_entry(mString{"YOURE_AMAZING"});
        v47->set_pt_bval((bool *)&v493->field_340.field_6F);
        v47->set_max_value(1000.0);
        v2->add_entry(v47);

        auto *v48 = create_menu_entry(mString{"ERRAND_BOY"});
        v48->set_pt_bval((bool *)&v493->field_340.field_70);
        v48->set_max_value(1000.0);
        v2->add_entry(v48);

        auto *v49 = create_menu_entry(mString{"BETTER_TO_DO"});
        v49->set_pt_bval(&v493->field_340.field_71);
        v49->set_max_value(1000.0);
        v2->add_entry(v49);

        auto *v50 = create_menu_entry(mString{"FANBOY"});
        v50->set_pt_bval(&v493->field_340.field_72);
        v50->set_max_value(1000.0);
        v2->add_entry(v50);

        auto *v51 = create_menu_entry(mString{"ANGSTY"});
        v51->set_pt_bval(&v493->field_340.field_73);
        v51->set_max_value(1000.0);
        v2->add_entry(v51);

        auto *v52 = create_menu_entry(mString{"SECRET_IDENTITY"});
        v52->set_pt_bval(&v493->field_340.field_74);
        v52->set_max_value(1000.0);
        v2->add_entry(v52);

        auto *v53 = create_menu_entry(mString{"STYLE"});
        v53->set_pt_bval(&v493->field_340.field_75);
        v53->set_max_value(1000.0);
        v2->add_entry(v53);

        auto *v54 = create_menu_entry(mString{"FASHION"});
        v54->set_pt_bval(&v493->field_340.field_76);
        v54->set_max_value(1000.0);
        v2->add_entry(v54);

        auto *v55 = create_menu_entry(mString{"PICTURES"});
        v55->set_pt_bval(&v493->field_340.field_77);
        v55->set_max_value(1000.0);
        v2->add_entry(v55);

        auto *v56 = create_menu_entry(mString{"BUNNY"});
        v56->set_pt_bval(&v493->field_340.field_78);
        v56->set_max_value(1000.0);
        v2->add_entry(v56);

        auto *v57 = create_menu_entry(mString{"CLOBBER"});
        v57->set_pt_bval(&v493->field_340.field_79);
        v57->set_max_value(1000.0);
        v2->add_entry(v57);

        auto *v58 = create_menu_entry(mString{"SCRAP_HEAP"});
        v58->set_pt_bval(&v493->field_340.field_7A);
        v58->set_max_value(1000.0);
        v2->add_entry(v58);

        auto *v59 = create_menu_entry(mString{"SILVER_SPOON"});
        v59->set_pt_bval(&v493->field_340.field_7B);
        v59->set_max_value(1000.0);
        v2->add_entry(v59);

        auto *v60 = create_menu_entry(mString{"KUNG_FU_FIGHTING"});
        v60->set_pt_bval(&v493->field_340.field_7C);
        v60->set_max_value(1000.0);
        v2->add_entry(v60);

        auto *v61 = create_menu_entry(mString{"BIG_TIME_SUPER_HERO"});
        v61->set_pt_bval(&v493->field_340.field_7D);
        v61->set_max_value(1000.0);
        v2->add_entry(v61);

        auto *v62 = create_menu_entry(mString{"ENJOY_THE_SUNSHINE"});
        v62->set_pt_bval(&v493->field_340.field_7E);
        v62->set_max_value(1000.0);
        v2->add_entry(v62);

        auto *v63 = create_menu_entry(mString{"TIME_PLAYED"});
        v63->set_p_ival(&v493->field_340.field_80);
        v63->set_max_value(1000.0);
        v2->add_entry(v63);

        auto *v64 = create_menu_entry(mString{"STORY_PERCENT_COMPLETE"});
        v64->set_p_ival(&v493->field_340.field_84);
        v64->set_max_value(1000.0);
        v2->add_entry(v64);

        auto *v65 = create_menu_entry(mString{"STORY_MISSION_FAILURES"});
        v65->set_p_ival(&v493->field_340.field_88);
        v65->set_max_value(1000.0);
        v2->add_entry(v65);

        auto *v66 = create_menu_entry(mString{"STORY_MISSIONS_COMPLETED"});
        v66->set_p_ival(&v493->field_340.field_8C);
        v66->set_max_value(1000.0);
        v2->add_entry(v66);

        auto *v67 = create_menu_entry(mString{"SPIDEY_RACES_COMPLETED"});
        v67->set_p_ival(&v493->field_340.field_90);
        v67->set_max_value(1000.0);
        v2->add_entry(v67);

        auto *v68 = create_menu_entry(mString{"MILES_RUN_SPIDEY"});
        v68->set_pt_fval(&v493->field_340.field_94);
        v68->set_max_value(1000.0);
        v2->add_entry(v68);

        auto *v69 = create_menu_entry(mString{"MILES_CRAWLED_SPIDEY"});
        v69->set_pt_fval(&v493->field_340.field_98);
        v69->set_max_value(1000.0);
        v2->add_entry(v69);

        auto *v70 = create_menu_entry(mString{"MILES_WEB_SWINGING"});
        v70->set_pt_fval(&v493->field_340.field_9C);
        v70->set_max_value(1000.0);
        v2->add_entry(v70);

        auto *v71 = create_menu_entry(mString{"MILES_WEB_ZIPPING"});
        v71->set_pt_fval(&v493->field_340.m_miles_web_zipping);
        v71->set_max_value(1000.0);
        v2->add_entry(v71);

        auto *v72 = create_menu_entry(mString{"WEB_FLUID_USED"});
        v72->set_pt_fval(&v493->field_340.m_web_fluid_used);
        v72->set_max_value(1000.0);
        v2->add_entry(v72);

        auto *v73 = create_menu_entry(mString{"YANCY_DEFEATED"});
        v73->set_p_ival(&v493->field_340.field_A8);
        v73->set_max_value(1000.0);
        v2->add_entry(v73);

        auto *v74 = create_menu_entry(mString{"DIE_CASTEDEFEATED"});
        v74->set_p_ival(&v493->field_340.field_AC);
        v74->set_max_value(1000.0);
        v2->add_entry(v74);

        auto *v75 = create_menu_entry(mString{"HIGH_ROLLERSDEFEATED"});
        v75->set_p_ival(&v493->field_340.field_B0);
        v75->set_max_value(1000.0);
        v2->add_entry(v75);

        auto *v76 = create_menu_entry(mString{"FOU_TOU_BANGDEFEATED"});
        v76->set_p_ival(&v493->field_340.field_B4);
        v76->set_max_value(1000.0);
        v2->add_entry(v76);

        auto *v77 = create_menu_entry(mString{"VENOM_RACES_COMPLETED"});
        v77->set_p_ival(&v493->field_340.field_B8);
        v77->set_max_value(1000.0);
        v2->add_entry(v77);

        auto *v78 = create_menu_entry(mString{"MILES_RUN_VENOM"});
        v78->set_pt_fval(&v493->field_340.field_BC);
        v78->set_max_value(1000.0);
        v2->add_entry(v78);

        auto *v79 = create_menu_entry(mString{"MILES_CRAWLED_VENOM"});
        v79->set_pt_fval(&v493->field_340.field_C0);
        v79->set_max_value(1000.0);
        v2->add_entry(v79);

        auto *v80 = create_menu_entry(mString{"MILES_LOCOMOTION_JUMPED"});
        v80->set_pt_fval(&v493->field_340.field_C4);
        v80->set_max_value(1000.0);
        v2->add_entry(v80);

        auto *v81 = create_menu_entry(mString{"PEOPLE_EATEN"});
        v81->set_p_ival(&v493->field_340.field_C8);
        v81->set_max_value(1000.0);
        v2->add_entry(v81);

        auto *v82 = create_menu_entry(mString{"CARS_THROWN"});
        v82->set_p_ival(&v493->field_340.field_CC);
        v82->set_max_value(1000.0);
        v2->add_entry(v82);

        auto *v83 = create_menu_entry(mString{"VENOM_HOT_PERSUIT"});
        v83->set_p_ival(&v493->field_340.field_D0);
        v83->set_max_value(1000.0);
        v2->add_entry(v83);

        auto *v84 = create_menu_entry(mString{"TOKENS_COLLECTED_COUNT"});
        v84->set_p_ival(&v493->field_340.field_D4);
        v84->set_max_value(1000.0);
        v2->add_entry(v84);

        auto *v85 = create_menu_entry(mString{"RACE_POINTS_EARNED_COUNT"});
        v85->set_p_ival(&v493->field_340.field_D8);
        v85->set_max_value(1000.0);
        v2->add_entry(v85);

        auto *v86 = create_menu_entry(mString{"COMBAT_TOURS_COMPLETED_COUNT"});
        v86->set_p_ival(&v493->field_340.field_DC);
        v86->set_max_value(1000.0);
        v2->add_entry(v86);

        auto *v87 = create_menu_entry(mString{"CITY_EVENTS_COMPLETED_COUNT"});
        v87->set_p_ival(&v493->field_340.field_E0);
        v87->set_max_value(1000.0);
        v2->add_entry(v87);

        auto *v88 = create_menu_entry(mString{"TOKENS_COLLECTED_MAX"});
        v88->set_p_ival(&v493->field_340.field_E4);
        v88->set_max_value(1000.0);
        v2->add_entry(v88);

        auto *v89 = create_menu_entry(mString{"RACE_POINTS_EARNED_MAX"});
        v89->set_p_ival(&v493->field_340.field_E8);
        v89->set_max_value(1000.0);
        v2->add_entry(v89);

        auto *v90 = create_menu_entry(mString{"COMBAT_TOURS_COMPLETED_MAX"});
        v90->set_p_ival(&v493->field_340.field_EC);
        v90->set_max_value(1000.0);
        v2->add_entry(v90);

        auto *v91 = create_menu_entry(mString{"CITY_EVENTS_COMPLETED_MAX"});
        v91->set_p_ival(&v493->field_340.field_F0);
        v91->set_max_value(1000.0);
        v2->add_entry(v91);

        auto *v92 = create_menu_entry(mString{"TOKENS_COLLECTED_TOT_COUNT"});
        v92->set_p_ival(&v493->field_340.field_F4);
        v92->set_max_value(1000.0);
        v2->add_entry(v92);

        auto *v93 = create_menu_entry(mString{"RACE_POINTS_EARNED_TOT_COUNT"});
        v93->set_p_ival(&v493->field_340.field_F8);
        v93->set_max_value(1000.0);
        v2->add_entry(v93);

        auto *v94 = create_menu_entry(mString{"COMBAT_TOURS_COMPLETED_TOT_COUNT"});
        auto *v401 = v94;
        v401->set_p_ival(&v493->field_340.field_FC);
        v401->set_max_value(1000.0);
        v2->add_entry(v401);

        auto *v95 = create_menu_entry(mString{"CITY_EVENTS_COMPLETED_TOT_COUNT"});
        v95->set_p_ival(&v493->field_340.field_100);
        v95->set_max_value(1000.0);
        v494->add_entry(v95);

        auto *v96 = create_menu_entry(mString{"TOKENS_COLLECTED_TOT_MAX"});

        v96->set_p_ival(&v493->field_340.field_104);
        v96->set_max_value(1000.0);
        v494->add_entry(v96);

        auto *v97 = create_menu_entry(mString{"RACE_POINTS_EARNED_TOT_MAX"});

        v97->set_p_ival(&v493->field_340.field_108);
        v97->set_max_value(1000.0);
        v494->add_entry(v97);

        auto *v98 = create_menu_entry(mString{"COMBAT_TOURS_COMPLETED_TOT_MAX"});

        v98->set_p_ival(&v493->field_340.field_10C);
        v98->set_max_value(1000.0);
        v494->add_entry(v98);

        auto *v99 = create_menu_entry(mString{"CITY_EVENTS_COMPLETED_TOT_MAX"});

        auto *v396 = v99;
        v396->set_p_ival(&v493->field_340.field_110);
        v396->set_max_value(1000.0);
        v494->add_entry(v396);
    }
}

void create_gamefile_menu(debug_menu *parent)
{
    assert(parent != nullptr);

    auto *v5 = create_menu_entry(mString{"Saved Game Settings"});
    v5->set_submenu(nullptr);
    v5->set_game_flags_handler(populate_gamefile_menu);
    parent->add_entry(v5);
}

void game_flags_handler(debug_menu_entry *a1)
{
    enum {
        SLOW_MOTION = 2u,
    };

    switch (a1->get_id()) {
    case 0u: {
        auto v1 = a1->get_bval();
        g_game_ptr->enable_physics(v1);
        debug_menu::physics_state_on_exit = a1->get_bval();
        break;
    }
    case 1u: {
        g_game_ptr->flag.single_step = true;
        break;
    }
    case SLOW_MOTION: {
        static int old_frame_lock = 0;
        int v27;
        if (a1->get_bval()) {
            old_frame_lock = os_developer_options::instance->get_int(mString{"FRAME_LOCK"});
            v27 = 120;
        } else {
            v27 = old_frame_lock;
        }

        os_developer_options::instance->set_int(mString{"FRAME_LOCK"}, v27);
        debug_menu::hide();
        break;
    }
    case 3u: {
        if (a1->get_bval()) {
            spider_monkey::start();
            spider_monkey::on_level_load();
            auto *v2 = input_mgr::instance;
            auto *rumble_device = v2->rumble_ptr;

            assert(rumble_device != nullptr);
            rumble_device->disable_vibration();
        } else {
            spider_monkey::on_level_unload();
            spider_monkey::stop();
        }

        debug_menu::hide();
        break;
    }
    case 4u: {
        auto *v3 = input_mgr::instance;
        auto *rumble_device = v3->rumble_ptr;
        assert(rumble_device != nullptr);

        if (a1->get_bval())
            rumble_device->enable_vibration();
        else
            rumble_device->disable_vibration();

        break;
    }
    case 5u: {
        auto v4 = a1->get_ival();
        set_god_mode(v4);
        debug_menu::hide();
        break;
    }
    case 6u:  //Show Districts
    {
        debug_menu::hide();
        os_developer_options::instance->set_flag(mString{"SHOW_STREAMER_INFO"}, a1->get_bval());

        if (a1->get_bval()) {
            os_developer_options::instance->set_flag(mString{"SHOW_DEBUG_TEXT"}, true);
        }

        //TODO
        //sub_66C242(&g_game_ptr->field_4C);
        break;
    }
    case 7u: {
        debug_menu::hide();
        os_developer_options::instance->set_flag(mString{"SHOW_DEBUG_INFO"}, a1->get_bval());
        break;
    }
    case 8u: {
        debug_menu::hide();
        os_developer_options::instance->set_flag(mString{"SHOW_FPS"}, a1->get_bval());
        break;
    }
    case 9u: {
        auto v24 = a1->get_bval();
        auto *v5 = input_mgr::instance;
        if (!v5->field_30[1]) {
            v24 = false;
        }


        os_developer_options::instance->set_flag(mString{"USERCAM_ON_CONTROLLER2"}, v24);
        auto *v6 = input_mgr::instance;
        [[maybe_unused]] auto *v23 = v6->field_30[1];

        //TODO
        /*
        if ( !(*(unsigned __int8 (__fastcall **)(int))(*(_DWORD *)v23 + 44))(v23) )
        {
            j_debug_print_va("Controller 2 is not connected!";
            ->set_bval(a1, 0, 1);
            v24 = 0;
        }
        if ( v24 )
        {
            j_debug_print_va("User cam (theoretically) enabled on controller 2");
            v7 = (*(int (__fastcall **)(int))(*(_DWORD *)v23 + 8))(v23);
            sub_676E45(g_mouselook_controller, v7);
        }
        else
        {
            sub_676E45(g_mouselook_controller, -1);
        }
        */

        auto *v8 = g_world_ptr->get_hero_ptr(0);
        if (v8 != nullptr && g_game_ptr->m_user_camera_enabled) {
            if (a1->get_bval()) {
                auto *v14 = g_world_ptr->get_hero_ptr(0);
                v14->unsuspend(1);
            } else {
                auto *v15 = g_world_ptr->get_hero_ptr(0);
                v15->suspend(1);
            }
        }
        break;
    }
    case 11u: {
        debug_menu::hide();
        auto a2 = os_developer_options::instance->get_int(mString{"HIRES_SCREENSHOT_X"});
        auto a3 = os_developer_options::instance->get_int(mString{"HIRES_SCREENSHOT_Y"});
        g_game_ptr->begin_hires_screenshot(a2, a3);
        break;
    }
    case 12u: {
        g_game_ptr->push_lores();
        break;
    }
    case 13u: {
        static auto load_districts = TRUE;
        if (load_districts) {
            auto *v11 = g_world_ptr->get_the_terrain();
            v11->unload_all_districts_immediate();
            resource_manager::set_active_district(false);
        } else {
            resource_manager::set_active_district(true);
        }

        load_districts = !load_districts;
        debug_menu::hide();
        break;
    }
    case 14u: {
        //TODO
        //sub_66FBE0();
        debug_menu::hide();
        break;
    }
    case 15u: {
        //sub_697DB1();
        debug_menu::hide();
        break;
    }
    case 16u: {
        //TODO
        //sub_698D33();
        debug_menu::hide();
        break;
    }
    case 17u: {
        [[maybe_unused]] auto v12 = a1->get_bval();

        //TODO
        //sub_6A88A5(g_game_ptr, v12);
        break;
    }
    case 18u:  //Camera
    {
        auto v13 = a1->get_ival();
        a1->set_ival(v13, false);
        auto v16 = a1->get_ival();
        enum {
            CHASE_CAM = 0,
            USER_CAM = 1,
            SCENE_ANALYZER_CAM = 2,
        };

        switch (v16) {
        case CHASE_CAM: {
            if (geometry_manager::is_scene_analyzer_enabled()) {
                geometry_manager::enable_scene_analyzer(false);
            }

            g_game_ptr->enable_user_camera(false);

            break;
        }
        case USER_CAM: {
            if (geometry_manager::is_scene_analyzer_enabled()) {
                geometry_manager::enable_scene_analyzer(false);
            }

            g_game_ptr->enable_user_camera(true);
            break;
        }
        case SCENE_ANALYZER_CAM: {
            g_game_ptr->enable_user_camera(false);
            geometry_manager::enable_scene_analyzer(true);
            break;
        }
        default:
            assert(0);
        }
        break;
    }
    default:
        return;
    }
}

void create_game_flags_menu(debug_menu *parent)
{
    assert(parent != nullptr);

    auto *v3 = new debug_menu{"Game", debug_menu::sort_mode_t::undefined};
    auto *v92 = v3;

    auto *v4 = new debug_menu_entry{v92};

    parent->add_entry(v4);

    debug_menu_entry *v89 = nullptr;
    auto *v5 = new debug_menu_entry{mString{"Report SLF Recall Timeouts"}};
    v89 = v5;

    static bool byte_1597BC0 = false;
    v89->set_pt_bval(&byte_1597BC0);
    v92->add_entry(v89);

    v89 = create_menu_entry(mString{"Physics Enabled"});

    v89->set_bval(true);
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(0);
    v92->add_entry(v89);

    auto *v7 = new debug_menu_entry{mString{"Single Step"}};
    v89 = v7;

    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(1);
    v92->add_entry(v89);

    auto *v8 = new debug_menu_entry(mString{"Slow Motion Enabled"});
    v89 = v8;

    v89->set_bval(false);
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(2);
    v92->add_entry(v89);

    auto *v9 = new debug_menu_entry{mString{"Monkey Enabled"}};
    v89 = v9;

    auto v1 = spider_monkey::is_running();
    v89->set_bval(v1);
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(3);
    v92->add_entry(v89);

    auto *v10 = new debug_menu_entry{mString{"Rumble Enabled"}};

    v89 = v10;

    v89->set_bval(true);
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(4);
    v92->add_entry(v89);

    v89 = create_menu_entry(mString{"God Mode"});
    v89->set_ival(os_developer_options::instance->get_int(mString{"GOD_MODE"}));

    const float v2[4] = {0, 5, 1, 1};
    v89->set_fl_values(v2);
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(5);
    v92->add_entry(v89);

    v89 = create_menu_entry(mString{"Show Districts"});
    v89->set_bval(os_developer_options::instance->get_flag(mString{"SHOW_STREAMER_INFO"}));
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(6);
    v92->add_entry(v89);

    v89 = create_menu_entry(mString{"Show Hero Position"});
    v89->set_bval(os_developer_options::instance->get_flag(mString{"SHOW_DEBUG_INFO"}));
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(7);
    v92->add_entry(v89);

    v89 = create_menu_entry(mString{"Show FPS"});
    v89->set_bval(os_developer_options::instance->get_flag(mString{"SHOW_FPS"}));
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(8);
    v92->add_entry(v89);

    v89 = create_menu_entry(mString{"User Camera on Controller 2"});
    v89->set_bval(os_developer_options::instance->get_flag(mString{"USERCAM_ON_CONTROLLER2"}));
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(9);
    v92->add_entry(v89);

    v89 = create_menu_entry(mString{"Toggle Unload All Districts"});
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(13);
    v92->add_entry(v89);

    auto *v88 = create_menu("Save/Load", debug_menu::sort_mode_t::undefined);

    auto *v18 = create_menu_entry(v88);
    v92->add_entry(v18);

    v89 = create_menu_entry(mString{"Save Game"});
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(14);
    v88->add_entry(v89);

    v89 = create_menu_entry(mString{"Load Game"});
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(15);
    v88->add_entry(v89);

    v89 = create_menu_entry(mString{"Attemp Auto Load"});
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(16);
    v88->add_entry(v89);

    auto *v87 = create_menu("Screenshot", debug_menu::sort_mode_t::undefined);

    auto *v23 = create_menu_entry(v87);
    v92->add_entry(v23);

    v89 = create_menu_entry(mString{"Hires Screenshot"});
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(11);
    v87->add_entry(v89);

    v89 = create_menu_entry(mString{"Lores Screenshot"});
    v89->set_game_flags_handler(game_flags_handler);
    v89->set_id(12);
    v87->add_entry(v89);
    create_devopt_menu(v92);
    create_gamefile_menu(v92);
}
