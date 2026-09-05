#include "main_menu_memcard_check.h"

#include "common.h"
#include "frontendmenusystem.h"
#include "game.h"
#include "localized_string_table.h"
#include "panelfile.h"
#include "func_wrapper.h"
#include "mstring.h"

#include <cstring>
#include <iterator>
#include <new>

VALIDATE_SIZE(main_menu_memcard_check, 0x1A0u);
VALIDATE_OFFSET(main_menu_memcard_check, field_100, 0x100);

main_menu_memcard_check::main_menu_memcard_check(FEMenuSystem *a2, int a4, int a5)
    : FEMenu(a2, 0, a4, a5, 8, 0)
{
    if constexpr (STANDALONE_SYSTEM)
    {
        auto *bytes = reinterpret_cast<unsigned char *>(this);
        std::memset(bytes + 0x2C, 0, sizeof(*this) - 0x2C);
        m_vtbl = 0x00895910;
        *reinterpret_cast<std::intptr_t *>(bytes + 0x2C) = 0x00895908;

        for (size_t offset = 0x12C; offset <= 0x18C; offset += sizeof(mString))
            new (bytes + offset) mString {};

        *reinterpret_cast<FEMenuSystem **>(bytes + 0x19C) = a2;
        bytes[0x128] = 1;

        static bool reported_disabled_memcard = false;
        if (!reported_disabled_memcard)
        {
            sp_log("Memory-card menu: insert/remove notifications disabled in standalone mode.\n");
            reported_disabled_memcard = true;
        }
    }
    else
    {
        THISCALL(0x00632B30, this, a2, a4, a5);
    }
}

void main_menu_memcard_check::_Init()
{
    auto *bytes = reinterpret_cast<unsigned char *>(this);
    auto *menu_system = *reinterpret_cast<FrontEndMenuSystem **>(bytes + 0x19C);
    assert(menu_system != nullptr && menu_system->field_7C != nullptr);
    auto *panel = menu_system->field_7C;

    struct QuadBinding {
        size_t offset;
        const char *name;
    };
    static constexpr QuadBinding quads[] {
        {0x30, "mm_bkg_city"},
        {0x34, "mm_bkg_city_01"},
        {0x38, "mm_bkg_city_02"},
        {0x4C, "mm_bkg_detail_01"},
        {0x3C, "mm_bkg_detail_02"},
        {0x50, "mm_bkg_grey_a_01"},
        {0x40, "mm_bkg_grey_a_02"},
        {0x54, "mm_bkg_grey_a_03"},
        {0x44, "mm_bkg_grey_a_04"},
        {0x58, "mm_bkg_grey_a_06"},
        {0x48, "mm_bkg_grey_a_07"},
        {0x94, "mm_logo_medium"},
        {0x98, "mm_logo_main"},
        {0x8C, "mm_bkg_white_02"},
        {0x90, "mm_bkg_white_03"},
        {0x5C, "mm_bkg_detail_light_01"},
        {0x60, "mm_bkg_detail_light_02"},
        {0x64, "mm_bkg_detail_light_03"},
        {0x68, "mm_bkg_detail_light_04"},
        {0x6C, "mm_bkg_detail_light_05"},
        {0x70, "mm_bkg_detail_light_06"},
        {0x74, "mm_bkg_detail_dark_01"},
        {0x78, "mm_bkg_detail_dark_02"},
        {0x7C, "mm_bkg_detail_dark_03"},
        {0x80, "mm_bkg_detail_dark_04"},
        {0x84, "mm_bkg_detail_dark_05"},
        {0x88, "mm_bkg_detail_dark_06"},
        {0x9C, "mm_textbox_frame"},
        {0xA0, "mm_textbox_gradient"},
        {0xA4, "mm_textbox_outline"},
        {0xA8, "mm_textbox_spider"},
        {0xAC, "mm_loading_bar_01"},
        {0xB0, "mm_loading_bar_02"},
        {0xB4, "mm_loading_bar_gauge"},
    };
    for (const auto &binding : quads)
    {
        auto *quad = panel->GetPQ(binding.name);
        assert(quad != nullptr);
        quad->TurnOn(false);
        *reinterpret_cast<PanelQuad **>(bytes + binding.offset) = quad;
    }

    static constexpr QuadBinding texts[] {
        {0xF4, "mm_mainmenu_text_CHECKING"},
        {0xEC, "mm_dialog_box_text_BODY"},
        {0xD4, "mm_dialog_box_text_line_01"},
        {0xD8, "mm_dialog_box_text_line_03"},
        {0xDC, "mm_dialog_box_text_line_02"},
        {0xF0, "mm_dialog_box_text_BODY_ps2"},
        {0xE0, "mm_dialog_box_text_line_01_ps2"},
        {0xE4, "mm_dialog_box_text_line_02_ps2"},
        {0xE8, "mm_dialog_box_text_line_03_ps2"},
    };
    for (const auto &binding : texts)
    {
        auto *text = panel->GetTextPointer(binding.name);
        if (text != nullptr)
            text->SetShown(false);
        *reinterpret_cast<FEText **>(bytes + binding.offset) = text;
    }

    static constexpr int animation_indices[] {6, 8, 9, 16, 18, 3, 1};
    for (size_t i = 0; i < std::size(animation_indices); ++i)
        *reinterpret_cast<PanelAnimFile **>(bytes + 0xB8 + i * sizeof(void *)) =
            panel->GetAnimationPointer(animation_indices[i]);

    auto *message = reinterpret_cast<mString *>(bytes + 0x18C);
    if (g_game_ptr != nullptr && g_game_ptr->field_7C != nullptr)
        *message = g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(464));
}
