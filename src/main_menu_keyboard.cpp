#include "main_menu_keyboard.h"

#include "common.h"
#include "fetext.h"
#include "frontendmenusystem.h"
#include "func_wrapper.h"
#include "panelanimfile.h"
#include "panelfile.h"
#include "panelquad.h"
#include "utility.h"
#include "variables.h"

VALIDATE_SIZE(main_menu_keyboard, 0x148);

main_menu_keyboard::main_menu_keyboard(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = 0x00895790;
        field_13A = false;
        field_13B = false;
        field_13C = false;
        field_13D = true;
        field_138 = 0;
        field_140 = a2;
        field_144 = 0;

        for (char c = 'A'; c <= 'Z'; ++c) {
            field_F4.push_back(c);
        }
        for (char c = '0'; c <= '9'; ++c) {
            field_F4.push_back(c);
        }
        field_F4.push_back(' ');
    } else {
        THISCALL(0x00633200, this, a2, a3, a4);
    }
}

void main_menu_keyboard::OnActivate()
{
    THISCALL(0x006241E0, this);
}

void main_menu_keyboard::_Init()
{
    auto *bytes = reinterpret_cast<unsigned char *>(this);
    auto *front_end = static_cast<FrontEndMenuSystem *>(field_140);
    assert(front_end != nullptr && front_end->field_7C != nullptr);
    auto *panel = front_end->field_7C;

    struct Binding {
        unsigned offset;
        const char *name;
    };
    static constexpr Binding quads[] = {
        {0x30, "mm_bkg_city"},
        {0x34, "mm_bkg_city_01"},
        {0x38, "mm_bkg_city_02"},
        {0x4C, "mm_bkg_detail_01"},
        {0x3C, "mm_bkg_detail_02"},
        {0x50, "mm_bkg_grey_b_01"},
        {0x40, "mm_bkg_grey_b_02"},
        {0x54, "mm_bkg_grey_b_03"},
        {0x58, "mm_bkg_grey_b_04"},
        {0x44, "mm_bkg_grey_b_05"},
        {0x48, "mm_bkg_grey_b_06"},
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
        {0x2C, "mm_logo_medium"},
        {0x8C, "mm_newname_box_hilite_a"},
        {0x90, "mm_newname_box_hilite_b"},
        {0x94, "mm_textbox_frame"},
        {0x98, "mm_textbox_gradient"},
        {0x9C, "mm_textbox_outline"},
        {0xA0, "mm_textbox_spider"},
        {0xA4, "mm_newname_burst_hilite"},
    };
    for (const auto &binding : quads) {
        auto *quad = panel->GetPQ(binding.name);
        assert(quad != nullptr);
        quad->TurnOn(false);
        *reinterpret_cast<PanelQuad **>(bytes + binding.offset) = quad;
    }

    static constexpr Binding texts[] = {
        {0x124, "mm_newname_text_header"},
        {0xC0, "mm_newname_letter_hilite"},
        {0x128, "mm_newname_text_line_01"},
        {0x12C, "mm_newname_text_line_02"},
        {0x130, "mm_newname_text_line_03"},
        {0x134, "mm_newname_text_line_04"},
        {0xC4, "mm_newname_letter_slot_01"},
        {0xC8, "mm_newname_letter_slot_02"},
        {0xCC, "mm_newname_letter_slot_03"},
        {0xD0, "mm_newname_letter_slot_04"},
        {0xD4, "mm_newname_letter_slot_05"},
        {0xD8, "mm_newname_letter_slot_06"},
        {0xDC, "mm_newname_letter_slot_07"},
        {0xE0, "mm_newname_letter_slot_08"},
    };
    for (const auto &binding : texts) {
        auto *text = panel->GetTextPointer(binding.name);
        if (text != nullptr) {
            text->SetShown(false);
        }
        *reinterpret_cast<FEText **>(bytes + binding.offset) = text;
    }

    *reinterpret_cast<PanelAnimFile **>(bytes + 0xA8) = panel->GetAnimationPointer(10);
    *reinterpret_cast<PanelAnimFile **>(bytes + 0xAC) = panel->GetAnimationPointer(11);
    *reinterpret_cast<PanelAnimFile **>(bytes + 0xB0) = panel->GetAnimationPointer(3);
    *reinterpret_cast<PanelAnimFile **>(bytes + 0xB8) = panel->GetAnimationPointer(1);
    *reinterpret_cast<PanelAnimFile **>(bytes + 0xB4) = panel->GetAnimationPointer(4);
    *reinterpret_cast<PanelAnimFile **>(bytes + 0xBC) = panel->GetAnimationPointer(2);

    auto *cursor_text = new FEText{};
    cursor_text->field_2C = 500.0f;
    cursor_text->field_30 = 400.0f - flt_965BDC;
    cursor_text->SetPos(500.0f, 400.0f);
    cursor_text->field_4C = color32{0xFFC8C8C8};
    cursor_text->field_3C = 1.2f;
    cursor_text->field_40 = 1.2f;
    cursor_text->field_18 = static_cast<font_index>(1);
    field_144 = reinterpret_cast<int>(cursor_text);
}

void main_menu_keyboard::OnDeactivate(FEMenu *a2)
{
    THISCALL(0x00613D10, this, a2);
}

void main_menu_keyboard_patch()
{
    FUNC_ADDRESS(address, &main_menu_keyboard::OnActivate);
    set_vfunc(0x008957BC, address);
}
