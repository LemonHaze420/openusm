#include "targeting_reticle.h"

#include "common.h"
#include "panelfile.h"
#include "panelquad.h"
#include "trace.h"

VALIDATE_SIZE(targeting_reticle, 0x1Cu);

targeting_reticle::targeting_reticle()
{
    this->field_4 = 0;
    this->field_8 = nullptr;
    this->field_0 = false;
}

void targeting_reticle::init()
{
    if ( this->field_8 == nullptr )
    {
        this->field_8 = PanelFile::UnmashPanelFile("targeting_reticle", static_cast<panel_layer>(7));
        this->field_C = this->field_8->GetPQ("TR_arrow_top_left");
        this->field_10 = this->field_8->GetPQ("TR_arrow_top_right");
        this->field_14 = this->field_8->GetPQ("TR_arrow_bottom_left");
        this->field_18 = this->field_8->GetPQ("TR_arrow_bottom_right");
        this->field_4 = 0;
        this->field_0 = false;
    }
}
