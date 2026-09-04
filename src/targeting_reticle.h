#pragma once

struct PanelFile;
struct PanelQuad;

struct targeting_reticle {
    bool field_0;
    int field_4;
    PanelFile *field_8;
    PanelQuad *field_C;
    PanelQuad *field_10;
    PanelQuad *field_14;
    PanelQuad *field_18;

    targeting_reticle();

    void init();
};
