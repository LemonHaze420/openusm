#include "tutorial_controller_gauge.h"

#include "func_wrapper.h"
#include "panelfile.h"

tutorial_controller_gauge::tutorial_controller_gauge() {}

void tutorial_controller_gauge::Init()
{
    if (this->field_0 == nullptr) {
        this->field_4C = 1.0;
        this->field_50 = 1.0;
        this->field_54 = 1.0;
        this->field_58 = 1.0;
        this->field_5C = 1.0;
        this->field_60 = 1.0;
        this->field_64 = 1.0;
        this->field_68 = 1.0;
        this->field_6C = 1.0;
        this->field_70 = 1.0;
        this->field_74 = 1.0;
        this->field_78 = 1.0;

        this->field_0 = PanelFile::UnmashPanelFile("controller_tut_ps", static_cast<panel_layer>(7));

        this->field_C[0] = this->field_0->GetPQ("CT_PS_box_back_01");

        this->field_C[1] = this->field_0->GetPQ("CT_PS_box_back_02");

        this->field_C[2] = this->field_0->GetPQ("CT_PS_box_back_03");

        this->field_C[3] = this->field_0->GetPQ("CT_PS2_con_back");

        this->field_1C[10] = this->field_0->GetPQ("CT_PS2_con_btn_analog_left");

        this->field_1C[11] = this->field_0->GetPQ("CT_PS2_con_btn_analog_right");

        this->field_1C[1] = this->field_0->GetPQ("CT_PS2_con_btn_circle");

        this->field_1C[2] = this->field_0->GetPQ("CT_PS2_con_btn_x");

        this->field_1C[3] = this->field_0->GetPQ("CT_PS2_con_btn_triangle");

        this->field_1C[3] = this->field_0->GetPQ("CT_PS2_con_btn_square");

        this->field_1C[6] = this->field_0->GetPQ("CT_PS2_con_btn_L1");

        this->field_1C[7] = this->field_0->GetPQ("CT_PS2_con_btn_L2");

        this->field_1C[4] = this->field_0->GetPQ("CT_PS2_con_btn_R1");

        this->field_1C[5] = this->field_0->GetPQ("CT_PS2_con_btn_R2");

        this->field_1C[9] = this->field_0->GetPQ("CT_PS2_con_btn_select");

        this->field_1C[8] = nullptr;
        this->field_74 = 1.2;
        this->field_78 = 1.2;
        this->field_5C = 1.2;
        this->field_60 = 1.2;
        this->field_64 = 1.2;
        this->field_68 = 1.2;
    }

    this->field_84 = this->field_0->GetPQ("CT_burst_hilite");

    this->field_88 = this->field_0->GetPQ("CT_burst_hilite_2");

    this->field_88->CopyFrom(this->field_84);

    this->field_4 = this->field_0->field_28.at(0);
    this->field_8 = this->field_0->field_28.at(1);
    this->field_7C = 1.0;
    this->field_80 = 1.0;
    this->Hide();
}

void tutorial_controller_gauge::Hide()
{
    THISCALL(0x006223B0, this);
}

void tutorial_controller_gauge::DeInit()
{
    if (this->field_0 != nullptr) {
        this->field_0 = nullptr;
        this->field_4 = nullptr;
        this->field_8 = nullptr;
    }
}
