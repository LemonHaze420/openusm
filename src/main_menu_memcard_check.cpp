#include "main_menu_memcard_check.h"

#include "common.h"
#include "frontendmenusystem.h"

#include "input_mgr.h"
#include "cursor.h"
#include "femultilinetext.h"
#include "game.h"
#include "game_settings.h"
#include "localized_string_table.h"
#include "main_menu_load.h"
#include "panelfile.h"
#include "memoryunitmanager.h"
#include "os_developer_options.h"
#include "panelanimfile.h"
#include "panelanim.h"
#include "panelquad.h"
#include "mstring.h"

#include <algorithm>
#include <iterator>

VALIDATE_SIZE(main_menu_memcard_check, 0x1A0u);
VALIDATE_OFFSET(main_menu_memcard_check, field_30, 0x30);
VALIDATE_OFFSET(main_menu_memcard_check, field_9C, 0x9C);
VALIDATE_OFFSET(main_menu_memcard_check, field_AC, 0xAC);
VALIDATE_OFFSET(main_menu_memcard_check, field_B8, 0xB8);
VALIDATE_OFFSET(main_menu_memcard_check, field_D4, 0xD4);
VALIDATE_OFFSET(main_menu_memcard_check, field_EC, 0xEC);
VALIDATE_OFFSET(main_menu_memcard_check, field_FC, 0xFC);
VALIDATE_OFFSET(main_menu_memcard_check, field_100, 0x100);
VALIDATE_OFFSET(main_menu_memcard_check, field_104, 0x104);
VALIDATE_OFFSET(main_menu_memcard_check, field_108, 0x108);
VALIDATE_OFFSET(main_menu_memcard_check, field_10C, 0x10C);
VALIDATE_OFFSET(main_menu_memcard_check, field_120, 0x120);
VALIDATE_OFFSET(main_menu_memcard_check, field_12C, 0x12C);
VALIDATE_OFFSET(main_menu_memcard_check, field_19C, 0x19C);
namespace {

void __stdcall insert_remove_callback(MemoryUnitManager::InsertRemoveObserver *, int) {}

MemoryUnitManager::InsertRemoveObserverVTable insert_remove_observer_vtable{
    &insert_remove_callback,
};
}  // namespace


main_menu_memcard_check::main_menu_memcard_check(FEMenuSystem *a2, int a4, int a5) : FEMenu(a2, 0, a4, a5, 8, 0)
{
    m_vtbl = 0x00895910;
    field_2C.m_vtbl = &insert_remove_observer_vtable;
    std::fill(std::begin(field_30), std::end(field_30), nullptr);
    std::fill(std::begin(field_9C), std::end(field_9C), nullptr);
    std::fill(std::begin(field_AC), std::end(field_AC), nullptr);
    field_B8 = nullptr;
    field_BC = nullptr;
    field_C0 = nullptr;
    field_C4 = nullptr;
    field_C8 = nullptr;
    field_CC = nullptr;
    field_D0 = nullptr;
    std::fill(std::begin(field_D4), std::end(field_D4), nullptr);
    std::fill(std::begin(field_E0), std::end(field_E0), nullptr);
    field_EC = nullptr;
    field_F0 = nullptr;
    field_F4 = nullptr;
    field_F8 = false;
    field_FC = 0.0f;
    field_100 = 0;
    field_104 = 0;
    field_108 = DIALOG_NONE;
    field_10C.m_data = nullptr;
    field_10C.m_max_size = 0;
    field_11C = 0;
    field_120 = false;
    field_128 = true;
    field_129 = false;
    field_19C = static_cast<FrontEndMenuSystem *>(a2);
}

void main_menu_memcard_check::_Init()
{
    assert(field_19C != nullptr && field_19C->field_7C != nullptr);
    auto *panel = field_19C->field_7C;

    struct QuadBinding {
        PanelQuad **destination;
        const char *name;
    };
    const QuadBinding quads[]{
        {&field_30[0], "mm_bkg_city"},
        {&field_30[1], "mm_bkg_city_01"},
        {&field_30[2], "mm_bkg_city_02"},
        {&field_30[7], "mm_bkg_detail_01"},
        {&field_30[3], "mm_bkg_detail_02"},
        {&field_30[8], "mm_bkg_grey_a_01"},
        {&field_30[4], "mm_bkg_grey_a_02"},
        {&field_30[9], "mm_bkg_grey_a_03"},
        {&field_30[5], "mm_bkg_grey_a_04"},
        {&field_30[10], "mm_bkg_grey_a_06"},
        {&field_30[6], "mm_bkg_grey_a_07"},
        {&field_30[25], "mm_logo_medium"},
        {&field_30[26], "mm_logo_main"},
        {&field_30[23], "mm_bkg_white_02"},
        {&field_30[24], "mm_bkg_white_03"},
        {&field_30[11], "mm_bkg_detail_light_01"},
        {&field_30[12], "mm_bkg_detail_light_02"},
        {&field_30[13], "mm_bkg_detail_light_03"},
        {&field_30[14], "mm_bkg_detail_light_04"},
        {&field_30[15], "mm_bkg_detail_light_05"},
        {&field_30[16], "mm_bkg_detail_light_06"},
        {&field_30[17], "mm_bkg_detail_dark_01"},
        {&field_30[18], "mm_bkg_detail_dark_02"},
        {&field_30[19], "mm_bkg_detail_dark_03"},
        {&field_30[20], "mm_bkg_detail_dark_04"},
        {&field_30[21], "mm_bkg_detail_dark_05"},
        {&field_30[22], "mm_bkg_detail_dark_06"},
        {&field_9C[0], "mm_textbox_frame"},
        {&field_9C[1], "mm_textbox_gradient"},
        {&field_9C[2], "mm_textbox_outline"},
        {&field_9C[3], "mm_textbox_spider"},
        {&field_AC[0], "mm_loading_bar_01"},
        {&field_AC[1], "mm_loading_bar_02"},
        {&field_AC[2], "mm_loading_bar_gauge"},
    };
    for (const auto &binding : quads) {
        *binding.destination = panel->GetPQ(binding.name);
        assert(*binding.destination != nullptr);
        (*binding.destination)->TurnOn(false);
    }

    field_F4 = static_cast<FEMultiLineText *>(panel->GetTextPointer("mm_mainmenu_text_CHECKING"));
    field_EC = static_cast<FEMultiLineText *>(panel->GetTextPointer("mm_dialog_box_text_BODY"));
    field_D4[0] = panel->GetTextPointer("mm_dialog_box_text_line_01");
    field_D4[1] = panel->GetTextPointer("mm_dialog_box_text_line_03");
    field_D4[2] = panel->GetTextPointer("mm_dialog_box_text_line_02");
    field_F0 = static_cast<FEMultiLineText *>(panel->GetTextPointer("mm_dialog_box_text_BODY_ps2"));
    field_E0[0] = panel->GetTextPointer("mm_dialog_box_text_line_01_ps2");
    field_E0[1] = panel->GetTextPointer("mm_dialog_box_text_line_02_ps2");
    field_E0[2] = panel->GetTextPointer("mm_dialog_box_text_line_03_ps2");
    field_F4->SetShown(false);
    field_EC->SetShown(false);
    field_F0->SetShown(false);
    for (auto *text : field_D4)
        text->SetShown(false);
    for (auto *text : field_E0)
        text->SetShown(false);

    field_B8 = panel->GetAnimationPointer(6);
    field_BC = panel->GetAnimationPointer(8);
    field_C0 = panel->GetAnimationPointer(9);
    field_C4 = panel->GetAnimationPointer(16);
    field_C8 = panel->GetAnimationPointer(18);
    field_CC = panel->GetAnimationPointer(3);
    field_D0 = panel->GetAnimationPointer(1);

    if (g_game_ptr != nullptr && g_game_ptr->field_7C != nullptr)
        field_18C = g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(464));
}

namespace {

void start_animation(PanelAnimFile *animation, bool reverse)
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
    animation->field_28 = 0;
    animation->field_2C = false;
    animation->field_2D = true;
}

void set_box_text(FEMultiLineText *text, const mString &value)
{
    text->SetTextBoxNoLocalize(*bit_cast<FEMultiLineText::string *>(const_cast<mString *>(&value)), -1, -1.0f);
}
}  // namespace

void main_menu_memcard_check::SetDialogMessage()
{
    auto localized = [](int id) {
        return g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(id));
    };

    switch (static_cast<int>(field_108)) {
    case 0:
        field_12C[0] = localized(451);
        break;
    case 1:
        field_12C[0] = localized(466);
        break;
    case 2:
        field_12C[0] = localized(21);
        break;
    case 3:
        field_12C[0] = localized(452);
        break;
    case 4:
        field_12C[0] = localized(453);
        break;
    case 5:
        field_12C[0] = "CHECKING MEMORY CARD.";
        break;
    case 6:
        field_12C[1] = localized(22);
        break;
    case 8:
        field_12C[1] = "ARE YOU SURE YOU WANT TO FORMAT?";
        break;
    case 9:
        field_12C[1] = localized(24);
        break;
    case 10:
        field_12C[1] = localized(21);
        break;
    case 13:
        field_12C[1] = localized(462);
        break;
    case 14:
        field_12C[1] = localized(460);
        break;
    case 15:
        field_12C[1] = field_18C;
        break;
    case 18:
        field_12C[1] = localized(475);
        break;
    case 19:
        field_12C[1] = localized(307);
        break;
    case 20: {
        const int device = input_mgr::instance->field_58;
        field_12C[1] = mString(0, localized(455), (device == -1 ? 1000000 : device) - 999999);
        break;
    }
    case 21:
        field_12C[1] = localized(472);
        break;
    case 22:
        field_12C[1] = localized(459);
        break;
    default:
        break;
    }
}
void main_menu_memcard_check::SetUpDialogBox(dialog_state state)
{
    if (state < 0 || state >= 24)
        return;


    if (static_cast<int>(field_108) == 21) {
        field_108 = static_cast<dialog_state>(field_10C.at(field_10C.size() - 1));
        --field_10C.m_size;
    }
    if (state != field_108 && (field_10C.size() == 0 || field_10C.at(field_10C.size() - 1) != field_108))
        field_10C.push_back(field_108);
    field_108 = state;
    field_120 = false;
    if ((field_104 == 4 || field_104 == 6) && field_129)
        field_104 = 5;
    else if (field_19C->field_30 == 11 && field_104 > 3)
        field_104 = 6;
    SetDialogMessage();
    auto localized = [](int id) {
        return g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(id));
    };
    int option = -1;
    switch (static_cast<int>(state)) {
    case 6:
    case 9:
    case 10:
    case 19:
    case 21:
        option = 39;
        break;
    case 8:
        option = 37;
        break;
    case 12:
    case 13:
    case 16:
    case 18:
    case 22:
        option = 50;
        break;
    case 14:
        option = 51;
        break;
    case 15:
        option = 255;
        break;
    default:
        break;
    }
    field_12C[3] = option < 0 ? "" : localized(option);
    switch (static_cast<int>(state)) {
    case 8:
        field_12C[4] = localized(38);
        break;
    case 13:
        field_12C[4] = localized(53);
        break;
    case 14:
    case 18:
    case 22:
        break;
    default:
        field_12C[4] = "";
        break;
    }
    if (state != 13 && state != 14 && state != 16)
        field_12C[5] = "";
    for (int i = 0; i < 3; ++i) {
        field_D4[i]->SetNoFlash(color32{0xFFC8C8C8});
        field_D4[i]->SetScale(0.9f);
        field_E0[i]->SetNoFlash(color32{0xFFC8C8C8});
        field_E0[i]->SetScale(0.9f);
    }

    int selected = state == 8 ? 1 : -1;
    if (selected < 0 || field_12C[selected + 3].size() == 0) {
        selected = -1;
        for (int i = 0; i < 3; ++i) {
            if (field_12C[i + 3].size() != 0) {
                selected = i;
                break;
            }
        }
    }
    if (selected >= 0) {
        field_11C = field_100;
        field_100 = selected;
        field_D4[selected]->SetFlash(color32{0xFFE6D03F}, color32{0x80E6D03F}, 0.6f);
        field_D4[selected]->SetScale(1.0f);
        field_E0[selected]->SetFlash(color32{0xFFE6D03F}, color32{0x80E6D03F}, 0.6f);
        field_E0[selected]->SetScale(1.0f);
    }
}

void main_menu_memcard_check::OperationFailed(MemoryUnitManager::eOperation operation,
                                              MemoryUnitManager::eStatus status)
{
    if (operation == MemoryUnitManager::OPERATION_LOAD && status == MemoryUnitManager::STATUS_LOAD_CORRUPT) {
        SetUpDialogBox(DIALOG_LOAD_CORRUPT);
        return;
    }

    if (operation == MemoryUnitManager::OPERATION_DELETE) {
        field_18C = g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(27));
        field_18C += " ";
        field_18C += g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(465));
    } else if (operation == MemoryUnitManager::OPERATION_FORMAT) {
        field_18C = g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(28));
        field_18C += " ";
        field_18C += g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(465));
    } else {
        field_18C = g_game_ptr->field_7C->lookup_localized_string(static_cast<global_text_enum>(464));
    }
    SetUpDialogBox(DIALOG_OPERATION_FAILED);
}


void main_menu_memcard_check::UpdateText()
{
    set_box_text(field_F4, field_12C[0]);
    set_box_text(field_EC, field_12C[1]);
    set_box_text(field_F0, field_12C[1]);
    field_12C[2] = field_12C[1];

    for (int i = 0; i < 3; ++i) {
        const bool shown = field_12C[3 + i].size() != 0;
        if (shown) {
            field_D4[i]->SetTextNoLocalize(*bit_cast<FEText::string *>(&field_12C[3 + i]));
            field_E0[i]->SetTextNoLocalize(*bit_cast<FEText::string *>(&field_12C[3 + i]));
        }
        field_D4[i]->SetShown(shown);
        field_E0[i]->SetShown(shown);
    }
}

void main_menu_memcard_check::LoadMemoryCard()
{
    if (!os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(146))) {
        field_108 = DIALOG_NO_SAVE;
        SetDialogMessage();
        UpdateText();
        field_120 = true;
        return;
    }

    g_game_ptr->gamefile->reset_container(false);
    DWORD disk_info[8]{};
    if (MemoryUnitManager::GetDiskInfo(disk_info) && MemoryUnitManager::EnumerateSaveDirectories() > 0) {
        g_game_ptr->gamefile->load();
        field_F8 = true;
        return;
    }

    const unsigned int save_size =
        MemoryUnitManager::GetGameSaveSize(std::max(0x4000, 2 * g_game_ptr->gamefile->field_4B4 + 400));
    const unsigned long long free_bytes = (static_cast<unsigned long long>(disk_info[1]) << 32) | disk_info[0];
    if (MemoryUnitManager::GetLastError() == MemoryUnitManager::STATUS_OK && 3ull * save_size <= free_bytes) {
        field_108 = DIALOG_NO_SAVE;
        SetDialogMessage();
        UpdateText();
        field_120 = true;
    } else {
        SetUpDialogBox(DIALOG_INSUFFICIENT_SPACE);
    }
}

void main_menu_memcard_check::Draw()
{
    for (auto *quad : field_30)
        quad->Draw();

    if (field_108 >= DIALOG_LOAD_CORRUPT) {
        for (auto *quad : field_9C)
            quad->Draw();
        field_EC->Draw();
        for (auto *text : field_D4)
            text->Draw();
    }
}

void main_menu_memcard_check::Update(Float delta_time)
{
    if (field_104 == 0) {
        for (auto *quad : field_AC)
            quad->SetAlpha(1.0f);
        field_F4->SetShown(true);
        start_animation(field_B8, true);
        field_104 = 1;
        field_FC = 1.0f;
    } else if (field_104 == 1 && !field_B8->field_2D && field_108 == DIALOG_CHECKING) {
        field_FC -= delta_time;
        if (field_FC <= 0.0f) {
            field_108 = DIALOG_NONE;
            LoadMemoryCard();
            SetDialogMessage();
            UpdateText();
            if (field_108 < DIALOG_LOAD_CORRUPT)
                field_FC = 1.0f;
        }
    }

    if (field_F8 && !MemoryUnitManager::Service())
        field_F8 = false;
    if (field_120) {
        field_30[23]->TurnOn(false);
        field_30[24]->TurnOn(false);
        field_19C->GoNextState();
    }
}

void main_menu_memcard_check::OnActivate()
{
    LoadMemoryCard();
    field_104 = 7;
    g_cursor->sub_5A6790();
    g_cursor->sub_5A67D0(350, 350, 430, 380);

    if (!field_D0->field_2D) {
        start_animation(field_D0, false);
        start_animation(field_CC, false);
    }
    for (auto *quad : field_30)
        quad->TurnOn(true);
    for (auto *quad : field_AC)
        quad->SetAlpha(0.0f);
    for (auto *quad : field_9C)
        quad->TurnOn(true);

    field_F4->SetShown(false);
    field_EC->SetShown(true);
    field_F0->SetShown(true);
    field_30[23]->TurnOn(false);
    field_30[24]->TurnOn(false);
    field_EC->SetNoFlash(color32{0xFFC8C8C8});
    field_EC->SetButtonColor(color32{0xFFFFFFFF});
    field_EC->SetButtonScale(1.0f);
    field_EC->SetScale(0.9f);
    field_F4->SetNoFlash(color32{0xFFE6D03F});
    field_F4->SetButtonColor(color32{0xFFE6D03F});
    field_F4->SetButtonScale(1.0f);
    field_F0->SetNoFlash(color32{0xFFC8C8C8});
    field_F0->SetButtonColor(color32{0xFFFFFFFF});
    field_F0->SetButtonScale(1.0f);

    field_30[25]->SetAlpha(0.0f);
    SetDialogMessage();
    UpdateText();
    field_120 = false;
    field_108 = DIALOG_CHECKING;
    SetDialogMessage();
    UpdateText();
    field_FC = 0;
    field_104 = 0;
}

void main_menu_memcard_check::OnCross(int)
{
    if (!field_C4->field_2D &&
        (field_108 == DIALOG_LOAD_CORRUPT || field_108 == DIALOG_OPERATION_FAILED ||
         field_108 == DIALOG_INSUFFICIENT_SPACE) &&
        field_100 == 0) {
        field_EC->SetShown(false);
        field_F0->SetShown(false);
        for (auto *text : field_D4)
            text->SetShown(false);
        for (auto *text : field_E0)
            text->SetShown(false);
        field_120 = true;
    }
}

void main_menu_memcard_check::OnSuccessfulLoad()
{
    auto *load_menu = static_cast<main_menu_load *>(field_19C->field_4[4]);
    bool any_valid = false;
    for (int i = 0; i < 3; ++i) {
        const auto &data = g_game_ptr->gamefile->field_28C[i];
        const bool valid = g_game_ptr->gamefile->m_game_data_valid[i] && data.timestamp.year != 0;
        load_menu->SetSaveSlot(i, valid ? &data : nullptr);
        any_valid |= valid;
    }
    field_108 = any_valid ? DIALOG_HAS_SAVE : DIALOG_NO_SAVE;
    SetDialogMessage();
    UpdateText();
    field_120 = true;
}
