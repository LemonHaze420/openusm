#include "thug_health.h"

#include "entity_base.h"
#include "common.h"
#include "panelfile.h"
#include "panelquad.h"
#include "trace.h"

VALIDATE_SIZE(thug_health, 0x364u);
VALIDATE_SIZE(thug_health::widget_instance, 0x1Cu);

thug_health::thug_health()
{
    this->field_0 = -1;

    this->field_4 = nullptr;

    for (int i = 0; i < 30; ++i) {
        this->field_1C[i].field_0 = false;
        this->field_1C[i].field_10 = {0};
    }
}

void thug_health::init()
{
    TRACE("thug_health::init");

    if (this->field_4 == nullptr) {
        this->field_4 = PanelFile::UnmashPanelFile("healthbar_thug", static_cast<panel_layer>(7));

        this->field_8 = this->field_4->GetPQ("thug_health_green");
        this->field_C = this->field_4->GetPQ("thug_health_yellow");
        this->field_10 = this->field_4->GetPQ("thug_health_red");
        this->field_14 = this->field_4->GetPQ("thug_health_skull");
        this->field_18 = this->field_4->GetPQ("thug_health_web");
    }
}


int thug_health::create()
{
    for (int index = 0; index < 30; ++index) {
        auto &widget = field_1C[index];
        if (!widget.field_0) {
            widget.field_10 = {0};
            widget.field_0 = true;
            widget.visible = true;
            widget.field_14 = 20;
            widget.field_18 = 0;
            return index;
        }
    }
    return field_0;
}


void thug_health::destroy(int index)
{
    if (static_cast<unsigned>(index) < 30 && field_1C[index].field_0)
        field_1C[index].field_0 = false;
}


void thug_health::set_entity(int index, entity_base *owner)
{
    if (static_cast<unsigned>(index) < 30 && field_1C[index].field_0 && owner != nullptr)
        field_1C[index].field_10 = owner->get_my_handle();
}

void thug_health_patch()
{
    {
        FUNC_ADDRESS(address, &thug_health::init);
        REDIRECT(0x00647E32, address);
    }
}
