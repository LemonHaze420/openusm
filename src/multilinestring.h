#pragma once

#include "float.hpp"
#include "font_index.h"
#include "mstring.h"
#include "vector2d.h"

struct MultiLineString {
    font_index m_font_index;
    vector2d field_4;
    int field_C;
    mString field_10;

    struct button_t {
        short field_0;
        short field_2;

        button_t() = default;
    };

    button_t *button_array;
    int button_array_size;

    //0x00609AA0
    MultiLineString();

    struct string {
        int field_0;
        int m_size;
        const char *guts;
        int field_C;
    };

    //0x00617BF0
    static double GetWidth(string a1, Float a5, font_index a3);

    static int ConvertStringToButtonCode(const char *a1, const char **a2, const mString &a3);

    //0x00617960
    void Draw(Float a2, int a3, int a4, Float a5, Float a6, Float a7, Float a8);

    //0x00628ED0
    int ParseForButtons(Float a3, Float a4);

    //0x0062E5E0
    void Set(string a2, font_index a7, Float a8, Float a9);
};

extern void MultiLineString_patch();
