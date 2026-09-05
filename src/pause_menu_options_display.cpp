#include "pause_menu_options_display.h"

#include "common.h"
#include "femultilinetext.h"
#include "fetext.h"
#include "func_wrapper.h"
#include "game.h"
#include "localized_string_table.h"
#include "panelfile.h"
#include "panelquad.h"
#include "pausemenusystem.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(pause_menu_options_display, 0x1A4u);
VALIDATE_SIZE(pause_menu_options_data, 0x120u);

pause_menu_options_data::pause_menu_options_data(int a2)
{
    field_0[2] = a2;
}

void pause_menu_options_data::initialize()
{
    auto *table = g_game_ptr->field_7C;
    for (int i = 0; i < 8; ++i) {
        field_20[i] = table->lookup_localized_string(static_cast<global_text_enum>(236 + i));
        field_A0[i] = table->lookup_localized_string(static_cast<global_text_enum>(244 + i));
    }
}

pause_menu_options_display::pause_menu_options_display(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0)
{
    field_140 = a2;
    m_vtbl = 0x00894150;

    for (auto &value : field_2C) {
        value = 0;
    }
    for (auto &value : field_F0) {
        value = 0;
    }

    field_148 = 0;
    field_150 = 5;
    field_14C = 5;
    field_154 = 0;
    field_158 = 0;

    field_144 = new pause_menu_options_data(10);
    field_144->initialize();
}

void pause_menu_options_display::_Load()
{
    TRACE("pause_menu_options_display::_Load");
    if constexpr (STANDALONE_SYSTEM) {
        auto *panel = static_cast<PauseMenuSystem *>(field_140)->field_2C;
        auto store_quad = [](int &slot, PanelQuad *quad) {
            slot = reinterpret_cast<std::intptr_t>(quad);
        };
        auto store_text = [](int &slot, FEText *text) {
            slot = reinterpret_cast<std::intptr_t>(text);
        };

        store_quad(field_12C[0], panel->GetPQ("pm_splash_dialog_box_01"));
        store_quad(field_12C[1], panel->GetPQ("pm_splash_dialog_box_02"));
        store_text(field_12C[2], panel->GetTextPointer("pm_splash_dialog_box_text_BODY"));
        store_text(field_12C[3], panel->GetTextPointer("pm_splash_dialog_box_text_NOWAY"));
        store_text(field_12C[4], panel->GetTextPointer("pm_splash_dialog_box_text_OKAY"));

        auto *body = reinterpret_cast<FEMultiLineText *>(field_12C[2]);
        body->SetText(static_cast<global_text_enum>(252));
        body->SetNoFlash(color32{0xFFC8C8C8});
        auto *no_way = reinterpret_cast<FEText *>(field_12C[3]);
        no_way->SetText(static_cast<global_text_enum>(254));
        auto *okay = reinterpret_cast<FEText *>(field_12C[4]);
        okay->SetText(static_cast<global_text_enum>(255));

        static constexpr const char *quad_names[49] = {
            "pm_all_back_01", "pm_all_back_02", "pm_all_back_02a", "pm_all_back_02b", "pm_all_back_03",
            "pm_all_back_04", "pm_all_back_05", "pm_all_detail_02", "pm_all_detail_03", "pm_all_detail_04",
            "pm_all_detail_05", "pm_all_detail_06", "pm_all_detail_07", "pm_all_detail_08", "pm_all_detail_09",
            "pm_all_detail_10", "pm_all_detail_11", "pm_all_box_01", "pm_all_box_02", "pm_all_box_03",
            "pm_all_icon", "pm_opt_text_box_01", "pm_opt_text_box_02", "pm_opt_hilite_text",
            "pm_opt_hilite_text_01", "pm_opt_gamesound_box_01", "pm_opt_gamesound_box_02",
            "pm_opt_gamesound_meter_01", "pm_opt_gamesound_meter_02", "pm_opt_gamesound_meter_03",
            "pm_opt_gamesound_meter_04", "pm_opt_gamesound_meter_05", "pm_opt_gamesound_meter_06",
            "pm_opt_gamesound_meter_07", "pm_opt_gamesound_meter_08", "pm_opt_gamesound_meter_09",
            "pm_opt_gamesound_meter_10", "pm_opt_music_box_01", "pm_opt_music_box_02", "pm_opt_music_meter_01",
            "pm_opt_music_meter_02", "pm_opt_music_meter_03", "pm_opt_music_meter_04", "pm_opt_music_meter_05",
            "pm_opt_music_meter_06", "pm_opt_music_meter_07", "pm_opt_music_meter_08", "pm_opt_music_meter_09",
            "pm_opt_music_meter_10",
        };
        for (int i = 0; i < 49; ++i) {
            store_quad(field_2C[i], panel->GetPQ(quad_names[i]));
        }

        static constexpr const char *text_names[15] = {
            "pm_header_text_OPTIONS", "pm_opt_text_left_01_AUDIOLEVELS", "pm_opt_text_left_02_GAMESOUND",
            "pm_opt_text_left_03_MUSIC", "pm_opt_text_left_04_SCOREDISPLAY",
            "pm_opt_text_left_05_CONTROLLERVIBRATION", "pm_opt_text_left_06_CONTROLLERSETUP",
            "pm_opt_text_left_07_INVERTCAMERAUPDOWN", "pm_opt_text_right_04_BLANK",
            "pm_opt_text_right_05_BLANK", "pm_opt_text_right_06_BLANK", "pm_opt_text_right_07_BLANK",
            "pm_opt_text_right_08_BLANK", "pm_opt_text_right_09", "pm_opt_cont_text_help",
        };
        for (int i = 0; i < 15; ++i) {
            store_text(field_F0[i], panel->GetTextPointer(text_names[i]));
        }
    } else {
        THISCALL(0x0063BE80, this);
    }
}

void pause_menu_options_display_patch()
{
    FUNC_ADDRESS(address, &pause_menu_options_display::_Load);
    set_vfunc(0x00894160, address);
}
