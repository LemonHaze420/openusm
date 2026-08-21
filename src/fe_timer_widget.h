#pragma once

#include <cstdint>

#include "float.hpp"
#include "utility.h"

struct PanelFile;
struct PanelAnimFile;
struct FEText;

struct fe_timer_widget {
    std::intptr_t m_vtbl;
    PanelFile *field_4;
    FEText *field_8;
    FEText *field_C;
    FEText *field_10;
    FEText *field_14;
    FEText *field_18;
    FEText *field_1C;
    PanelAnimFile *field_20;
    float field_24;
    bool field_28;
    bool field_29;
    bool field_2A;
    float field_2C;

    //0x00615B50
    fe_timer_widget();

    //0x00647BA0
    void _Init();

    //virtual
    void Init();

    //0x00633750
    void _Update(Float a2);

    //virtual
    void Update(Float a2);

    //0x00615B90
    void _Draw();

    //virtual
    void Draw();

    //0x006280F0
    void _SetShown(bool a2);

    //virtual
    void SetShown(bool a2);

    static inline void *g_vtbl[]{
        nullptr, func_address(&_Init), func_address(&_Update), func_address(&_Draw), func_address(&_SetShown)};
};

extern void fe_timer_widget_patch();
