#include "fe_crosshair.h"

#include "panelfile.h"

fe_crosshair::fe_crosshair() {}

void fe_crosshair::Init()
{
    if (this->field_10 == nullptr) {
        this->field_10 = PanelFile::UnmashPanelFile("targeting_reticle_venom", static_cast<panel_layer>(7));

        this->field_14 = this->field_10->GetPQ("venom_target_reticle");
        this->field_C = 0;
    }
}

void fe_crosshair::DeInit()
{
    if (this->field_10 != nullptr) {
        this->field_10 = nullptr;
    }
}
