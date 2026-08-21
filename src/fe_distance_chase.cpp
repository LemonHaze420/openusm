#include "fe_distance_chase.h"

#include "common.h"
#include "panelfile.h"

VALIDATE_SIZE(fe_distance_chase, 0x70);

fe_distance_chase::fe_distance_chase() {}

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

void fe_distance_chase::DeInit(int a2)
{
    this->panels[a2] = nullptr;
    if (a2 == this->field_28) {
        this->field_6C = 0;
        this->field_28 = 7;
    }
}
