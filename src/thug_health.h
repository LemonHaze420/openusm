#pragma once

#include "entity_base_vhandle.h"
#include "vector3d.h"

struct PanelFile;
struct PanelQuad;

struct thug_health {
    struct widget_instance {
        bool field_0;
        vector3d field_4;
        entity_base_vhandle field_10;
        int field_14;
        int field_18;

        widget_instance() = default;
    };

    int field_0;
    PanelFile *field_4;
    PanelQuad *field_8;
    PanelQuad *field_C;
    PanelQuad *field_10;
    PanelQuad *field_14;
    PanelQuad *field_18;
    widget_instance field_1C[30];

    //0x0061B530
    thug_health();

    //0x00644250
    void init();
};

extern void thug_health_patch();
