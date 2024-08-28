#pragma once

#include "float.hpp"
#include "global_text_enum.h"

#include <cstdint>

struct FEMenuSystem;
struct FEText;
struct FEMenuEntry;

struct FEMenu {
    std::intptr_t m_vtbl;
    FEMenuEntry **field_4;
    FEMenuSystem *field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    float field_1C;
    int16_t field_20;
    int16_t highlighted;
    int16_t num_entries;
    int16_t field_26;
    int16_t field_28;
    char field_2A;
    char field_2B;

    //0x0060AA90
    FEMenu(FEMenuSystem *a2, uint32_t a3, int a4, int a5, int16_t a6, int16_t a7);

    void * operator new (size_t );

    void operator delete(void *ptr, size_t size);

    //0x00618610
    //virtual
    void AddEntry(int a2, FEText *a3, bool a4);

    /* virtual */ void AddEntry(int a2, global_text_enum a3);

    /* virtual */ void Init();

    //virtual
    void Load();

    //virtual
    void Draw();

    /* virtual */ void Update(Float a2);

    /* virtual */ void OnActivate();

    /* virtual */ void OnDeactivate(FEMenu *a2);

    //virtual
    void OnSelect(int a2);

    //virtual
    void OnStart(int a2);

    //virtual
    void OnUp(int a2);

    //virtual
    void OnDown(int a2);

    //virtual
    void OnLeft(int a2);

    //virtual
    void OnRight(int a2);

    //virtual
    void OnCross(int a2);

    //virtual
    void OnTriangle(int a2);

    //virtual
    void OnSquare(int a2);

    //virtual
    void OnCircle(int a2);

    //virtual
    void OnL1(int);

    //virtual
    void OnR1(int);

    //virtual
    void OnL2(int);

    //virtual
    void OnR2(int);

    /* virtual */ void OnAnyButtonPress(int a2, int a3);

    /* virtual */ void OnButtonRelease(int a2, int a3);

    /* virtual */ void SetHigh(int a2, bool a3);

    /* virtual */ void SetVis(int a2);

    /* virtual */ void Up();

    /* virtual */ void Down();

    /* virtual */ void ButtonHeldAction();
};

extern void sub_582A30();

extern void sub_5A6D70();

extern void FEMenu_patch();
