#include "pause_menu_save_load_hookup.h"

#include "game.h"
#include "localized_string_table.h"

#include "common.h"
#include "func_wrapper.h"

#include <cstdio>
#include <ctime>

VALIDATE_OFFSET(pause_menu_save_load_hookup, field_24, 0x24);
VALIDATE_SIZE(pause_menu_save_load_hookup::save_slot, 0x24);
VALIDATE_SIZE(pause_menu_save_load_hookup, 0x298);

void pause_menu_save_load_hookup::initialize()
{
    field_0[0] = 0x00894364;
    field_0[2] = 0;
    field_0[3] = 3;
    field_0[4] = 3;
    field_0[5] = 0;
    field_0[6] = 0;
    field_0[7] = 0;
    field_0[8] = 0;
    field_24 = false;
    field_25 = false;
    field_26 = false;
    field_28 = 0;

    auto *table = g_game_ptr->field_7C;
    field_98[0] = table->lookup_localized_string(static_cast<global_text_enum>(57));
    field_98[1] = table->lookup_localized_string(static_cast<global_text_enum>(58));
    field_98[2] = table->lookup_localized_string(static_cast<global_text_enum>(55));
    field_98[3] = "ALL UNSAVED PROGRESS WILL BE LOST IF YOU LOAD A GAME. PROCEED WITH LOAD?";
    field_98[4] = "THE PREVIOUSLY SAVED GAME WILL BE LOST IF YOU SAVE. PROCEED WITH SAVE?";
    field_98[5] = "READY TO SAVE?";
    field_98[6] = "";
    field_98[7] = "";
    field_98[8] = "LOADING GAME.";
    field_98[9] = "SAVING GAME.";

    char date_time[64] {};
    const std::time_t now = std::time(nullptr);
    if (const auto *local_time = std::localtime(&now))
        std::strftime(date_time, sizeof(date_time), "%x %X", local_time);
    field_98[10] = date_time;

    field_98[11] = "THERE ARE CURRENTLY NO SAVED GAMES TO LOAD.";
    field_98[12] = "THE GAME HAS BEEN SUCCESSFULLY SAVED.";
    field_98[13] = "LOAD SUCCESSFUL.";
    field_98[14] = "";
    field_98[15] = "";
    field_98[16] = "";
    field_98[17] = "FAIL.";
    field_98[18] = "";
    field_98[19] = "";
    field_98[20] = table->lookup_localized_string(static_cast<global_text_enum>(255));
    field_98[21] = table->lookup_localized_string(static_cast<global_text_enum>(254));
    field_98[22] = table->lookup_localized_string(static_cast<global_text_enum>(54));
    field_98[23] = table->lookup_localized_string(static_cast<global_text_enum>(50));
    field_98[24] = table->lookup_localized_string(static_cast<global_text_enum>(51));
    field_98[25] = table->lookup_localized_string(static_cast<global_text_enum>(59));
    field_98[26] = table->lookup_localized_string(static_cast<global_text_enum>(53));
    field_98[27] = table->lookup_localized_string(static_cast<global_text_enum>(56));
    field_98[28] = table->lookup_localized_string(static_cast<global_text_enum>(471));
    field_98[29] = table->lookup_localized_string(static_cast<global_text_enum>(37));
    field_98[30] = table->lookup_localized_string(static_cast<global_text_enum>(38));
    field_98[31] = "";

    for (int i = 0; i < 3; ++i) {
        char slot_name[256] {};
        std::snprintf(slot_name, sizeof(slot_name), "%d. %s", i + 1, field_98[2].c_str());
        field_2C[i].field_0 = slot_name;
        field_2C[i].field_10 = "-- --- ---- - --:--:--";
        field_2C[i].field_20 = true;
    }
}

void pause_menu_save_load_hookup::SaveFile(int a2)
{
    THISCALL(0x0062C970, this, a2);
}

void pause_menu_save_load_hookup::set_current_state(int a2)
{
    THISCALL(0x0062BF90, this, a2);
}

void pause_menu_save_load_hookup::LoadFile(int a2)
{
    THISCALL(0x0062C9B0, this, a2);
}
