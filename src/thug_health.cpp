#include "thug_health.h"

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

void thug_health_patch()
{
    {
        FUNC_ADDRESS(address, &thug_health::init);
        REDIRECT(0x00647E32, address);
    }
}
