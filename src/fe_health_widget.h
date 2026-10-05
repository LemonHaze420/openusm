#pragma once

#include "config.h"

struct PanelQuad;
struct PanelFile;

struct fe_health_widget {
    PanelFile *panels[12];
    int field_30;
    int number_of_types;
    int field_38;
    bool field_3C;
    PanelQuad *field_40;
    PanelQuad *field_44;
    PanelQuad *field_48;
    float field_4C;
    float field_50;
    bool field_54;
    bool field_55;
    bool field_56;

    fe_health_widget(int a1);

    //0x0061A3F0
    void SetShown(bool a2);


    void DrawAllPanels();

    //0x0061A5A0
    void UpdateMasking();

    //0x0063B170
    void clear_bars();

    void Init(int a2, const char *a3, bool a4);

    void DeInit(int a2);

    //0x00641BC0
    void SetType(int the_type, int source_hash_code);


    void set_poison_bar_precent(float percent);


    void set_regen_bar_shown(bool shown);


    void set_poison_bar_shown(bool shown);


    void set_health_bar_shown(bool shown);
};

extern void fe_health_widget_patch();
