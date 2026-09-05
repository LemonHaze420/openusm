#include "pause_menu_message_log.h"

#include "common.h"
#include "fetext.h"
#include "panelfile.h"
#include "panelquad.h"
#include "pausemenusystem.h"
#include "func_wrapper.h"
#include "utility.h"
#include "trace.h"

VALIDATE_SIZE(pause_menu_message_log, 0x100u);

pause_menu_message_log::pause_menu_message_log(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x008943D8;
    for (int i = 0; i <= 36; ++i)
        field_2C[i] = 0;
    field_2C[37] = reinterpret_cast<int>(a2);
    field_2C[38] = 1;
    field_2C[39] = 8;
    field_2C[40] = 1;
    field_2C[41] = 1;
    field_2C[42] = 0;
    field_2C[43] = -1;
}

void pause_menu_message_log::_Load()
{
    TRACE("pause_menu_message_log::Load");

    reinterpret_cast<unsigned char *>(&field_2C[45])[0] = 0;
    reinterpret_cast<unsigned char *>(&field_2C[45])[1] = 0;
    auto *menu_system = reinterpret_cast<PauseMenuSystem *>(field_2C[37]);
    auto *panel = menu_system->field_2C;
    static constexpr const char *quad_names[28] {
        "pm_all_back_01", "pm_all_back_02", "pm_all_back_02a", "pm_all_back_02b", "pm_all_back_03",
        "pm_all_back_04", "pm_all_back_05", "pm_all_detail_02", "pm_all_detail_03", "pm_all_detail_04",
        "pm_all_detail_05", "pm_all_detail_06", "pm_all_detail_07", "pm_all_detail_08", "pm_all_detail_09",
        "pm_all_detail_10", "pm_all_detail_11", "pm_all_box_01", "pm_all_box_02", "pm_all_box_03",
        "pm_all_icon", "pm_status_hilite_text", "pm_status_hilite_text_01", "pm_scroll_arrow_down_01",
        "pm_scroll_arrow_up_01", "pm_scroll_bar_03", "pm_scroll_bar_04", "pm_scroll_spider_icon_01",
    };
    for (int i = 0; i < 28; ++i) {
        auto *quad = panel->GetPQ(quad_names[i]);
        field_2C[i] = static_cast<int>(reinterpret_cast<uintptr_t>(quad));
    }

    static constexpr const char *text_names[9] {
        "pm_header_text_MESSAGELOG", "pm_status_text_left_01_BLANK", "pm_status_text_left_02_BLANK",
        "pm_status_text_left_03_BLANK", "pm_status_text_left_04_BLANK", "pm_status_text_left_05_BLANK",
        "pm_status_text_left_06_BLANK", "pm_status_text_left_07_BLANK", "pm_status_text_left_08_BLANK",
    };
    FEText *texts[9] {};
    for (int i = 0; i < 9; ++i) {
        texts[i] = panel->GetTextPointer(text_names[i]);
        field_2C[28 + i] = static_cast<int>(reinterpret_cast<uintptr_t>(texts[i]));
    }
    texts[0]->SetText(static_cast<global_text_enum>(274));
    for (int i = 1; i < 9; ++i)
        texts[i]->SetTextNoLocalize(FEText::string {mString {""}});

    reinterpret_cast<float *>(&field_2C[48])[0] = reinterpret_cast<PanelQuad *>(field_2C[27])->GetCenterY();
    reinterpret_cast<float *>(&field_2C[49])[0] = panel->GetPQ("pm_scroll_spider_icon_end_marker_01")->GetCenterY();
    reinterpret_cast<float *>(&field_2C[46])[0] = reinterpret_cast<PanelQuad *>(field_2C[21])->GetCenterY() - texts[1]->GetY();
    reinterpret_cast<float *>(&field_2C[47])[0] = reinterpret_cast<PanelQuad *>(field_2C[22])->GetCenterY() - texts[1]->GetY();
    reinterpret_cast<unsigned char *>(&field_2C[50])[0] = 0;
}

void pause_menu_message_log_patch()
{
    FUNC_ADDRESS(address, &pause_menu_message_log::_Load);
    set_vfunc(0x0008943E8, address);
}
