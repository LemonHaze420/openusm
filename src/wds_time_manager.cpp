#include "wds_time_manager.h"

#include "common.h"
#include "debug_render.h"
#include "variable.h"

#include <cstdint>

VALIDATE_SIZE(wds_time_manager, 32u);

wds_time_manager::wds_time_manager()
{
    field_14 = 0.5f;
    field_4 = 0.0f;
    field_1C = false;
    field_18 = -999.0f;
    field_0 = 1.0f;
    field_8 = 0.0f;
    field_C = 0;
    field_14 = 0.5f;
    field_10 = 0.033333335f;
}

void wds_time_manager::frame_advance(Float a2)
{
    this->field_18 = a2;
    this->field_4 += a2;
    this->field_10 = (1.0 - this->field_14) * this->field_10 + a2 * this->field_14;
    ++this->field_C;
    this->field_8 += a2;
    if (debug_render_get_ival(PAUSE_TIMERS)) {
        this->field_1C = true;
    } else if (this->field_1C) {
        this->field_1C = false;
    }
}

float wds_time_manager::get_level_time()
{
    return this->field_4;
}
