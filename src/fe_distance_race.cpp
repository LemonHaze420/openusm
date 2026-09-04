#include "fe_distance_race.h"

#include "common.h"
#include "panelfile.h"

VALIDATE_SIZE(fe_distance_race, 0xC4);

fe_distance_race::fe_distance_race()
{
    this->field_0 = nullptr;
    this->field_4 = nullptr;
    this->field_8 = nullptr;
    this->field_C = nullptr;
    this->field_10 = nullptr;
    this->field_14 = nullptr;
    this->field_18 = nullptr;
    this->field_1C = nullptr;
    this->field_20 = nullptr;
    this->field_24 = nullptr;
    this->field_28 = nullptr;
    this->field_2C = nullptr;
    this->field_30 = nullptr;
    this->field_34 = nullptr;
    this->field_38 = nullptr;
    this->field_40 = nullptr;
    this->field_3C = nullptr;
    this->field_44 = nullptr;
    this->field_48 = nullptr;
    this->field_54 = nullptr;
    this->field_4C = nullptr;
    this->field_58 = nullptr;
    this->field_50 = nullptr;
    this->field_5C = nullptr;
    this->field_60 = nullptr;
    this->field_C0 = 0;
}

void fe_distance_race::Init()
{
    this->field_0 = PanelFile::UnmashPanelFile("race_meter", static_cast<panel_layer>(7));

    this->field_38 = this->field_0->GetPQ("rm_spidey_player_icon");

    this->field_3C = this->field_0->GetPQ("rm_venom_player_icon");

    this->field_40 = this->field_0->GetPQ("rm_spidey_player_back");

    this->field_44 = this->field_0->GetPQ("rm_venom_player_back");

    this->field_48 = this->field_0->GetPQ("rm_spidey_comp_icon");

    this->field_4C = this->field_0->GetPQ("rm_torch_comp_icon");

    this->field_50 = this->field_0->GetPQ("rm_venom_comp_icon");

    this->field_54 = this->field_0->GetPQ("rm_spidey_comp_back");

    this->field_58 = this->field_0->GetPQ("rm_torch_comp_back");

    this->field_5C = this->field_0->GetPQ("rm_venom_comp_back");

    this->field_4 = this->field_0->GetPQ("rm_back_01");

    this->field_8 = this->field_0->GetPQ("rm_back_02");

    this->field_C = this->field_0->GetPQ("rm_detail_01");

    this->field_10 = this->field_0->GetPQ("rm_detail_02");

    this->field_14 = this->field_0->GetPQ("rm_detail_03");

    this->field_18 = this->field_0->GetPQ("rm_detail_04");

    this->field_1C = this->field_0->GetPQ("rm_detail_05");

    this->field_20 = this->field_0->GetPQ("rm_detail_06");

    this->field_24 = this->field_0->GetPQ("rm_detail_07");

    this->field_28 = this->field_0->GetPQ("rm_detail_08");

    this->field_2C = this->field_0->GetPQ("rm_detail_09");

    this->field_30 = this->field_0->GetPQ("rm_detail_10");

    this->field_34 = this->field_0->GetPQ("rm_detail_11");

    this->field_60 = this->field_0->field_28.at(0);
    this->field_38->GetPos(this->field_90, this->field_70);
    this->field_48->GetPos(this->field_90, this->field_80);

    auto v52 = this->field_1C->GetCenterX();
    auto v47 = this->field_38->GetCenterX();
    this->field_64 = 0;
    this->field_6C = v52 - v47;
    this->field_68 = 0;

    this->field_A0[0] = this->field_90[0];
    this->field_B0[0] = this->field_90[0];

    this->field_A0[1] = this->field_90[1];
    this->field_B0[1] = this->field_90[1];

    this->field_A0[2] = this->field_90[2];
    this->field_B0[2] = this->field_90[2];

    this->field_A0[3] = this->field_90[3];
    this->field_B0[3] = this->field_90[3];
}

void fe_distance_race::DeInit()
{
    this->field_0 = nullptr;
}
