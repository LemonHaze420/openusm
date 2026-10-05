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
#include "cursor.h"
#include "femultilinetext.h"
#include "fileusm.h"
#include "input.h"
#include "inputsettings.h"
#include "main_menu_memcard_check.h"
#include "panelanim.h"
#include "game.h"
#include "game_settings.h"
#include <cstring>

namespace {
void play_keyboard_animation(PanelAnimFile *animation, bool loop)
{
    for (int i = 0; i < animation->field_0.size(); ++i)
        animation->field_0.m_data[i]->field_14->StartAnim(true);
    animation->field_18 = bit_cast<int>(0.0f);
    animation->field_1C = bit_cast<int>(0.0f);
    animation->field_20 = animation->field_14;
    animation->field_24 = 0;
    animation->field_28 = loop ? 1 : 0;
    animation->field_2C = false;
    animation->field_2D = true;
}
}  // namespace

VALIDATE_SIZE(main_menu_keyboard, 0x148);

main_menu_keyboard::main_menu_keyboard(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
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
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x006241E0, this);
        return;
    }
    field_28 |= 0x80;
    if (field_B0->field_2D) {
        field_B0->Stop();
        field_BC->Stop();
        play_keyboard_animation(field_B8, true);
        play_keyboard_animation(field_B4, true);
    }
    for (auto *quad : field_2C)
        quad->TurnOn(true);
    for (auto *quad : field_8C)
        quad->TurnOn(true);
    for (auto *quad : field_94)
        quad->TurnOn(true);
    field_A4->TurnOn(false);
    field_124->SetShown(true);
    field_C0->SetShown(false);
    field_124->SetNoFlash(color32{0xFFC8C8C8});
    field_124->SetScale(1.0f);
    field_124->SetText(static_cast<global_text_enum>(299));
    field_C0->SetNoFlash(color32{0xFFC8C8C8});
    field_C0->SetScale(1.5f);
    mString cursor{"_"};
    field_C0->SetTextNoLocalize(FEText::string{cursor});
    for (int i = 0; i < 4; ++i) {
        auto *text = static_cast<FEMultiLineText *>(field_128[i]);
        text->SetShown(true);
        text->SetNoFlash(color32{0xFFE6D03F});
        text->SetScale(0.8f);
        text->SetButtonColor(color32{0xFFFFFFFF});
        text->SetButtonScale(0.8f);
        int label = 304 + i;
        if (i == 3)
            label = static_cast<main_menu_memcard_check *>(field_140->field_4[2])->field_108 == 2 ? 302 : 301;
        text->SetText(static_cast<global_text_enum>(label));
    }
    field_E4 = get_msg(g_fileUSM, "DEFAULT_PLAYER");
    while (field_E4.size() < 8)
        field_E4 += " ";
    for (int i = 0; i < 8; ++i) {
        const char c = field_E4.c_str()[i];
        field_104[i] = c >= 'A' && c <= 'Z'   ? c - 'A'
                       : c >= '0' && c <= '9' ? c - 23
                       : c == ' '             ? static_cast<int>(field_F4.size()) - 1
                                              : 0;
        auto *text = field_C4[i];
        text->SetShown(true);
        text->SetNoFlash(color32{0xFFC8C8C8});
        text->SetScale(1.5f);
        mString letter{0, "%c", c};
        text->SetTextNoLocalize(FEText::string{letter});
    }
    field_138 = 0;
    field_C4[0]->SetScale(2.0f);
    field_A8->SetPosition(field_C4[0]->GetX(), field_A4->GetCenterY());
    play_keyboard_animation(field_AC, false);
    static string_hash drop_sound{"FE_PLYR_DROP"};
    [[maybe_unused]] auto sound = sub_60B960(drop_sound, 1.0f, 1.0f);
    field_13A = false;
    field_13B = false;
    field_13C = true;
    Input::instance->sub_8203F0(0, g_inputSettings4);
    g_cursor->sub_5A6790();
    g_cursor->sub_5A67D0(470, 375, 540, 410);
}

void main_menu_keyboard::Draw()
{
    if (field_AC->field_2D) {
        field_C4[field_138]->SetScale(2.0f);
        for (auto *text : field_128)
            text->SetScale(0.8f);
    }
    for (auto *quad : field_2C)
        quad->Draw();
    for (auto *quad : field_8C)
        quad->Draw();
    for (auto *quad : field_94)
        quad->Draw();
    field_A4->Draw();
    field_124->Draw();
    field_C0->Draw();
    reinterpret_cast<FEText *>(field_144)->Draw();
    for (auto *text : field_C4)
        text->Draw();
}

void main_menu_keyboard::Update(Float delta_time)
{
    FEMenu::Update(delta_time);
    const float progress = bit_cast<float>(field_AC->field_18) / field_AC->field_20;
    if ((field_13A || field_13B) && progress > 0.33f) {
        field_A4->TurnOn(false);
        field_A8->Stop();
    }
    auto *system = static_cast<FrontEndMenuSystem *>(field_140);
    if (field_13A && !field_AC->field_2D) {
        if (field_13D)
            system->BringUpDialogBox(19, FrontEndMenuSystem::fe_state{7}, FrontEndMenuSystem::fe_state{7});
        else
            system->GoNextState();
    } else if (field_13B && !field_AC->field_2D) {
        if (system->field_30 <= 6)
            --system->field_30;
        else {
            system->field_30 = 6;
            system->MakeActive(3);
        }
    }
    if (field_13C && progress > 0.66f) {
        field_A4->TurnOn(true);
        play_keyboard_animation(field_A8, true);
        field_13C = false;
    }
    if (field_AC->field_2D)
        reinterpret_cast<FEText *>(field_144)->SetX(field_124->GetX() + 320.0f);
}

void main_menu_keyboard::update_letter()
{
    field_E4.guts[field_138] = field_F4[field_104[field_138]];
    mString letter{0, "%c", field_E4.guts[field_138]};
    field_C4[field_138]->SetTextNoLocalize(FEText::string{letter});
}

void main_menu_keyboard::OnUp(int)
{
    if (field_AC->field_2D)
        return;
    static string_hash scroll{"FE_PF_UDScroll"};
    [[maybe_unused]] auto sound = sub_60B960(scroll, 1.0f, 1.0f);
    if (--field_104[field_138] < 0)
        field_104[field_138] = static_cast<int>(field_F4.size()) - 1;
    update_letter();
}

void main_menu_keyboard::OnDown(int)
{
    if (field_AC->field_2D)
        return;
    static string_hash scroll{"FE_PF_UDScroll"};
    [[maybe_unused]] auto sound = sub_60B960(scroll, 1.0f, 1.0f);
    if (++field_104[field_138] == static_cast<int>(field_F4.size()))
        field_104[field_138] = 0;
    update_letter();
}

void main_menu_keyboard::move_letter(int delta)
{
    if (field_AC->field_2D)
        return;
    static string_hash scroll{"FE_PF_LRScroll"};
    [[maybe_unused]] auto sound = sub_60B960(scroll, 1.0f, 1.0f);
    const int previous = field_138;
    field_138 = (field_138 + delta + 8) % 8;
    field_C0->SetX(field_C4[field_138]->GetX());
    field_A8->SetPosition(field_C4[field_138]->GetX(), field_A4->GetCenterY());
    field_C4[previous]->SetScale(1.5f);
    field_C4[field_138]->SetScale(2.0f);
}

void main_menu_keyboard::OnLeft(int)
{
    move_letter(-1);
}
void main_menu_keyboard::OnRight(int)
{
    move_letter(1);
}

void main_menu_keyboard::OnCross(int)
{
    if (field_AC->field_2D)
        return;
    static string_hash accept{"FE_PF_Accept"};
    [[maybe_unused]] auto sound = sub_60B960(accept, 1.0f, 1.0f);
    field_13D = true;
    for (int i = 0; i < 8; ++i)
        if (field_E4.guts[i] != ' ')
            field_13D = false;
    if (field_13D) {
        field_E4 = get_msg(g_fileUSM, "DEFAULT_PLAYER");
        while (field_E4.size() < 8)
            field_E4 += " ";
        field_13D = false;
    }
    std::strncpy(g_game_ptr->gamefile->field_4A8, field_E4.c_str(), 12);
    g_game_ptr->gamefile->field_4A8[11] = '\0';
    play_keyboard_animation(field_AC, false);
    field_AC->field_24 = 1;
    field_13A = true;
}

void main_menu_keyboard::OnTriangle(int)
{
    if (field_AC->field_2D || static_cast<main_menu_memcard_check *>(field_140->field_4[2])->field_108 != 2)
        return;
    play_keyboard_animation(field_AC, false);
    field_AC->field_24 = 1;
    static string_hash back{"FE_PF_Back"};
    [[maybe_unused]] auto sound = sub_60B960(back, 1.0f, 1.0f);
    field_13B = true;
}

void main_menu_keyboard::OnSquare(int controller)
{
    if (field_AC->field_2D)
        return;
    static string_hash clear{"FE_PF_Clear"};
    [[maybe_unused]] auto sound = sub_60B960(clear, 1.0f, 1.0f);
    for (int i = 0; i < 8; ++i) {
        field_104[i] = static_cast<int>(field_F4.size()) - 1;
        field_E4.guts[i] = field_F4[field_104[i]];
        mString letter{0, "%c", field_E4.guts[i]};
        field_C4[i]->SetTextNoLocalize(FEText::string{letter});
    }
    field_138 = 1;
    OnLeft(controller);
}

void main_menu_keyboard::OnCircle(int controller)
{
    if (field_AC->field_2D)
        return;
    static string_hash backspace{"FE_PF_Backspace"};
    [[maybe_unused]] auto sound = sub_60B960(backspace, 1.0f, 1.0f);
    field_104[field_138] = static_cast<int>(field_F4.size()) - 1;
    update_letter();
    if (field_138 > 0)
        OnLeft(controller);
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
    if constexpr (STANDALONE_SYSTEM) {
        Input::instance->sub_8203F0(0, g_inputSettingsMenu);
        field_28 &= ~0x80;
    } else {
        THISCALL(0x00613D10, this, a2);
    }
}

void main_menu_keyboard_patch()
{
    FUNC_ADDRESS(address, &main_menu_keyboard::OnActivate);
    set_vfunc(0x008957BC, address);
}
