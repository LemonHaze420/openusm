#include "threat_assessment_meters.h"

#include "common.h"
#include "fe_mini_map_dot.h"
#include "panelfile.h"
#include "panelquad.h"
#include "trace.h"

VALIDATE_SIZE(threat_assessment_meters, 0x1B8u);
VALIDATE_SIZE(threat_assessment_meters::tam_instance, 0x2Cu);

threat_assessment_meters::threat_assessment_meters()
{
    this->field_0 = -1;
    this->field_4 = -1;
    this->field_8 = 1.0;
    this->field_C = 0;
    this->field_10 = 0.30000001;

    this->field_14 = nullptr;
    this->field_134 = 0.0;
}

void threat_assessment_meters::init()
{
    TRACE("threat_assessment_meters::init");

    if ( !this->field_14 )
    {
        this->field_14 = PanelFile::UnmashPanelFile("tam_poi", static_cast<panel_layer>(7));
        this->field_18[0] = this->field_14->GetPQ("TAM_meter_icon");
        this->field_18[1] = this->field_14->GetPQ("TAM_meter_arrow");
        this->field_18[2] = this->field_14->GetPQ("TAM_meter_gauge_01");
        this->field_18[3] = this->field_14->GetPQ("TAM_meter_gauge_02");
        this->field_18[4] = this->field_14->GetPQ("TAM_meter_gauge_03");
        this->field_18[5] = this->field_14->GetPQ("TAM_meter_gauge_04");

        this->field_30 = this->field_14->field_28.at(0);
        auto v31 = this->field_18[0]->GetCenterX();
        auto v28 = this->field_18[0]->GetCenterY();

        for ( int i = 0; i < 6; ++i )
        {
            float v30;
            float v29;
            this->field_18[i]->GetCenterPos(v30, v29);

            this->field_34[i] = vector2d {v30 - v31, v29 - v28};
        }

        this->field_134 = 30.0f;

        for (int i = 0; i < 4; ++i)
        {
            auto *mem = mem_alloc(sizeof(fe_mini_map_dot));
            auto *icon = new (mem) fe_mini_map_dot {
                static_cast<mini_map_dot_type>(5),
                vector3d {0, 0, 0}
            };

            assert(icon != nullptr);

            icon->field_24 = false;
            this->field_64[i].field_1C = icon;
        }

        this->field_18[2]->sub_616690(this->field_138, this->field_178);
        this->field_18[3]->sub_616690(this->field_148, this->field_188);
        this->field_18[4]->sub_616690(this->field_158, this->field_198);
        this->field_18[5]->sub_616690(this->field_168, this->field_1A8);
    }
}

void threat_assessment_meters_patch()
{
    {
        FUNC_ADDRESS(address, &threat_assessment_meters::init);
        REDIRECT(0x00647E26, address);
    }
}
