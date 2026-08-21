#include "race_announcer.h"

#include "panelfile.h"

race_announcer::race_announcer() {}

void race_announcer::Init()
{
    if (this->field_0 == nullptr) {
        this->field_0 = PanelFile::UnmashPanelFile("race_announcer", static_cast<panel_layer>(7));

        this->field_8 = this->field_0->GetPQ("ra_burst");

        this->field_C = this->field_0->GetTextPointer("ra_text_READY");

        this->field_10 = this->field_0->GetTextPointer("ra_text_SET");

        this->field_14 = this->field_0->GetTextPointer("ra_text_GO");
        this->field_C->SetText(static_cast<global_text_enum>(421));
        this->field_10->SetText(static_cast<global_text_enum>(422));
        this->field_14->SetText(static_cast<global_text_enum>(423));
        this->field_C->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
        this->field_10->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
        this->field_14->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
        this->field_4 = *this->field_0->field_28.m_data;
    }

    if (this->field_0 != nullptr) {
        this->field_8->TurnOn(false);
        auto **v2 = &this->field_C;
        auto v3 = 3;
        do {
            (*v2)->SetShown(false);
            ++v2;
            --v3;
        } while (v3);
        this->field_19 = false;
    }
}

void race_announcer::DeInit()
{
    if (this->field_0 != nullptr) {
        this->field_0 = nullptr;
    }
}
