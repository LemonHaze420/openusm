#pragma once

#include "float.hpp"
#include "font_index.h"

#include <cstdint>

struct FEMenu;

struct FEMenuSystem {
    std::intptr_t m_vtbl;

    FEMenu **field_4;
    font_index field_8;
    int m_index;

    int field_10;
    int m_count;
    uint16_t field_18[8];

    char field_28;
    char field_29;
    short field_2A;

    //0x00618760
    FEMenuSystem(int a2, font_index a3);

    void * operator new(std::size_t);

    void operator delete(void *, std::size_t);

    //virtual
    void Update(Float);

    //0x00629960
    //virtual
    void UpdateButtonDown();

    //0x0060B610
    //virtual
    void MakeActive(int idx);

    //virtual
    char GetDefaultColorScheme();

    //virtual
    bool GetSingleInput();

    //0x006187D0
    //virtual
    void OnButtonPress(int a2, int a3);

    //0x0060B6B0
    //virtual
    void OnButtonRelease(int a2, int a3);

    //0x006298D0
    //virtual
    void UpdateButtonPresses();
};

extern void FEMenuSystem_patch();
