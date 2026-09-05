#pragma once

#include "float.hpp"

#include "mvector.h"

struct PanelAnim;
struct PanelFile;

struct PanelAnimFile {
    mVector<PanelAnim> field_0;
    float field_14;
    int field_18;
    int field_1C;
    float field_20;
    int field_24;
    int field_28;
    bool field_2C;
    bool field_2D;

    //0x00628A40
    void Update(Float a2);
    void Stop();

    //0x00617170
    void PostUnmashFixup(PanelFile *a1);
};

extern void PanelAnimFile_patch();
