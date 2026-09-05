#include "main_menu_start.h"

#include "common.h"
#include "fetext.h"
#include "game.h"
#include "frontendmenusystem.h"
#include "panelanim.h"
#include "panelanimfile.h"
#include "panelfile.h"
#include "panelquadsection.h"
#include "panelquad.h"

#include <algorithm>

VALIDATE_SIZE(main_menu_start, 0x130);

main_menu_start::main_menu_start(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0)
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
    static constexpr Binding quads[] {
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
        field_2C[binding.index] = reinterpret_cast<int>(quad);
    }

    auto *press_start = panel->GetTextPointer("mm_mainmenu_text_PRESSSTART");
    auto *checking = panel->GetTextPointer("mm_mainmenu_text_CHECKING");
    if (press_start != nullptr)
        press_start->SetShown(false);
    if (checking != nullptr)
        checking->SetShown(false);
    field_2C[59] = reinterpret_cast<int>(press_start);
    field_2C[60] = reinterpret_cast<int>(checking);

    static constexpr int animation_indices[] {
        5, 17, 6, 15, 14, 3, 0, 1,
        19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30,
    };
    static constexpr unsigned animation_slots[] {
        39, 40, 41, 42, 43, 44, 45, 46,
        53, 54, 55, 56, 57, 58,
        47, 48, 49, 50, 51, 52,
    };
    for (unsigned i = 0; i < 20; ++i)
        field_2C[animation_slots[i]] =
            reinterpret_cast<int>(panel->GetAnimationPointer(animation_indices[i]));
}

void main_menu_start::_OnActivate()
{
    auto *press_start = reinterpret_cast<FEText *>(field_2C[59]);
    auto *checking = reinterpret_cast<FEText *>(field_2C[60]);
    if (press_start != nullptr) {
        press_start->SetText(static_cast<global_text_enum>(294));
        press_start->SetNoFlash(color32 {0xFFB42828});
        press_start->SetScale(1.0f);
        press_start->SetShown(false);
    }
    if (checking != nullptr)
        checking->SetShown(false);

    for (unsigned i = 0; i < 34; ++i)
        reinterpret_cast<PanelQuad *>(field_2C[i])->TurnOn(true);
    for (unsigned i = 34; i < 39; ++i)
        reinterpret_cast<PanelQuad *>(field_2C[i])->TurnOn(false);
    reinterpret_cast<PanelQuad *>(field_2C[21])->TurnOn(false);

    auto *animation = reinterpret_cast<PanelAnimFile *>(field_2C[39]);
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
}

bool main_menu_start::IsIdle() const
{
    for (unsigned slot = 39; slot <= 42; ++slot) {
        auto *animation = reinterpret_cast<PanelAnimFile *>(field_2C[slot]);
        if (animation != nullptr && animation->field_2D)
            return false;
    }
    return true;
}

void main_menu_start::Update(Float delta_time)
{
    auto *front_end = static_cast<FrontEndMenuSystem *>(field_12C);
    if (front_end->field_30 == 3 && field_12A >= 2) {
        field_120 = std::min(1.0f, field_120 + float(delta_time));
        if (g_game_ptr->level.load_completed && field_120 >= 0.99f) {
            field_12A = 4;
            front_end->GoNextState();
        }
    } else if (front_end->field_30 == 4) {
        field_124 = std::min(1.0f, field_124 + float(delta_time));
        field_128 = false;
    }

    if (field_12A == 0 && IsIdle()) {
        for (unsigned slot = 12; slot <= 20; ++slot)
            reinterpret_cast<PanelQuad *>(field_2C[slot])->TurnOn(false);
        reinterpret_cast<PanelQuad *>(field_2C[21])->TurnOn(true);
        start_panel_animation(reinterpret_cast<PanelAnimFile *>(field_2C[40]), false, false);
        for (unsigned slot = 44; slot <= 58; ++slot)
            start_panel_animation(reinterpret_cast<PanelAnimFile *>(field_2C[slot]), false, false);
        field_12A = 1;
    } else if (field_12A == 1 && IsIdle()) {
        field_12A = front_end->field_30 == 4 ? 4 : 3;
        auto *checking = reinterpret_cast<FEText *>(field_2C[60]);
        if (checking != nullptr)
            checking->SetShown(true);
        for (unsigned slot = 36; slot <= 38; ++slot)
            reinterpret_cast<PanelQuad *>(field_2C[slot])->TurnOn(true);
        reinterpret_cast<PanelQuad *>(field_2C[21])->TurnOn(false);
        start_panel_animation(reinterpret_cast<PanelAnimFile *>(field_2C[41]), true, false);
    } else if (field_12A == 4 && IsIdle()) {
        start_panel_animation(reinterpret_cast<PanelAnimFile *>(field_2C[41]), false, false);
        field_12A = 5;
    } else if (field_12A == 5 && IsIdle()) {
        auto *checking = reinterpret_cast<FEText *>(field_2C[60]);
        if (checking != nullptr)
            checking->SetShown(false);
        for (unsigned slot = 36; slot <= 38; ++slot)
            reinterpret_cast<PanelQuad *>(field_2C[slot])->TurnOn(false);
        auto *press_start = reinterpret_cast<FEText *>(field_2C[59]);
        if (press_start != nullptr)
            press_start->SetShown(true);
        reinterpret_cast<PanelQuad *>(field_2C[34])->TurnOn(true);
        reinterpret_cast<PanelQuad *>(field_2C[35])->TurnOn(true);
        start_panel_animation(reinterpret_cast<PanelAnimFile *>(field_2C[42]), false, false);
        field_12A = 6;
    } else if (field_12A == 6 && IsIdle()) {
        start_panel_animation(reinterpret_cast<PanelAnimFile *>(field_2C[43]), false, true);
        field_12A = 7;
    }

    FEMenu::Update(delta_time);
}

void main_menu_start::_OnDeactivate()
{
    auto *animation = reinterpret_cast<PanelAnimFile *>(field_2C[43]);
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
