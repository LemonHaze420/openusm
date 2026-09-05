#include "pause_menu_controller.h"

#include "common.h"
#include "fe_menu_nav_bar.h"
#include "func_wrapper.h"
#include "game.h"
#include "localized_string_table.h"
#include "panelfile.h"
#include "pausemenusystem.h"
#include "utility.h"
#include "trace.h"

VALIDATE_SIZE(pause_menu_controller, 0x218u);

pause_menu_controller::pause_menu_controller(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x00894200;
    field_214 = static_cast<PauseMenuSystem *>(a2);
    field_2C = nullptr;

    for (auto &panel : field_30)
        panel = nullptr;
    for (int i = 0; i <= 27; ++i)
        field_temp[i] = 0;
    for (int i = 56; i <= 77; ++i)
        field_temp[i] = 0;
}

void pause_menu_controller::load_ps2_spider(PanelFile *panel) {
    static constexpr const char *quad_names[26] {
        "pm_opt_cont_PS2_spidey_01a_line",
        "pm_opt_cont_PS2_spidey_01b_line",
        "pm_opt_cont_PS2_spidey_02a_line",
        "pm_opt_cont_PS2_spidey_02b_line",
        nullptr, nullptr, nullptr, nullptr,
        "pm_opt_cont_PS2_spidey_05a_line",
        "pm_opt_cont_PS2_spidey_05b_line",
        "pm_opt_cont_PS2_spidey_06a_line",
        "pm_opt_cont_PS2_spidey_06b_line",
        "pm_opt_cont_PS2_spidey_07a_line",
        "pm_opt_cont_PS2_spidey_07b_line",
        "pm_opt_cont_PS2_spidey_08a_line",
        "pm_opt_cont_PS2_spidey_08b_line",
        "pm_opt_cont_PS2_spidey_09a_line",
        "pm_opt_cont_PS2_spidey_09b_line",
        "pm_opt_cont_PS2_spidey_10a_line",
        "pm_opt_cont_PS2_spidey_10b_line",
        "pm_opt_cont_PS2_spidey_11a_line",
        "pm_opt_cont_PS2_spidey_11b_line",
        "pm_opt_cont_PS2_spidey_12a_line",
        "pm_opt_cont_PS2_spidey_12b_line",
        "pm_opt_cont_PS2_spidey_13a_line",
        "pm_opt_cont_PS2_spidey_13b_line",
    };
    for (int i = 0; i < 26; ++i) {
        auto *quad = quad_names[i] != nullptr ? panel->GetPQ(quad_names[i]) : nullptr;
        field_temp[i] = static_cast<int>(reinterpret_cast<uintptr_t>(quad));
    }

    static constexpr const char *text_names[22] {
        "pm_opt_cont_PS2_spidey_01_text_BLANK",
        "pm_opt_cont_PS2_spidey_02_text_BLANK",
        nullptr,
        nullptr,
        "pm_opt_cont_PS2_spidey_05_text_BLANK",
        "pm_opt_cont_PS2_spidey_06_text_BLANK",
        "pm_opt_cont_PS2_spidey_07_text_BLANK",
        "pm_opt_cont_PS2_spidey_08_text_BLANK",
        "pm_opt_cont_PS2_spidey_09_text_BLANK",
        "pm_opt_cont_PS2_spidey_10_text_BLANK",
        "pm_opt_cont_PS2_spidey_11_text_BLANK",
        "pm_opt_cont_PS2_spidey_12_text_BLANK",
        "pm_opt_cont_PS2_spidey_13_text_BLANK",
        "pm_opt_cont_PS2_spidey_14_text_BLANK",
        "pm_opt_cont_PS2_spidey_15_text_BLANK",
        "pm_opt_cont_PS2_spidey_16_text_BLANK",
        "pm_opt_cont_PS2_spidey_17_text_BLANK",
        "pm_opt_cont_PS2_spidey_18_text_BLANK",
        "pm_opt_cont_PS2_spidey_19_text_BLANK",
        "pm_opt_cont_PS2_spidey_20_text_BLANK",
        "pm_opt_cont_PS2_spidey_21_text_BLANK",
        "pm_opt_cont_text_help",
    };
    static constexpr int text_ids[22] {
        119, 120, 0, 0, 121, 122, 123, 124, 125, 126, 127,
        128, 129, 130, 131, 132, 133, 134, 135, 147, 137, 138,
    };
    for (int i = 0; i < 22; ++i) {
        auto *text = text_names[i] != nullptr ? panel->GetTextPointer(text_names[i]) : nullptr;
        field_temp[56 + i] = static_cast<int>(reinterpret_cast<uintptr_t>(text));
        if (text != nullptr) {
            text->SetText(static_cast<global_text_enum>(text_ids[i]));
            text->SetNoFlash(color32 {0xFFE6D03F});
        }
    }
    reinterpret_cast<FEText *>(field_temp[66])->SetNoFlash(color32 {0xFFC8C8C8});
    reinterpret_cast<FEText *>(field_temp[74])->SetNoFlash(color32 {0xFFC8C8C8});
    reinterpret_cast<FEText *>(field_temp[77])->SetNoFlash(color32 {0xFFC8C8C8});
}

void pause_menu_controller::load_ps2_venom(PanelFile *panel) {
    static constexpr const char *quad_names[26] {
        "pm_opt_cont_PS2_spidey_01a_line",
        "pm_opt_cont_PS2_spidey_01b_line",
        "pm_opt_cont_PS2_spidey_02a_line",
        "pm_opt_cont_PS2_spidey_02b_line",
        nullptr, nullptr, nullptr, nullptr,
        "pm_opt_cont_PS2_spidey_05a_line",
        "pm_opt_cont_PS2_spidey_05b_line",
        "pm_opt_cont_PS2_spidey_06a_line",
        "pm_opt_cont_PS2_spidey_06b_line",
        "pm_opt_cont_PS2_spidey_07a_line",
        "pm_opt_cont_PS2_spidey_07b_line",
        "pm_opt_cont_PS2_spidey_08a_line",
        "pm_opt_cont_PS2_spidey_08b_line",
        "pm_opt_cont_PS2_spidey_09a_line",
        "pm_opt_cont_PS2_spidey_09b_line",
        "pm_opt_cont_PS2_spidey_10a_line",
        "pm_opt_cont_PS2_spidey_10b_line",
        "pm_opt_cont_PS2_spidey_11a_line",
        "pm_opt_cont_PS2_spidey_11b_line",
        "pm_opt_cont_PS2_spidey_12a_line",
        "pm_opt_cont_PS2_spidey_12b_line",
        "pm_opt_cont_PS2_spidey_13a_line",
        "pm_opt_cont_PS2_spidey_13b_line",
    };
    for (int i = 0; i < 26; ++i) {
        auto *quad = quad_names[i] != nullptr ? panel->GetPQ(quad_names[i]) : nullptr;
        field_temp[28 + i] = static_cast<int>(reinterpret_cast<uintptr_t>(quad));
    }

    static constexpr const char *text_names[14] {
        "pm_opt_cont_PS2_venom_01_text_BLANK",
        "pm_opt_cont_PS2_venom_02_text_BLANK",
        "pm_opt_cont_PS2_venom_03_text_BLANK",
        "pm_opt_cont_PS2_venom_04_text_BLANK",
        "pm_opt_cont_PS2_venom_05_text_BLANK",
        "pm_opt_cont_PS2_venom_06_text_BLANK",
        "pm_opt_cont_PS2_venom_07_text_BLANK",
        "pm_opt_cont_PS2_venom_08_text_BLANK",
        "pm_opt_cont_PS2_venom_09_text_BLANK",
        "pm_opt_cont_PS2_venom_10_text_BLANK",
        "pm_opt_cont_PS2_venom_11_text_BLANK",
        "pm_opt_cont_PS2_venom_12_text_BLANK",
        "pm_opt_cont_PS2_venom_13_text_BLANK",
        "pm_opt_cont_PS2_venom_14_text_BLANK",
    };
    static constexpr int text_ids[14] {
        139, 140, 141, 122, 123, 124, 142, 143, 128, 130, 144, 145, 147, 137,
    };
    for (int i = 0; i < 22; ++i) {
        FEText *text = i < 14 ? panel->GetTextPointer(text_names[i]) : nullptr;
        field_temp[78 + i] = static_cast<int>(reinterpret_cast<uintptr_t>(text));
        if (text != nullptr) {
            text->SetText(static_cast<global_text_enum>(text_ids[i]));
            text->SetNoFlash(color32 {0xFFE6D03F});
        }
    }
}

void pause_menu_controller::_Load() {
    TRACE("pause_menu_controller::_Load");

    auto *v2 = this->field_214->field_2C;
    this->field_2C = v2->GetTextPointer("pm_header_text_CONTROLLER");

    this->field_2C->SetShown(true);

    this->field_2C->SetNoFlash(color32{200, 200, 200, 255});

    this->field_30[0] = v2->GetPQ("pm_all_back_01");
    this->field_30[1] = v2->GetPQ("pm_all_back_02");
    this->field_30[2] = v2->GetPQ("pm_all_back_02a");
    this->field_30[3] = v2->GetPQ("pm_all_back_02b");
    this->field_30[4] = v2->GetPQ("pm_all_back_03");
    this->field_30[5] = v2->GetPQ("pm_all_back_04");
    this->field_30[6] = v2->GetPQ("pm_all_back_05");
    this->field_30[7] = v2->GetPQ("pm_all_detail_02");
    this->field_30[8] = v2->GetPQ("pm_all_detail_03");
    this->field_30[9] = v2->GetPQ("pm_all_detail_04");
    this->field_30[10] = v2->GetPQ("pm_all_detail_05");
    this->field_30[11] = v2->GetPQ("pm_all_detail_06");
    this->field_30[12] = v2->GetPQ("pm_all_detail_07");
    this->field_30[13] = v2->GetPQ("pm_all_detail_08");
    this->field_30[14] = v2->GetPQ("pm_all_detail_09");
    this->field_30[15] = v2->GetPQ("pm_all_detail_10");
    this->field_30[16] = v2->GetPQ("pm_all_detail_11");
    this->field_30[17] = v2->GetPQ("pm_all_box_04");
    this->field_30[18] = v2->GetPQ("pm_all_box_05");
    this->field_30[19] = v2->GetPQ("pm_all_box_06");
    this->field_30[20] = v2->GetPQ("pm_opt_cont_image");
    for (auto &panel : this->field_30) {
        panel->TurnOn(true);
    }

    this->load_ps2_spider(v2);
    this->load_ps2_venom(v2);
}

static Var<int> dword_960E14{0x00960E14};

void sub_579030() {
    dword_960E14() = 0;
}

void pause_menu_controller::OnActivate() {
    auto *v2 = this->field_214->field_30;
    v2->field_4 = {};
    v2->field_28 = 0;
    v2->AddButtons(menu_nav_bar::button_type{2}, menu_nav_bar::button_type{17}, static_cast<global_text_enum>(8));
    v2->AddButtons(menu_nav_bar::button_type{15},
                   menu_nav_bar::button_type{17},
                   static_cast<global_text_enum>(3));
    v2->Reformat();
    sub_579030();
}

void pause_menu_controller_patch() {
    {
        FUNC_ADDRESS(address, &pause_menu_controller::_Load);
        set_vfunc(0x00894210, address);
    }
}
