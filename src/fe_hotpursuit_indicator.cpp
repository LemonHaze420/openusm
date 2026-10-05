#include "fe_hotpursuit_indicator.h"

#include "panelfile.h"


#include <algorithm>

fe_hotpursuit_indicator::fe_hotpursuit_indicator()
{
    field_8 = nullptr;
}

void fe_hotpursuit_indicator::Draw()
{
    if (field_0 && field_8 != nullptr) {
        field_14->Mask(std::clamp(field_4 * 0.2f, 0.0f, 1.0f), 2, -1.0f);
        field_8->Draw();
    }
}

void fe_hotpursuit_indicator::Init()
{
    if (this->field_8 == nullptr) {
        this->field_8 = PanelFile::UnmashPanelFile("mg_hotpursuit_indicator", static_cast<panel_layer>(7));
        auto **v3 = &this->field_10;

        this->field_10 = this->field_8->GetPQ("mg_hp_indicator_frame");
        this->field_14 = this->field_8->GetPQ("mg_hp_indicator_bar_use");
        this->field_18 = this->field_8->GetPQ("mg_hp_indicator_anim_only");
        auto v8 = 3;
        do {
            (*v3)->TurnOn(true);
            ++v3;
            --v8;
        } while (v8);
        this->field_C = *this->field_8->field_28.m_data;
        this->field_0 = false;
        this->field_1 = false;
        this->field_4 = 0;
    }
}

void fe_hotpursuit_indicator::DeInit()
{
    if (this->field_8 != nullptr) {
        this->field_8 = nullptr;
    }
}
