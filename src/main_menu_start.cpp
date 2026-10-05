#include "main_menu_start.h"

#include "common.h"
#include "fetext.h"
#include "femultilinetext.h"
#include "game.h"
#include "frontendmenusystem.h"
#include "panelanim.h"
#include "panelanimfile.h"
#include "panelfile.h"
#include "panelquadsection.h"
#include "panelquad.h"

#include <algorithm>

VALIDATE_SIZE(main_menu_start, 0x130);
VALIDATE_OFFSET(main_menu_start, quads, 0x2C);
VALIDATE_OFFSET(main_menu_start, animations, 0xC8);
VALIDATE_OFFSET(main_menu_start, press_start, 0x118);
VALIDATE_OFFSET(main_menu_start, checking, 0x11C);

main_menu_start::main_menu_start(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    m_vtbl = 0x00894648;
    field_128 = false;
    field_12C = a2;
    field_120 = 0.0f;
    field_124 = 0.0f;
    field_12A = 0;
}

void main_menu_start::_Init()
{
    auto *front_end = static_cast<FrontEndMenuSystem *>(field_12C);
    assert(front_end != nullptr && front_end->field_7C != nullptr);
    auto *panel = front_end->field_7C;

    struct Binding {
        unsigned index;
        const char *name;
    };
    static constexpr Binding quads[]{
        {1, "mm_bkg_city"},
        {8, "mm_bkg_detail_01"},
        {4, "mm_bkg_detail_02"},
        {9, "mm_bkg_grey_a_01"},
        {5, "mm_bkg_grey_a_02"},
        {0, "mm_logo_main"},
        {2, "mm_bkg_city_01"},
        {3, "mm_bkg_city_02"},
        {10, "mm_bkg_grey_a_03"},
        {6, "mm_bkg_grey_a_04"},
        {12, "mm_bkg_grey_a_05"},
        {11, "mm_bkg_grey_a_06"},
        {7, "mm_bkg_grey_a_07"},
        {13, "mm_bkg_grey_a_08"},
        {14, "mm_bkg_grey_a_09"},
        {15, "mm_pre_main_back_black"},
        {16, "mm_pre_main_screen_01"},
        {17, "mm_pre_main_screen_02"},
        {18, "mm_pre_main_screen_03"},
        {19, "mm_pre_main_screen_04"},
        {20, "mm_pre_main_spider_icon"},
        {21, "mm_bkg_white_01"},
        {22, "mm_bkg_detail_light_01"},
        {23, "mm_bkg_detail_light_02"},
        {24, "mm_bkg_detail_light_03"},
        {25, "mm_bkg_detail_light_04"},
        {26, "mm_bkg_detail_light_05"},
        {27, "mm_bkg_detail_light_06"},
        {28, "mm_bkg_detail_dark_01"},
        {29, "mm_bkg_detail_dark_02"},
        {30, "mm_bkg_detail_dark_03"},
        {31, "mm_bkg_detail_dark_04"},
        {32, "mm_bkg_detail_dark_05"},
        {33, "mm_bkg_detail_dark_06"},
        {34, "mm_mainmenu_box_hilite_a"},
        {35, "mm_mainmenu_box_hilite_b"},
        {36, "mm_loading_bar_01"},
        {37, "mm_loading_bar_02"},
        {38, "mm_loading_bar_gauge"},
    };
    for (const auto &binding : quads) {
        auto *quad = panel->GetPQ(binding.name);
        assert(quad != nullptr);
        quad->TurnOn(false);
        this->quads[binding.index] = quad;
    }

    press_start = panel->GetTextPointer("mm_mainmenu_text_PRESSSTART");
    checking = static_cast<FEMultiLineText *>(panel->GetTextPointer("mm_mainmenu_text_CHECKING"));
    if (press_start != nullptr)
        press_start->SetShown(false);
    if (checking != nullptr)
        checking->SetShown(false);

    static constexpr int animation_indices[]{
        5, 17, 6, 15, 14, 3, 0, 1, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    };
    static constexpr unsigned animation_slots[]{
        39, 40, 41, 42, 43, 44, 45, 46, 53, 54, 55, 56, 57, 58, 47, 48, 49, 50, 51, 52,
    };
    for (unsigned i = 0; i < 20; ++i)
        animations[animation_slots[i] - 39] = panel->GetAnimationPointer(animation_indices[i]);
}

void main_menu_start::_OnActivate()
{
    if (press_start != nullptr) {
        press_start->SetText(static_cast<global_text_enum>(294));
        press_start->SetNoFlash(color32{0xFFC8C8C8u});
        press_start->SetScale(1.0f);
        press_start->SetShown(false);
    }
    if (checking != nullptr) {
        checking->SetTextBox(static_cast<global_text_enum>(293), checking->field_7C, -1.0f);
        checking->SetNoFlash(color32{0xFFB42828u});
        checking->SetShown(false);
    }

    for (unsigned i = 0; i < 34; ++i)
        quads[i]->TurnOn(true);
    for (unsigned i = 34; i < 39; ++i)
        quads[i]->TurnOn(false);
    quads[21]->TurnOn(false);

    auto *animation = animations[0];
    if (animation != nullptr) {
        for (int i = 0; i < animation->field_0.size(); ++i) {
            auto *target = animation->field_0.m_data[i]->field_14;
            if (target != nullptr)
                target->StartAnim(true);
        }
        animation->field_18 = bit_cast<int>(0.0f);
        animation->field_1C = bit_cast<int>(0.0f);
        animation->field_28 = 0;
        animation->field_20 = animation->field_14;
        animation->field_2C = false;
        animation->field_2D = true;
        animation->field_24 = 0;
    }

    field_12A = 0;
    field_124 = 0.0f;
    field_120 = 0.0f;
}

namespace {
void start_panel_animation(PanelAnimFile *animation, bool reverse, bool loop)
{
    if (animation == nullptr)
        return;

    for (int i = 0; i < animation->field_0.size(); ++i) {
        auto *target = animation->field_0.m_data[i]->field_14;
        if (target != nullptr)
            target->StartAnim(true);
    }

    animation->field_18 = bit_cast<int>(0.0f);
    animation->field_1C = bit_cast<int>(0.0f);
    animation->field_20 = animation->field_14;
    animation->field_24 = reverse ? 1 : 0;
    animation->field_28 = loop ? 1 : 0;
    animation->field_2C = false;
    animation->field_2D = true;
}
}  // namespace

bool main_menu_start::IsIdle() const
{
    for (unsigned index = 0; index < 4; ++index) {
        auto *animation = animations[index];
        if (animation != nullptr && animation->field_2D)
            return false;
    }
    return true;
}

void main_menu_start::Update(Float delta_time)
{
    auto *front_end = static_cast<FrontEndMenuSystem *>(field_12C);
    if (front_end->field_30 == 3 && field_12A >= 2) {
        const bool load_completed = g_game_ptr->level.load_completed;
        field_120 = std::min(1.0f, field_120 + (load_completed ? 0.02f : 0.002f));
        if (load_completed && field_120 >= 0.99f) {
            field_12A = 4;
            front_end->GoNextState();
        }
    } else if (front_end->field_30 == 4) {
        field_124 = std::min(1.0f, field_124 + float(delta_time));
        field_128 = false;
    }

    if (field_12A == 0 && IsIdle()) {
        for (unsigned slot = 12; slot <= 20; ++slot)
            quads[slot]->TurnOn(false);
        quads[21]->TurnOn(true);
        start_panel_animation(animations[1], false, false);
        for (unsigned index = 5; index < 20; ++index)
            start_panel_animation(animations[index], false, true);
        field_12A = 1;
    } else if (field_12A == 1 && IsIdle()) {
        field_12A = front_end->field_30 == 4 ? 4 : 3;
        if (checking != nullptr)
            checking->SetShown(true);
        for (unsigned slot = 36; slot <= 38; ++slot)
            quads[slot]->TurnOn(true);
        quads[21]->TurnOn(false);
        start_panel_animation(animations[2], true, false);
    } else if (field_12A == 4 && IsIdle()) {
        start_panel_animation(animations[2], false, false);
        field_12A = 5;
    } else if (field_12A == 5 && IsIdle()) {
        if (checking != nullptr)
            checking->SetShown(false);
        for (unsigned slot = 36; slot <= 38; ++slot)
            quads[slot]->TurnOn(false);
        if (press_start != nullptr)
            press_start->SetShown(true);
        quads[34]->TurnOn(true);
        quads[35]->TurnOn(true);
        start_panel_animation(animations[3], false, false);
        field_12A = 6;
    } else if (field_12A == 6 && IsIdle()) {
        start_panel_animation(animations[4], false, true);
        field_12A = 7;
    }

    FEMenu::Update(delta_time);
}

void main_menu_start::Draw()
{
    for (unsigned slot = 0; slot < 34; ++slot)
        quads[slot]->Draw();

    quads[38]->Mask(std::clamp(field_120, 0.0f, 1.0f), 2, -1.0f);
    for (unsigned slot = 36; slot < 39; ++slot)
        quads[slot]->Draw();

    checking->Draw();
    quads[34]->Draw();
    quads[35]->Draw();
    press_start->Draw();
}

void main_menu_start::_OnDeactivate()
{
    auto *animation = animations[4];
    if (animation != nullptr)
        animation->Stop();
}

void main_menu_start::OnStart(int)
{
    auto *front_end = static_cast<FrontEndMenuSystem *>(field_12C);
    if (front_end->field_30 == 4 && field_12A == 7 && !field_128)
        front_end->GoNextState();
}

void main_menu_start::OnCross(int controller)
{
    OnStart(controller);
}
