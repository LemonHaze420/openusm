#include "main_menu_load.h"

#include "common.h"
#include "game.h"
#include "fetext.h"
#include "frontendmenusystem.h"
#include "localized_string_table.h"
#include "mstring.h"

#include "panelanimfile.h"
#include "panelfile.h"
#include "panelquad.h"
#include <cstring>
#include <new>
#include "func_wrapper.h"

VALIDATE_SIZE(main_menu_load, 0x174);

main_menu_load::main_menu_load(FEMenuSystem *a2, int a4, int a5)
    : FEMenu(a2, 0, a4, a5, 8, 0)
{
    if constexpr (STANDALONE_SYSTEM)
    {
        auto *bytes = reinterpret_cast<unsigned char *>(this);
        std::memset(bytes + 0x2C, 0, sizeof(*this) - 0x2C);
        m_vtbl = 0x008947A8;

        auto *title = new (bytes + 0xE4) mString {};
        auto *date = new (bytes + 0xF4) mString {};
        if (g_game_ptr != nullptr && g_game_ptr->field_7C != nullptr)
            *title = g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(55));
        *date = "-- --- ---- - --:--:--";

        *reinterpret_cast<FEMenuSystem **>(bytes + 0x170) = a2;
        *reinterpret_cast<float *>(bytes + 0x2C) = 0.8f;
        *reinterpret_cast<float *>(bytes + 0x30) = 1.0f;
        *reinterpret_cast<float *>(bytes + 0x34) = 0.8f;
        *reinterpret_cast<uint32_t *>(bytes + 0x38) = 0xFFC87238;
        *reinterpret_cast<uint32_t *>(bytes + 0x3C) = 0xFFE6D03F;
    }
    else
    {
        THISCALL(0x00623950, this, a2, a4, a5);
    }
}

void main_menu_load::_Init()
{
    auto *bytes = reinterpret_cast<unsigned char *>(this);
    auto *front_end = static_cast<FrontEndMenuSystem *>(
        *reinterpret_cast<FEMenuSystem **>(bytes + 0x170));
    assert(front_end != nullptr && front_end->field_7C != nullptr);
    auto *panel = front_end->field_7C;

    struct Binding {
        unsigned offset;
        const char *name;
    };
    static constexpr Binding quads[] {
        {0x44, "mm_bkg_city"},
        {0x48, "mm_bkg_city_01"},
        {0x4C, "mm_bkg_city_02"},
        {0x60, "mm_bkg_detail_01"},
        {0x50, "mm_bkg_detail_02"},
        {0x64, "mm_bkg_grey_b_01"},
        {0x54, "mm_bkg_grey_b_02"},
        {0x68, "mm_bkg_grey_b_03"},
        {0x6C, "mm_bkg_grey_b_04"},
        {0x58, "mm_bkg_grey_b_05"},
        {0x5C, "mm_bkg_grey_b_06"},
        {0x70, "mm_bkg_detail_light_01"},
        {0x74, "mm_bkg_detail_light_02"},
        {0x78, "mm_bkg_detail_light_03"},
        {0x7C, "mm_bkg_detail_light_04"},
        {0x80, "mm_bkg_detail_light_05"},
        {0x84, "mm_bkg_detail_light_06"},
        {0x88, "mm_bkg_detail_dark_01"},
        {0x8C, "mm_bkg_detail_dark_02"},
        {0x90, "mm_bkg_detail_dark_03"},
        {0x94, "mm_bkg_detail_dark_04"},
        {0x98, "mm_bkg_detail_dark_05"},
        {0x9C, "mm_bkg_detail_dark_06"},
        {0x40, "mm_logo_medium"},
        {0xA0, "mm_options_box_hilite_a"},
        {0xA4, "mm_options_box_hilite_b"},
        {0xA8, "mm_textbox_frame"},
        {0xAC, "mm_textbox_gradient"},
        {0xB0, "mm_textbox_outline"},
        {0xB4, "mm_textbox_spider"},
    };
    for (const auto &binding : quads)
    {
        auto *quad = panel->GetPQ(binding.name);
        assert(quad != nullptr);
        quad->TurnOn(false);
        *reinterpret_cast<PanelQuad **>(bytes + binding.offset) = quad;
    }

    static constexpr Binding texts[] {
        {0xC0, "mm_loadsg_text_header"},
        {0xC4, "mm_loadsg_text_link_01a"},
        {0xC8, "mm_loadsg_text_link_01b"},
        {0xCC, "mm_loadsg_text_link_02a"},
        {0xD0, "mm_loadsg_text_link_02b"},
        {0xD4, "mm_loadsg_text_link_03a"},
        {0xD8, "mm_loadsg_text_link_03b"},
    };
    for (const auto &binding : texts)
    {
        auto *text = panel->GetTextPointer(binding.name);
        if (text != nullptr)
            text->SetShown(false);
        *reinterpret_cast<FEText **>(bytes + binding.offset) = text;
    }

    *reinterpret_cast<PanelAnimFile **>(bytes + 0xB8) =
        panel->GetAnimationPointer(7);
    *reinterpret_cast<PanelAnimFile **>(bytes + 0xBC) =
        panel->GetAnimationPointer(12);
}
