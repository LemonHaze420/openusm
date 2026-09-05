#include "pause_menu_save_load_display.h"

#include "pause_menu_save_load_hookup.h"

#include "common.h"
#include "fetext.h"
#include "panelfile.h"
#include "panelquad.h"
#include "pausemenusystem.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(pause_menu_save_load_display, 0xF4u);

pause_menu_save_load_display::pause_menu_save_load_display(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x008942B0;
    field_2C = new pause_menu_save_load_hookup;
    field_2C->initialize();
    field_2C->field_0[1] = reinterpret_cast<int>(this);

    for (int i = 0; i <= 38; ++i)
        field_30[i] = 0;
    field_30[39] = reinterpret_cast<int>(a2);
    field_30[42] = 1;
    field_30[43] = 3;
    field_30[44] = 3;
    field_30[45] = 1;
    field_30[46] = 1;
    reinterpret_cast<unsigned char *>(&field_30[48])[0] = 0;
    reinterpret_cast<unsigned char *>(&field_30[48])[1] = 0;
}

void pause_menu_save_load_display::_Load()
{
    TRACE("pause_menu_save_load_display::_Load");

    auto *menu_system = reinterpret_cast<PauseMenuSystem *>(field_30[39]);
    auto *panel = menu_system->field_2C;
    static constexpr const char *quad_names[28] {
        "pm_all_back_01", "pm_all_back_02", "pm_all_back_02a", "pm_all_back_02b", "pm_all_back_03",
        "pm_all_back_04", "pm_all_back_05", "pm_all_detail_02", "pm_all_detail_03", "pm_all_detail_04",
        "pm_all_detail_05", "pm_all_detail_06", "pm_all_detail_07", "pm_all_detail_08", "pm_all_detail_09",
        "pm_all_detail_10", "pm_all_detail_11", "pm_all_box_01", "pm_all_box_02", "pm_all_box_03",
        "pm_all_icon", "pm_save_hilite_text", "pm_save_hilite_text_01", "pm_save_dialog_box_01",
        "pm_save_dialog_box_02", "pm_save_dialog_box_loading_bar_01", "pm_save_dialog_box_loading_bar_02",
        "pm_save_dialog_box_loading_bar_gauge",
    };
    for (int i = 0; i < 28; ++i) {
        auto *quad = panel->GetPQ(quad_names[i]);
        field_30[i] = static_cast<int>(reinterpret_cast<uintptr_t>(quad));
        quad->TurnOn(true);
    }

    static constexpr const char *text_names[11] {
        "pm_header_text_SAVEGAME", "pm_save_text_left_01_BLANK", "pm_save_text_left_02_BLANK",
        "pm_save_text_left_03_BLANK", "pm_save_text_right_01_BLANK", "pm_save_text_right_02_BLANK",
        "pm_save_text_right_03_BLANK", "pm_save_dialog_box_text_link_left_BLANK",
        "pm_save_dialog_box_text_link_right_BLANK", "pm_save_dialog_box_text_link_center_BLANK",
        "pm_save_dialog_box_text_message_BLANK",
    };
    FEText *texts[11] {};
    for (int i = 0; i < 11; ++i) {
        texts[i] = panel->GetTextPointer(text_names[i]);
        field_30[28 + i] = static_cast<int>(reinterpret_cast<uintptr_t>(texts[i]));
        texts[i]->SetShown(true);
        texts[i]->SetNoFlash(color32 {0xFFC87238});
    }

    const auto &header = field_2C->field_98[field_2C->field_0[5] != 0 ? 1 : 0];
    texts[0]->SetTextNoLocalize(FEText::string {header});
    for (int slot = 0; slot < 3; ++slot) {
        texts[1 + slot]->SetTextNoLocalize(FEText::string {field_2C->field_2C[slot].field_0});
        texts[4 + slot]->SetTextNoLocalize(FEText::string {field_2C->field_2C[slot].field_10});
    }
    for (int i = 7; i < 11; ++i)
        texts[i]->SetTextNoLocalize(FEText::string {field_2C->field_98[31]});

    texts[0]->SetNoFlash(color32 {0xFFC8C8C8});
    texts[10]->SetNoFlash(color32 {0xFFC8C8C8});
    texts[7]->SetNoFlash(color32 {0xFFE6D03F});
    texts[7]->SetScale(1.2f, 1.2f);
    texts[8]->SetNoFlash(color32 {0xFFE6D03F});
    texts[8]->SetScale(1.2f, 1.2f);
    texts[9]->SetNoFlash(color32 {0xFFE6D03F});
    texts[9]->SetScale(1.2f, 1.2f);

    field_30[47] = 0;
    field_30[40] = static_cast<int>(reinterpret_cast<PanelQuad *>(field_30[21])->GetCenterY() - texts[1]->GetY());
    field_30[41] = static_cast<int>(reinterpret_cast<PanelQuad *>(field_30[22])->GetCenterY() - texts[1]->GetY());

    for (int i = 1; i <= 6; ++i) {
        texts[i]->SetNoFlash(color32 {0xFFC87238});
        texts[i]->SetScale(1.0f, 1.0f);
    }
    const int selected = field_30[45];
    const int right_column = field_30[44];
    texts[selected]->SetNoFlash(color32 {0xFFE6D03F});
    texts[selected]->SetScale(1.2f, 1.2f);
    texts[right_column + selected]->SetNoFlash(color32 {0xFFE6D03F});
    texts[right_column + selected]->SetScale(1.2f, 1.2f);

    for (int i = 23; i < 28; ++i)
        reinterpret_cast<PanelQuad *>(field_30[i])->TurnOn(false);
    for (int i = 7; i < 11; ++i)
        texts[i]->SetShown(false);
}

void pause_menu_save_load_display_patch()
{
    FUNC_ADDRESS(address, &pause_menu_save_load_display::_Load);
    set_vfunc(0x008942C0, address);
}
