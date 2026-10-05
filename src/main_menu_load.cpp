#include "main_menu_load.h"

#include "common.h"
#include "game.h"
#include "game_data_essentials.h"
#include "fetext.h"
#include "frontendmenusystem.h"
#include "localized_string_table.h"
#include "mstring.h"

#include "panelanimfile.h"
#include "panelfile.h"
#include "panelquad.h"
#include "func_wrapper.h"
#include <cstdio>
#include <algorithm>
#include <iterator>

VALIDATE_SIZE(main_menu_load, 0x174);
VALIDATE_OFFSET(main_menu_load, field_2C, 0x2C);
VALIDATE_OFFSET(main_menu_load, field_40, 0x40);
VALIDATE_OFFSET(main_menu_load, field_B8, 0xB8);
VALIDATE_OFFSET(main_menu_load, field_C0, 0xC0);
VALIDATE_OFFSET(main_menu_load, field_E4, 0xE4);
VALIDATE_OFFSET(main_menu_load, field_F4, 0xF4);
VALIDATE_OFFSET(main_menu_load, field_104, 0x104);
VALIDATE_OFFSET(main_menu_load, field_170, 0x170);

main_menu_load::main_menu_load(FEMenuSystem *a2, int a4, int a5) : FEMenu(a2, 0, a4, a5, 8, 0)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = 0x008947A8;
        field_2C = 0.8f;
        field_30 = 1.0f;
        field_34 = 0.8f;
        field_38 = color32{0xFFC87238};
        field_3C = color32{0xFFE6D03F};
        std::fill(std::begin(field_40), std::end(field_40), nullptr);
        field_B8 = nullptr;
        field_BC = nullptr;
        std::fill(std::begin(field_C0), std::end(field_C0), nullptr);
        field_DC = 0;
        field_E0 = 0;
        field_170 = a2;
        if (g_game_ptr != nullptr && g_game_ptr->field_7C != nullptr)
            field_E4 = g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(55));
        field_F4 = "-- --- ---- - --:--:--";
        for (auto &slot : field_104) {
            slot.date = field_F4;
            slot.disabled = true;
        }
    } else {
        THISCALL(0x00623950, this, a2, a4, a5);
    }
}

void main_menu_load::_Init()
{
    auto *front_end = static_cast<FrontEndMenuSystem *>(field_170);
    assert(front_end != nullptr && front_end->field_7C != nullptr);
    auto *panel = front_end->field_7C;

    struct QuadBinding {
        PanelQuad **destination;
        const char *name;
    };
    const QuadBinding quads[]{
        {&field_40[1], "mm_bkg_city"},
        {&field_40[2], "mm_bkg_city_01"},
        {&field_40[3], "mm_bkg_city_02"},
        {&field_40[8], "mm_bkg_detail_01"},
        {&field_40[4], "mm_bkg_detail_02"},
        {&field_40[9], "mm_bkg_grey_b_01"},
        {&field_40[5], "mm_bkg_grey_b_02"},
        {&field_40[10], "mm_bkg_grey_b_03"},
        {&field_40[11], "mm_bkg_grey_b_04"},
        {&field_40[6], "mm_bkg_grey_b_05"},
        {&field_40[7], "mm_bkg_grey_b_06"},
        {&field_40[12], "mm_bkg_detail_light_01"},
        {&field_40[13], "mm_bkg_detail_light_02"},
        {&field_40[14], "mm_bkg_detail_light_03"},
        {&field_40[15], "mm_bkg_detail_light_04"},
        {&field_40[16], "mm_bkg_detail_light_05"},
        {&field_40[17], "mm_bkg_detail_light_06"},
        {&field_40[18], "mm_bkg_detail_dark_01"},
        {&field_40[19], "mm_bkg_detail_dark_02"},
        {&field_40[20], "mm_bkg_detail_dark_03"},
        {&field_40[21], "mm_bkg_detail_dark_04"},
        {&field_40[22], "mm_bkg_detail_dark_05"},
        {&field_40[23], "mm_bkg_detail_dark_06"},
        {&field_40[0], "mm_logo_medium"},
        {&field_40[24], "mm_options_box_hilite_a"},
        {&field_40[25], "mm_options_box_hilite_b"},
        {&field_40[26], "mm_textbox_frame"},
        {&field_40[27], "mm_textbox_gradient"},
        {&field_40[28], "mm_textbox_outline"},
        {&field_40[29], "mm_textbox_spider"},
    };
    for (const auto &binding : quads) {
        *binding.destination = panel->GetPQ(binding.name);
        assert(*binding.destination != nullptr);
        (*binding.destination)->TurnOn(false);
    }

    static constexpr const char *text_names[]{
        "mm_loadsg_text_header",
        "mm_loadsg_text_link_01a",
        "mm_loadsg_text_link_01b",
        "mm_loadsg_text_link_02a",
        "mm_loadsg_text_link_02b",
        "mm_loadsg_text_link_03a",
        "mm_loadsg_text_link_03b",
    };
    for (int i = 0; i < 7; ++i) {
        field_C0[i] = panel->GetTextPointer(text_names[i]);
        if (field_C0[i] != nullptr)
            field_C0[i]->SetShown(false);
    }

    field_B8 = panel->GetAnimationPointer(7);
    field_BC = panel->GetAnimationPointer(12);
}

void main_menu_load::SetSaveSlot(int slot, const game_data_essentials *data)
{
    assert(slot >= 0 && slot < 3);
    auto &destination = field_104[slot];
    destination.disabled = data == nullptr || data->timestamp.year == 0;
    if (destination.disabled)
        return;

    destination.title = mString{slot + 1} + ". " + data->field_14;
    const int month = data->timestamp.month >= 1 && data->timestamp.month <= 12 ? data->timestamp.month : 1;
    const unsigned int elapsed = static_cast<unsigned int>(data->field_C);
    char date[64]{};
    std::snprintf(date,
                  sizeof(date),
                  "%02d %s %04d - %02u:%02u:%02u",
                  data->timestamp.day,
                  g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(month + 436)),
                  data->timestamp.year,
                  std::min(elapsed / 3600, 99u),
                  elapsed / 60 % 60,
                  elapsed % 60);
    destination.date = date;
}
