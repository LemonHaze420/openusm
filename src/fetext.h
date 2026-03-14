#pragma once

#include "panelanimobject.h"

#include "color32.h"
#include "femanager.h"
#include "global_text_enum.h"
#include "mstring.h"
#include "panel_layer.h"
#include "vector2d.h"

#include <cstdint>

struct FETextFlashInfo;

struct FEText : PanelAnimObject {

    FETextFlashInfo *flash_info;
    font_index field_18;

    mString field_1C;

    int field_2C;
    int field_30;
    vector2d field_34;
    float field_3C;
    float field_40;
    vector2d field_44;
    color32 field_4C;

    mString field_50;
    int field_60;
    int8_t field_64;
    char pad[3];

    FEText();

    //0x00617500
    FEText(font_index a2,
           global_text_enum a3,
           Float a4,
           Float a5,
           int a6,
           panel_layer a7,
           Float a8,
           int a9,
           int a10,
           color32 a11);

    bool CheckIfNotTooLong(int);

    //0x0062E540
    //virtual
    void _unmash(mash_info_struct *a1, void *a3);

    //virtual
    int _get_mash_sizeof();

    //0x00617640
    //virtual
    void Draw();

    struct string {
        int field_0;
        int m_size;
        char *guts;
        int field_C;

        string(const mString &a1) {
            std::memcpy(this, &a1, sizeof(mString));
        }
    };

    /* virtual */ void Update(Float a2);

    void _SetText(global_text_enum a2);

    //0x00617760
    /* virtual */ void SetText(global_text_enum a2);

    /* virtual */ void SetPos(Float a2, Float a3);

    //0x00609980
    //virtual
    void SetNoFlash(color32 a2);

    //0x0043C240
    //virtual
    void SetScale(Float a2, Float a3);

    //0x0043C220
    //virtual
    void SetScale(Float a2);

    //0x0043C370
    //virtual
    bool GetFlag(int a2);

    void _SetTextNoLocalize(string a1);

    //0x0043C410
    /* virtual */ void SetTextNoLocalize(string a1);

    /* virtual */ void SetX(Float a2);

    /* virtual */ void SetY(Float a2);

    /* virtual */ mString GetName();

    /* virtual */ void AdjustForJustification(float *a2, float *a3);

    /* virtual */ void SetFlash(color32 a2, color32 a3, Float a4);

    /* virtual */ float GetX();

    /* virtual */ float GetY();
};

extern void FEText_patch();
