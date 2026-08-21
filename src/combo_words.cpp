#include "combo_words.h"

#include "panelfile.h"

combo_words::combo_words() {}

void combo_words::Init()
{
    TRACE("combo_words::Init");

    if (this->field_0 == nullptr) {
        this->field_0 = PanelFile::UnmashPanelFile("combo_words", static_cast<panel_layer>(6));
        this->field_4 = *this->field_0->field_28.m_data;

        this->field_8[0] = this->field_0->GetTextPointer("combo_word_01");
        this->field_8[1] = this->field_0->GetTextPointer("combo_word_02");
        this->field_8[2] = this->field_0->GetTextPointer("combo_word_03");
        this->field_8[3] = this->field_0->GetTextPointer("combo_word_04");
        this->field_8[4] = this->field_0->GetTextPointer("combo_word_05");
        this->field_8[5] = this->field_0->GetTextPointer("combo_word_06");
        this->field_8[6] = this->field_0->GetTextPointer("combo_word_07");

        for (int i = 0; i < 7; ++i) {
            this->field_8[i]->SetText(static_cast<global_text_enum>(408 + i));
        }

        for (int i = 0; i < 7; ++i) {
            auto *v3 = this->field_8[i];
            v3->SetNoFlash(color32{0xC8, 0xC8, 0xC8, 0xFF});
            v3->SetShown(false);
        }
    }

    auto v19 = this->field_24;
    if (v19 > -1 && v19 < 7) {
        this->field_8[v19]->SetShown(false);
    }
}

void combo_words::DeInit()
{
    if (this->field_0 != nullptr) {
        this->field_0 = nullptr;
    }
}

void combo_words_patch()
{
    REDIRECT(0x00571DED, func_address(&combo_words::Init));
}
