#include "fe_score_widget.h"

#include "panelfile.h"

fe_score_widget::fe_score_widget() {}

void fe_score_widget::Init()
{
    if (this->field_14 == nullptr) {
        this->field_14 = PanelFile::UnmashPanelFile("highscoretracking", static_cast<panel_layer>(7));

        this->field_0 = this->field_14->GetTextPointer("highscore_01");
        this->field_0->SetShown(true);

        this->field_0->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
        this->field_0->SetScale(1.0);

        mString v5{"0"};
        this->field_0->SetTextNoLocalize(*bit_cast<FEText::string *>(&v5));
        this->field_18 = *this->field_14->field_28.m_data;
        this->field_4 = 0;
        this->field_5 = 0;
        this->field_8 = 0;
        this->field_C = 0;
        this->field_10 = 0;
    }
}

void fe_score_widget::DeInit()
{
    if (this->field_14 != nullptr) {
        this->field_14 = nullptr;
    }
}
