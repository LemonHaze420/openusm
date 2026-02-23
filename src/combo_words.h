#pragma once

struct PanelFile;
struct FEText;
struct PanelAnimFile;

struct combo_words {
    PanelFile *field_0;
    PanelAnimFile *field_4;
    FEText *field_8[7];
    int field_24;
    float field_28;

    combo_words();

    void Init();

    void DeInit();
};

extern void combo_words_patch();
