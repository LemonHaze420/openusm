#include "pause_menu_root.h"

#include "comic_panels.h"
#include "common.h"
#include "config.h"
#include "entity_base.h"
#include "fe_health_widget.h"
#include "fetext.h"
#include "femanager.h"
#include "femenusystem.h"
#include "igofrontend.h"
#include "mstring.h"
#include "panelquad.h"
#include "pausemenusystem.h"
#include "panelfile.h"
#include "utility.h"
#include "trace.h"
#include "vtbl.h"
#include "wds.h"


#include "cursor.h"
#include "fe_menu_nav_bar.h"
#include "fileusm.h"
#include "mission_manager.h"
#include "mission_manager_script_data.h"
#include "panelanimfile.h"
#include "pause_menu_transition.h"
#include "ai_player_controller.h"
#include "actor.h"
#include "script.h"
#include "script_manager.h"
#include "femultilinetext.h"
#include "sound_instance_id.h"
#include "variables.h"
#include <cstring>

VALIDATE_SIZE(pause_menu_root, 0x100u);

pause_menu_root::pause_menu_root(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    this->m_vtbl = 0x00893F38;
    this->field_AC = a2;

    this->field_9C = nullptr;
    this->field_B0 = 0;
    this->field_B4 = 0;
    this->field_30 = 0;
    this->field_2C = 0;
    this->field_2D = 0;
}

void pause_menu_root::_Load()
{
    TRACE("pause_menu_root::Load");

    if constexpr (STANDALONE_SYSTEM) {
        auto *v2 = bit_cast<PauseMenuSystem *>(this->field_AC)->field_2C;

        this->field_3C[0] = v2->GetPQ("pm_splash_back_01a");
        this->field_3C[1] = v2->GetPQ("pm_splash_back_01b");
        this->field_3C[2] = v2->GetPQ("pm_splash_back_02a");
        this->field_3C[3] = v2->GetPQ("pm_splash_back_02b");
        this->field_3C[4] = v2->GetPQ("pm_splash_back_03a");
        this->field_3C[5] = v2->GetPQ("pm_splash_back_03b");
        this->field_3C[6] = v2->GetPQ("pm_splash_back_stub_01");
        this->field_3C[7] = v2->GetPQ("pm_splash_back_stub_02");
        this->field_3C[8] = v2->GetPQ("pm_splash_icon");

        this->field_68 = v2->GetPQ("pm_splash_dialog_box_01");
        this->field_6C = v2->GetPQ("pm_splash_dialog_box_02");
        this->field_60 = v2->GetPQ("pm_splash_hilite_text");
        this->field_64 = v2->GetPQ("pm_splash_hilite_text_01");
        this->field_70 = v2->GetPQ("pm_splash_back_04");
        this->field_74 = v2->GetPQ("pm_splash_back_venom");

        this->field_78[0] = v2->GetTextPointer("pm_splash_text_01");
        this->field_78[1] = v2->GetTextPointer("pm_splash_text_02");
        this->field_78[2] = v2->GetTextPointer("pm_splash_text_03");
        this->field_78[3] = v2->GetTextPointer("pm_splash_text_04");
        this->field_78[4] = v2->GetTextPointer("pm_splash_text_05");
        this->field_78[5] = v2->GetTextPointer("pm_splash_text_06");
        this->field_78[6] = v2->GetTextPointer("pm_splash_text_07");
        this->field_78[7] = v2->GetTextPointer("pm_splash_text_08");
        this->field_78[8] = v2->GetTextPointer("pm_splash_text_09");

        this->field_9C = v2->GetTextPointer("pm_splash_text_GAMEPAUSED");
        this->field_A0 = v2->GetTextPointer("pm_splash_dialog_box_text_BODY");
        this->field_A4 = v2->GetTextPointer("pm_splash_dialog_box_text_NOWAY");

        this->field_A8 = v2->GetTextPointer("pm_splash_dialog_box_text_OKAY");
        for (auto i = 0u; i < 9u; ++i) {
            this->field_3C[i]->TurnOn(true);
        }

        this->field_78[0]->SetShown(true);
        this->field_78[0]->SetNoFlash(color32{0xFFE6D03F});
        this->field_78[0]->SetScale(1.2, 1.2);

        for (auto i = 0u; i < 8u; ++i) {
            auto *v6 = this->field_78[i + 1];
            v6->SetShown(true);
            v6->SetNoFlash(color32{0xFFC87238});
        }

        this->field_68->TurnOn(1);
        this->field_6C->TurnOn(1);
        this->field_60->TurnOn(1);
        this->field_64->TurnOn(1);
        this->field_9C->SetShown(true);
        this->field_9C->SetText(static_cast<global_text_enum>(253));
        this->field_9C->SetNoFlash(color32{0xFFC8C8C8});
        this->field_A0->SetShown(1);
        this->field_A0->SetText(static_cast<global_text_enum>(271));
        this->field_A0->SetNoFlash(color32{0xFFC8C8C8});
        this->field_A4->SetShown(1);
        this->field_A4->SetText(static_cast<global_text_enum>(254));
        this->field_A4->SetNoFlash(color32{0xFFC87238});
        this->field_A8->SetShown(1);
        this->field_A8->SetText(static_cast<global_text_enum>(255));
        this->field_A8->SetNoFlash(color32{0xFFC87238});

        this->field_78[7]->SetText(static_cast<global_text_enum>(265));
        this->field_78[0]->SetText(static_cast<global_text_enum>(275));
        this->field_78[1]->SetText(static_cast<global_text_enum>(260));
        this->field_78[2]->SetText(static_cast<global_text_enum>(258));
        this->field_78[3]->SetText(static_cast<global_text_enum>(259));
        this->field_78[4]->SetText(static_cast<global_text_enum>(273));
        this->field_78[5]->SetText(static_cast<global_text_enum>(263));
        this->field_78[8]->SetText(static_cast<global_text_enum>(261));
        this->field_78[6]->SetText(static_cast<global_text_enum>(297));

        auto v8 = this->field_78[0]->GetX();
        auto v9 = this->field_78[0]->GetY();
        this->field_60->GetPos(this->field_B8, this->field_C8);
        this->field_64->GetPos(this->field_D8, this->field_E8);

        for (auto i = 0u; i < 4u; ++i) {
            this->field_B8[i] = this->field_B8[i] - v8;
            this->field_C8[i] = this->field_C8[i] - v9;
            this->field_D8[i] = this->field_D8[i] - v8;
            this->field_E8[i] = this->field_E8[i] - v9;
        }

        this->field_F8 = false;
    } else {
        THISCALL(0x0063B2E0, this);
    }
}

void pause_menu_root::OnUp(int a2)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0061BD00, this, a2);
        return;
    }
    if (byte_965C21 || field_30 || field_2C || field_F8)
        return;
    if (--field_B0 < 0)
        field_B0 = 9;
    if (field_B0 == 8 && mission_manager::s_inst->is_story_active())
        --field_B0;
    update_selected();
    (void)sub_60B960(string_hash{"FE_PS_UDScroll"}, 1.0f, 1.0f);
}

void pause_menu_root::OnDown(int a2)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0061BE10, this, a2);
        return;
    }
    if (byte_965C21 || field_30 || field_2C || field_F8)
        return;
    if (++field_B0 >= 10)
        field_B0 = 0;
    if (field_B0 == 8 && mission_manager::s_inst->is_story_active())
        ++field_B0;
    update_selected();
    (void)sub_60B960(string_hash{"FE_PS_UDScroll"}, 1.0f, 1.0f);
}

void sub_582AD0();

void pause_menu_root::reformat_nav_bar()
{
    auto *nav = static_cast<PauseMenuSystem *>(field_AC)->field_30;
    nav->Reset();
    nav->AddButtons({15}, {17}, static_cast<global_text_enum>(3));
    nav->Reformat();
    nav->text_box->SetTextNoLocalize(mString{get_msg(g_fileUSM, "RESUME")});
}

void pause_menu_root::OnActivate()
{
    reformat_nav_bar();
    field_28 |= 0x80;
    field_30 = 0;
    field_2C = false;
    highlighted = -1;
    field_FC = mission_manager::s_inst->is_story_active();
    auto *nav = static_cast<PauseMenuSystem *>(field_AC)->field_30;
    nav->text_box->SetNoFlash(color32{0xFFC8C8C8});
    nav->text_box->SetScale(1.0f, 1.0f);
    sub_582AD0();
    g_cursor->sub_5A6790();
    g_cursor->sub_5A67D0(305, 420, 385, 445);
}

void pause_menu_root::OnDeactivate([[maybe_unused]] FEMenu *next)
{
    field_28 &= ~0x80;
}

void pause_menu_root::update_selected()
{
    auto *nav = static_cast<PauseMenuSystem *>(field_AC)->field_30;
    auto *previous = field_B4 == 9 ? nav->text_box : field_78[field_B4];
    previous->SetNoFlash(color32{field_B4 == 9 ? 0xFFC8C8C8u : 0xFFC87238u});
    previous->SetScale(1.0f, 1.0f);
    auto *selected = field_B0 == 9 ? nav->text_box : field_78[field_B0];
    selected->SetNoFlash(color32{0xFFE6D03F});
    selected->SetScale(1.2f, 1.2f);
    field_B4 = field_B0;
}

void pause_menu_root::OnStart([[maybe_unused]] int controller)
{
    if (byte_965C21)
        return;
    auto *pause = static_cast<PauseMenuSystem *>(field_AC);
    pause->SetTransition(21);
    pause->MakeActive(1);
    if ((pause->field_38 & 0xFF) == 0)
        comic_panels::game_play_panel()->field_67 = false;
}

void pause_menu_root::OnTriangle(int controller)
{
    if (byte_965C21 || field_30 || field_2C)
        return;
    if (field_F8) {
        auto *pause = static_cast<PauseMenuSystem *>(field_AC);
        auto *transition = static_cast<pause_menu_transition *>(pause->field_4[1]);
        reinterpret_cast<PanelAnimFile *>(transition->field_44)->Start(true);
        field_F8 = false;
        reformat_nav_bar();
    } else {
        OnStart(controller);
    }
}

void pause_menu_root::OnLeft([[maybe_unused]] int controller)
{
    if (byte_965C21 || field_30 || field_2C || !field_F8 || field_F9)
        return;
    field_F9 = true;
    field_A8->SetNoFlash(color32{0xFFE6D03F});
    field_A8->SetScale(1.2f, 1.2f);
    field_A4->SetNoFlash(color32{0xFFC87238});
    field_A4->SetScale(1.0f, 1.0f);
    (void)sub_60B960(string_hash{"FE_WB_LRScroll"}, 1.0f, 1.0f);
}

void pause_menu_root::OnRight([[maybe_unused]] int controller)
{
    if (byte_965C21 || field_30 || field_2C || !field_F8 || !field_F9)
        return;
    field_F9 = false;
    field_A8->SetNoFlash(color32{0xFFC87238});
    field_A8->SetScale(1.0f, 1.0f);
    field_A4->SetNoFlash(color32{0xFFE6D03F});
    field_A4->SetScale(1.2f, 1.2f);
    (void)sub_60B960(string_hash{"FE_WB_LRScroll"}, 1.0f, 1.0f);
}

namespace {
int pause_mission_type()
{
    if (mission_manager::s_inst->m_script == nullptr)
        return -1;
    return static_cast<int>(
        *static_cast<float *>(script_manager::get_game_var_address(mString{"g_mission_type"}, nullptr, nullptr)));
}

bool pause_spidey_mission()
{
    const auto *name = mission_manager::s_inst->m_script->field_0.c_str();
    return std::strncmp(name, "s01_fathers_pride", 65535) != 0 && std::strncmp(name, "s02_workout", 65535) != 0;
}

bool pause_optional_mission()
{
    return pause_mission_type() == 1 &&
           std::strncmp(mission_manager::s_inst->m_script->field_0.c_str(), "lm_storm_races", 65535) != 0;
}
}

void pause_menu_root::OnCross(int controller)
{
    if (byte_965C21)
        return;
    if (field_B0 == 9) {
        OnTriangle(controller);
        return;
    }
    if (field_30 || field_2C)
        return;
    auto *pause = static_cast<PauseMenuSystem *>(field_AC);
    auto *transition = static_cast<pause_menu_transition *>(pause->field_4[1]);
    auto *dialog = reinterpret_cast<PanelAnimFile *>(transition->field_44);
    const bool active = mission_manager::s_inst->is_mission_active();
    const bool abortable = active && !pause_optional_mission() && pause_spidey_mission();
    const auto begin_transition = [&](int state) {
        pause->SetTransition(state);
        pause->MakeActive(1);
        if ((pause->field_38 & 0xFF) == 0)
            comic_panels::game_play_panel()->field_67 = false;
    };
    const auto body = [&](int text) {
        auto *box = static_cast<FEMultiLineText *>(field_A0);
        box->SetTextBox(static_cast<global_text_enum>(text), box->GetBoxWidth(), -1.0f);
    };
    const auto confirm = [&](int text, bool yes) {
        dialog->Start(false);
        field_F8 = true;
        reformat_nav_bar();
        field_F9 = yes;
        field_A8->SetNoFlash(color32{yes ? 0xFFE6D03Fu : 0xFFC87238u});
        field_A8->SetScale(yes ? 1.2f : 1.0f, yes ? 1.2f : 1.0f);
        field_A4->SetNoFlash(color32{yes ? 0xFFC87238u : 0xFFE6D03Fu});
        field_A4->SetScale(yes ? 1.0f : 1.2f, yes ? 1.0f : 1.2f);
        body(text);
    };
    if (!field_F8) {
        switch (field_B0) {
        case 0:
            if (pause_mission_type() == 4)
                return;
            begin_transition(3);
            break;
        case 1:
            if (active && !pause_spidey_mission())
                return;
            begin_transition(4);
            break;
        case 2:
            begin_transition(5);
            break;
        case 3:
            sub_5A6D70();
            break;
        case 4:
            begin_transition(7);
            break;
        case 5:
            if (!active) {
                pause->MakeActive(8);
                comic_panels::game_play_panel()->field_67 = true;
            } else {
                if (!pause_spidey_mission())
                    return;
                if (pause_optional_mission())
                    clear_missions_for_unlockables();
                else
                    confirm(270, false);
            }
            break;
        case 6:
            begin_transition(8);
            break;
        case 7:
            if (abortable)
                confirm(267, false);
            else {
                dword_922908 = 2;
                g_cursor->sub_5B0D70();
                g_cursor->field_120 = true;
                byte_922994 = true;
            }
            break;
        case 8:
            if (mission_manager::s_inst->is_story_active())
                return;
            confirm(static_cast<actor *>(g_world_ptr->get_hero_ptr(0))->get_player_controller()->m_hero_type ==
                            hero_type_enum::SPIDEY
                        ? 271
                        : 272,
                    true);
            break;
        default:
            return;
        }
        (void)sub_60B960(string_hash{"FE_PS_Accept"}, 1.0f, 1.0f);
        return;
    }
    if (field_F9) {
        if (field_B0 == 8 || (field_B0 == 7 && abortable)) {
            if (field_B0 == 8)
                field_34 = static_cast<actor *>(g_world_ptr->get_hero_ptr(0))->get_player_controller()->m_hero_type ==
                           hero_type_enum::SPIDEY;
            spawn_thread_for_func(string_hash{field_B0 == 8 ? "toggle_hero()" : "progression_mission_aborted()"},
                                  script::get_gsoi());
            script::exec_thread(false);
            begin_transition(21);
        } else if (field_B0 == 7) {
            body(269);
            byte_965BF8 = true;
            byte_922994 = true;
            dword_922908 = 1;
        } else if (field_B0 == 5) {
            clear_missions_for_unlockables();
        }
    }
    if (!field_2D) {
        dialog->Start(true);
        field_F8 = false;
        reformat_nav_bar();
    }
    (void)sub_60B960(string_hash{"FE_WB_Accept"}, 1.0f, 1.0f);
}

void pause_menu_root::OnWindowMessage(unsigned message, [[maybe_unused]] int wparam, [[maybe_unused]] int lparam)
{
    const auto hit = [&](FEText **texts, int count, int extent) {
        const auto point = g_cursor->field_104;
        for (int i = 0; i < count; ++i) {
            auto *text = texts[i];
            if (text->IsShown() && text->GetX() - extent < point.x && text->GetX() + extent > point.x &&
                text->GetY() - 10.0f < point.y && text->GetY() + 10.0f > point.y)
                return i;
        }
        return -1;
    };
    const auto nav_hit = [&] {
        const auto point = g_cursor->field_104;
        for (const auto &rect : g_cursor->field_12C)
            if (point.x > rect.left && point.x < rect.right && point.y > rect.top && point.y < rect.bottom)
                return true;
        return false;
    };
    FEText *answers[]{field_A8, field_A4};
    switch (message) {
    case WM_RBUTTONUP:
        OnTriangle(0);
        break;
    case WM_KEYUP:
        if (!var<bool>(0x00965C22))
            byte_922994 = false;
        break;
    case WM_MOUSEMOVE: {
        g_cursor->sub_581C60();
        if (field_F8) {
            const int answer = hit(answers, 2, 40);
            if (answer == 0)
                OnLeft(0);
            else if (answer == 1)
                OnRight(0);
        } else {
            const int selected = hit(field_78, 9 - field_FC, 200);
            if (selected != -1)
                field_B0 = selected;
            else if (nav_hit())
                field_B0 = 9;
            if (field_B0 != field_B4)
                update_selected();
        }
        break;
    }
    case WM_LBUTTONUP:
        if (field_F8) {
            if (hit(answers, 2, 40) != -1)
                OnCross(0);
        } else if (hit(field_78, 9 - field_FC, 200) != -1) {
            OnCross(0);
        } else if (nav_hit()) {
            OnStart(0);
        }
        break;
    }
}

void pause_menu_root::Draw()
{
    auto *pause = static_cast<PauseMenuSystem *>(field_AC);
    auto *transition = static_cast<pause_menu_transition *>(pause->field_4[1]);
    const bool dialog_animating = reinterpret_cast<PanelAnimFile *>(transition->field_44)->field_2D;
    if (!dialog_animating && field_F8) {
        field_A8->SetScale(field_F9 ? 1.2f : 1.0f, field_F9 ? 1.2f : 1.0f);
        field_A4->SetScale(field_F9 ? 1.0f : 1.2f, field_F9 ? 1.0f : 1.2f);
    }
    auto *selected = field_B0 == 9 ? pause->field_30->text_box : field_78[field_B0];
    selected->SetScale(1.2f, 1.2f);
    const float x = selected->GetX();
    const float y = selected->GetY();
    float xs[4], ys[4];
    for (int i = 0; i < 4; ++i) {
        xs[i] = x + field_B8[i];
        ys[i] = y + field_C8[i];
    }
    field_60->SetPos(xs, ys);
    for (int i = 0; i < 4; ++i) {
        xs[i] = x + field_D8[i];
        ys[i] = y + field_E8[i];
    }
    field_64->SetPos(xs, ys);
    const bool mission_active = mission_manager::s_inst->is_mission_active();
    const bool story_mission = pause_mission_type() == 4;
    const bool spidey = mission_active && pause_spidey_mission();
    field_78[7]->SetText(
        static_cast<global_text_enum>(mission_active && !pause_optional_mission() && spidey ? 265 : 266));
    const auto style = [](FEText *text, bool disabled, bool high) {
        text->SetNoFlash(color32{disabled ? 0xFF808080u : high ? 0xFFE6D03Fu : 0xFFC87238u});
        text->SetScale(!disabled && high ? 1.2f : 1.0f, !disabled && high ? 1.2f : 1.0f);
    };
    style(field_78[0], story_mission, field_B0 == 0);
    style(field_78[5], story_mission && !spidey, field_B0 == 5);
    style(field_78[1], story_mission && !spidey, field_B0 == 1);
    if (globalTextLanguage == 2)
        field_78[7]->SetScale(field_B0 == 7 ? 1.1f : 0.9f, field_B0 == 7 ? 1.2f : 1.0f);
    for (auto *quad : field_3C)
        quad->Draw();
    const bool venom = field_30
                           ? field_34 == 1
                           : static_cast<actor *>(g_world_ptr->get_hero_ptr(0))->get_player_controller()->m_hero_type ==
                                 hero_type_enum::VENOM;
    (venom ? field_74 : field_70)->Draw();
    const bool story_active = mission_manager::s_inst->is_story_active();
    for (int i = 0; i < 9; ++i)
        if (i != 8 || !story_active)
            field_78[i]->Draw();
    if (field_B0 != 9) {
        field_60->Draw();
        field_64->Draw();
    }
    field_9C->Draw();
    if (field_F8 || dialog_animating) {
        field_68->Draw();
        field_6C->Draw();
        field_A0->Draw();
        if (!field_2D) {
            field_A4->Draw();
            field_A8->Draw();
        }
    }
    auto *nav = pause->field_30;
    if (nav->field_28)
        nav->text_box->SetScale(0.8f, 1.0f);
    nav->text_box->Draw();
    nav->background_a->Draw();
    if (nav->field_1C != nullptr)
        nav->field_1C->Draw();
    if (nav->field_20 != nullptr)
        nav->field_20->Draw();
    if (nav->field_24 != nullptr)
        nav->field_24->Draw();
}

void pause_menu_root::clear_missions_for_unlockables()
{
    spawn_thread_for_func(string_hash{"clear_missions_for_unlockables()"}, script::get_gsoi());
    script::exec_thread(false);
    static_cast<PauseMenuSystem *>(field_AC)->Deactivate();
    mission_manager::s_inst->lock();
}

void sub_648F40()
{
    CDECL_CALL(0x00648F40);
}

void pause_menu_root::Update(Float a2)
{
    if constexpr (1) {
        if (this->field_2D) {
            sub_648F40();
        }

        FEMenu::Update(a2);
        if (this->field_30) {
            this->update_switching_heroes();
        }

        if (this->field_2C) {
            if (!mission_stack_manager::s_inst->waiting_for_push_or_pop()) {
                auto *v3 = this->field_AC;
                this->field_2C = false;

                v3->MakeActive(8);

                comic_panels::game_play_panel()->field_67 = true;
            }
        }
    } else {
        THISCALL(0x006490A0, this, a2);
    }
}

void pause_menu_root::update_switching_heroes()
{
    int v2 = this->field_30;
    if (v2 == 4) {
        g_world_ptr->remove_player(g_world_ptr->num_players - 1);
    } else if (v2 == 2) {
        int v3;
        if (this->field_34) {
            g_world_ptr->add_player(mString{"venom"});

            v3 = 4;
        } else {
            g_world_ptr->add_player(mString{"ultimate_spiderman"});

            v3 = 0;
        }

        auto *v4 = g_femanager.IGO->m_hero_health;
        if (v4->panels[v3] != nullptr) {
            v4->field_30 = g_world_ptr->get_hero_ptr(0)->my_handle.field_0;
            v4->field_38 = v3;
            v4->UpdateMasking();
            v4->clear_bars();
        }

        v4->SetShown(this->field_38);
    }

    --this->field_30;
}

void pause_menu_root_patch()
{
    {
        FUNC_ADDRESS(address, &pause_menu_root::_Load);
        set_vfunc(0x00893F48, address);
    }
    return;

    {
        FUNC_ADDRESS(address, &pause_menu_root::Update);
        set_vfunc(0x00893F58, address);
    }

    {
        FUNC_ADDRESS(address, &pause_menu_root::OnUp);
        //set_vfunc(0x00893F74, address);
    }
}
