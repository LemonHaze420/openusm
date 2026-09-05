#pragma once

#include <cstdint>

#include "mash_virtual_base.h"
#include "float.hpp"
#include "panel_layer.h"

struct PanelAnimObject : mash_virtual_base {
    float field_4;
    float field_8;
    int field_C;
    char field_10;

    //0x00615D20
    PanelAnimObject();

    //0x00615D40
    PanelAnimObject(int);

    //virtual
    void Update(Float a2);

    /* virtual */ void StartAnim(bool a1);
    void ResetXform();

    //0x00401050
    //virtual
    double GetZvalue();

    //0x00607CC0
    //virtual
    void SetZvalue(Float a1, panel_layer a2);

    //0x0043A0D0
    //virtual
    bool IsShown();

    //virtual
    int get_mash_sizeof();

    //virtual
    void SetShown(bool a2);
};
