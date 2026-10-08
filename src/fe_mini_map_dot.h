#pragma once

#include "mini_map_dot_type.h"
#include "vector3d.h"

struct PanelQuad;
struct matrix4x4;
struct nglQuad;

struct fe_mini_map_dot {
    PanelQuad *field_0;
    PanelQuad *field_4;
    nglQuad *field_8;
    nglQuad *field_C;
    nglQuad *field_10;
    vector3d field_14;
    mini_map_dot_type field_20;
    bool field_24;
    bool field_25;
    bool field_26;
    bool field_27;
    int highlight_circle_count_down;

    //0x0063AB90
    fe_mini_map_dot(mini_map_dot_type a2, vector3d a3);
    ~fe_mini_map_dot();

    //0x0060C5E0
    void Draw();
    void Update(const matrix4x4 &transform, const vector3d &hero_position, float sine, float left, float right,
                float top, float bottom);
};
