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

pause_menu_controller::pause_menu_controller(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x00610AD0, this, a2, a3, a4);
}

void pause_menu_controller::load_ps2_spider(PanelFile *a2)
{
    THISCALL(0x0063C710, this, a2);
}

void pause_menu_controller::load_ps2_venom(PanelFile *a2)
{
    THISCALL(0x0063CC50, this, a2);
}

void pause_menu_controller::_Load()
{
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

void sub_579030()
{
    dword_960E14() = 0;
}

void pause_menu_controller::OnActivate()
{
    auto *v2 = this->field_214->field_30;
    v2->field_4 = {};
    v2->field_28 = 0;
    v2->AddButtons(menu_nav_bar::button_type{2}, menu_nav_bar::button_type{17}, static_cast<global_text_enum>(8));
    v2->AddButtons(menu_nav_bar::button_type{15}, menu_nav_bar::button_type{17}, static_cast<global_text_enum>(3));
    v2->Reformat();
    sub_579030();
}

void pause_menu_controller_patch()
{
    {
        FUNC_ADDRESS(address, &pause_menu_controller::_Load);
        set_vfunc(0x00894210, address);
    }
}
