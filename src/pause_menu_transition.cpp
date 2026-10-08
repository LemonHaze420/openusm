#include "pause_menu_transition.h"

#include "common.h"
#include "config.h"
#include "func_wrapper.h"
#include "panelfile.h"
#include "pausemenusystem.h"
#include "trace.h"
#include "utility.h"
#include "panelanimfile.h"
#include "comic_panels.h"
#include "fe_menu_nav_bar.h"
#include "pause_menu_status.h"
#include "sound_instance_id.h"
#include "fetext.h"

VALIDATE_SIZE(pause_menu_transition, 0x50u);

pause_menu_transition::pause_menu_transition(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 0, 0)
{
    this->m_vtbl = 0x00893FE8;

    this->field_2C = bit_cast<PauseMenuSystem *>(a2);

    this->field_30 = 0;
    this->field_34 = 0;
    this->field_38 = 0;
    this->field_3C = 0;
    this->field_40 = 0;
    this->field_48 = 21;
}

void pause_menu_transition::set_transition(int a1)
{
    this->field_48 = a1;
}

void pause_menu_transition::Update([[maybe_unused]] Float a2)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0061C680, this, a2);
        return;
    }
    auto *splash = reinterpret_cast<PanelAnimFile *>(field_30);
    auto *content = reinterpret_cast<PanelAnimFile *>(field_34);
    auto *header = reinterpret_cast<PanelAnimFile *>(field_38);
    auto *detail = reinterpret_cast<PanelAnimFile *>(field_3C);
    auto *controller = reinterpret_cast<PanelAnimFile *>(field_40);
    const auto activate = [&](int index, bool show_comic) {
        field_2C->MakeActive(index);
        if (show_comic || (field_2C->field_38 & 0xFF) == 0)
            comic_panels::game_play_panel()->field_67 = show_comic;
    };
    switch (field_48) {
    case 0:
        if (header->field_2D)
            header->Stop();
        if (!splash->field_2D)
            activate(2, false);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        if (!content->field_2D && !header->field_2D) {
            header->Start(false);
            detail->Start(false, true);
        }
        if (!splash->field_2D) {
            constexpr int destinations[]{3, 3, 3, 6, 6, 4, 7, 9};
            activate(destinations[field_48 - 1], true);
        }
        break;
    case 9:
    case 10:
        if (!controller->field_2D)
            activate(field_48 == 9 ? 5 : 4, true);
        break;
    case 11:
    case 17:
    case 18:
    case 19:
    case 20:
        if (!content->field_2D && !header->field_2D) {
            if ((field_4C & 0xFF) != 0) {
                field_2C->Deactivate();
            } else {
                content->Start(true);
                detail->Stop();
                field_4C = (field_4C & ~0xFF) | 1;
            }
        }
        break;
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        if (!content->field_2D && !header->field_2D) {
            content->Start(true);
            detail->Stop();
        }
        if (!splash->field_2D)
            activate(2, false);
        break;
    case 21:
        if (!splash->field_2D)
            field_2C->Deactivate();
        break;
    }
}

void pause_menu_transition::OnActivate()
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x006392B0, this);
        return;
    }
    auto *splash = reinterpret_cast<PanelAnimFile *>(field_30);
    auto *content = reinterpret_cast<PanelAnimFile *>(field_34);
    auto *header = reinterpret_cast<PanelAnimFile *>(field_38);
    auto *controller = reinterpret_cast<PanelAnimFile *>(field_40);
    switch (field_48) {
    case 0:
        splash->Start(false);
        field_2C->field_2C->Update(0.001f);
        header->Start(false);
        field_2C->field_2C->Update(0.001f);
        (void)sub_60B960(string_hash{"FE_PS_IN"}, 1.0f, 1.0f);
        break;
    case 1:
    case 2:
    case 3: {
        splash->Start(true);
        content->Start(false);
        auto *status = static_cast<pause_menu_status *>(field_2C->field_4[3]);
        status->SetContentType(field_48 - 1);
        status->OnActivate();
        break;
    }
    case 4:
    case 5:
        splash->Start(true);
        content->Start(false);
        if (field_48 == 4)
            THISCALL(0x00638380, field_2C);
        else
            THISCALL(0x00636050, field_2C->field_4[6]);
        field_2C->field_4[6]->OnActivate();
        break;
    case 6:
    case 7:
    case 8:
        splash->Start(true);
        content->Start(false);
        field_2C->field_4[field_48 == 6 ? 4 : field_48 == 7 ? 7 : 9]->OnActivate();
        break;
    case 9:
    case 10:
        controller->Start(field_48 == 10);
        break;
    case 11:
    case 17:
    case 18:
    case 19:
    case 20:
        sub_582A30();
        header->Start(true);
        field_4C &= ~0xFF;
        (void)sub_60B960(string_hash{"FE_PS_OUT"}, 1.0f, 1.0f);
        break;
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        splash->Start(false);
        header->Start(true);
        break;
    case 21:
        sub_582A30();
        splash->Start(true);
        (void)sub_60B960(string_hash{"FE_PS_OUT"}, 1.0f, 1.0f);
        break;
    }
    auto *nav = field_2C->field_30;
    nav->Reset();
    nav->Reformat();
}

void pause_menu_transition::Draw()
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0062A8E0, this);
        return;
    }
    const auto draw = [this](int index) {
        field_2C->field_4[index]->Draw();
    };
    switch (field_48) {
    case 0:
    case 21:
        draw(2);
        break;
    case 1:
    case 2:
    case 3:
    case 12:
        draw(2);
        draw(3);
        break;
    case 4:
    case 5:
    case 13:
        draw(2);
        draw(6);
        break;
    case 6:
    case 14:
        draw(2);
        draw(4);
        break;
    case 7:
    case 15:
        draw(2);
        draw(7);
        break;
    case 8:
    case 16:
        draw(2);
        draw(9);
        break;
    case 9:
    case 10:
        draw(5);
        draw(4);
        break;
    case 11:
        draw(5);
        break;
    case 17:
        draw(3);
        break;
    case 18:
        draw(6);
        break;
    case 19:
        draw(4);
        break;
    case 20:
        draw(7);
        break;
    }
}

void pause_menu_transition::_Load()
{
    TRACE("pause_menu_transition::Load");
    if constexpr (STANDALONE_SYSTEM) {
        auto *panel = field_2C->field_2C;
        field_30 = reinterpret_cast<std::intptr_t>(panel->GetAnimationPointer(0));
        field_34 = reinterpret_cast<std::intptr_t>(panel->GetAnimationPointer(1));
        field_38 = reinterpret_cast<std::intptr_t>(panel->GetAnimationPointer(2));
        field_3C = reinterpret_cast<std::intptr_t>(panel->GetAnimationPointer(3));
        field_40 = reinterpret_cast<std::intptr_t>(panel->GetAnimationPointer(4));
        field_44 = reinterpret_cast<std::intptr_t>(panel->GetAnimationPointer(5));
    } else {
        THISCALL(0x0061C640, this);
    }
}

void pause_menu_transition_patch()
{
    {
        FUNC_ADDRESS(address, &pause_menu_transition::_Load);
        set_vfunc(0x00893FF8, address);
    }
}
