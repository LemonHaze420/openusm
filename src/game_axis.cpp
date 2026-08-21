#include "game_axis.h"

#include "common.h"
#include "func_wrapper.h"
#include "input_mgr.h"
#include "trace.h"

VALIDATE_SIZE(game_axis, 0x30);

game_axis::game_axis()
{
    this->field_2C = false;
    this->field_2D = false;
    this->field_0 = input_mgr::instance->field_58;
    this->field_8 = 0.0;
    this->field_4 = 5;

    this->clear();
}

void game_axis::clear()
{
    this->field_2D = true;
    this->field_10 = 0.0;
    this->field_C = 0.0;
    this->field_18 = 0.0;
    this->m_timeout = default_timeout;
    this->m_threshold = default_threshold;
    this->field_1C = 0.0;
    this->field_28 = 0;
    this->field_24 = 0;
    this->field_2C = false;
}

void game_axis::update(Float a2)
{
    TRACE("game_axis::update");

    sp_log("%d, %d", this->field_4, this->field_0);

    if constexpr (0) {
    } else {
        THISCALL(0x0051D900, this, a2);
    }
}
