#pragma once

#include "color32.h"

struct FETextFlashInfo {
    color32 field_0;
    float field_4;
    float field_8;
    float field_C;

    FETextFlashInfo(color32 a3, float a4);

    //0x00609700
    [[nodiscard]] color32 GetColor(color32 a3);
};
