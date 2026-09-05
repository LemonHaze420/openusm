#pragma once

#include "mvector.h"

#include "float.hpp"

struct PanelQuad;
struct PanelAnimKeyframe;
struct PanelFile;
struct matrix4x4;

struct PanelAnim {
    mVector<PanelAnimKeyframe> field_0;

    PanelQuad *field_14;
    bool field_18;

    PanelAnim();

    void PostUnmashFixup(PanelFile *pf);
    float GetVisibility(Float time) const;
    matrix4x4 GetXFormMatrix(Float time) const;
};
