#include "fe_health_widget.h"

#include "common.h"
#include "func_wrapper.h"
#include "panelfile.h"

VALIDATE_SIZE(fe_health_widget, 0x58);

fe_health_widget::fe_health_widget(int a1) {
    this->field_30 = 0;
    this->number_of_types = a1;

    for (auto i = 0; i < this->number_of_types; ++i) {
        this->panels[i] = nullptr;
    }

    this->field_54 = 0;
    this->field_55 = 0;
    this->field_38 = this->number_of_types;
    this->field_40 = nullptr;
    this->field_44 = nullptr;
    this->field_48 = nullptr;
    this->field_4C = 1.0;
    this->field_50 = 1.0;
}

void fe_health_widget::SetShown(bool a2) {
    THISCALL(0x0061A3F0, this, a2);
}

void fe_health_widget::UpdateMasking() {
    THISCALL(0x0061A5A0, this);
}

char fe_health_widget::clear_bars() {
    return static_cast<char>(THISCALL(0x0063B170, this));
}

void fe_health_widget::Init(int type_id, const char *a3, bool a4)
{
    TRACE("fe_health_widget::Init");

    assert(type_id >= 0 && type_id < this->number_of_types);

    this->panels[type_id] = PanelFile::UnmashPanelFile(a3, static_cast<panel_layer>(7));
    this->field_3C = a4;
    this->field_55 = false;
    this->clear_bars();
}

void fe_health_widget::DeInit(int a2) {
    this->panels[a2] = nullptr;
    if (a2 == this->field_38) {
        this->field_54 = 0;
        this->field_38 = this->number_of_types;
    }
}

void fe_health_widget_patch()
{
    REDIRECT(0x005718F4, func_address(&fe_health_widget::Init));
}
