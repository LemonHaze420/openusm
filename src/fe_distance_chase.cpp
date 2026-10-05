#include "fe_distance_chase.h"

#include "common.h"
#include "panelfile.h"

VALIDATE_SIZE(fe_distance_chase, 0x70);

// Inlined at 0x00648CFD in IGOFrontEnd::IGOFrontEnd.
fe_distance_chase::fe_distance_chase() : panels{}, field_6C(false)
{
    field_28 = 7;
}

void fe_distance_chase::Init(int type_id, const char *a3)
{
    this->panels[type_id] = PanelFile::UnmashPanelFile(a3, static_cast<panel_layer>(7));
    this->field_1C = this->panels[type_id]->GetPQ("dm_spider_icon");

    this->field_20 = this->panels[type_id]->GetPQ("dm_venom_icon_chase");

    this->field_1C->GetPos(this->field_3C, this->field_2C);
    auto *v7 = this->panels[type_id]->GetPQ("dm_burst");
    v7->GetPos(this->field_4C, this->field_2C);

    std::memcpy(this->field_5C, this->field_3C, sizeof(this->field_3C));

    this->field_24 = 0;
    this->field_1C->TurnOn(true);
    this->field_20->TurnOn(false);
    this->field_28 = type_id;
}

// 0x0062FDE0
void fe_distance_chase::Update(Float time_inc)
{
    if (field_28 < 0 || field_28 > 6 || panels[field_28] == nullptr)
        return;
    PanelFile *panel = panels[field_28];
    if (!field_6C && (panel->field_28.empty() || !panel->field_28.at(0)->field_2D))
        return;
    panel->Update(time_inc);
    field_1C->SetPos(field_5C, field_2C);
    field_20->SetPos(field_5C, field_2C);
}

void fe_distance_chase::DeInit(int a2)
{
    this->panels[a2] = nullptr;
    if (a2 == this->field_28) {
        this->field_6C = 0;
        this->field_28 = 7;
    }
}
