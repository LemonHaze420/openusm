#include "medal_award_ui.h"

#include "femultilinetext.h"
#include "panelfile.h"

medal_award_ui::medal_award_ui() {}

void medal_award_ui::Init()
{
    if (this->field_0 == nullptr) {
        this->field_0 = PanelFile::UnmashPanelFile("medals_interface", static_cast<panel_layer>(7));

        this->field_4[0] = this->field_0->GetPQ("medal_outline_01");

        this->field_4[1] = this->field_0->GetPQ("medal_outline_02");

        this->field_4[2] = this->field_0->GetPQ("medal_tb_back_01");

        this->field_4[3] = this->field_0->GetPQ("medal_tb_back_02");

        this->field_4[4] = this->field_0->GetPQ("medal_tb_detail_01");

        this->field_4[5] = this->field_0->GetPQ("medal_tb_gradient");

        this->field_1C[0] = this->field_0->GetPQ("medal_image_bronze");

        this->field_1C[1] = this->field_0->GetPQ("medal_image_silver");

        this->field_1C[2] = this->field_0->GetPQ("medal_image_gold");

        this->field_1C[3] = this->field_0->GetPQ("medal_image_ultimate");

        this->field_2C = this->field_0->GetTextPointer("medal_text_YOUWON");

        this->field_34 = this->field_0->GetTextPointer("medal_text_PRESS");

        this->field_30 = this->field_0->GetTextPointer("medal_text_MEDAL");
        this->field_2C->SetText(static_cast<global_text_enum>(419));
        this->field_34->SetText(static_cast<global_text_enum>(420));
        this->field_30->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
        this->field_2C->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
        this->field_34->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
        bit_cast<FEMultiLineText *>(this->field_34)->SetButtonColor(color32{0xFF, 0xFF, 0xFF, 0xFF});
        bit_cast<FEMultiLineText *>(this->field_34)->SetButtonScale(1.0);
        this->field_38 = *this->field_0->field_28.m_data;

        auto v31 = 6;
        auto **v3 = this->field_4;
        do {
            (*v3)->TurnOn(false);
            ++v3;
            --v31;
        } while (v31);

        auto v32 = 4;

        auto **v16 = this->field_1C;
        do {
            (*v16)->TurnOn(false);
            ++v16;
            --v32;
        } while (v32);
        this->field_30->SetShown(0);
        this->field_2C->SetShown(0);
        this->field_34->SetShown(0);
    }
}

void medal_award_ui::DeInit()
{
    if (this->field_0 != nullptr) {
        this->field_0 = nullptr;
    }
}
