#include "point_trigger.h"

#include "common.h"
#include "trace.h"

VALIDATE_SIZE(point_trigger, 0x64);

point_trigger::point_trigger(string_hash a2, const vector3d &a3, Float a4) : trigger(a2)
{
    TRACE("point_trigger::point_trigger");

    this->m_vtbl = 0x00889F30;
    this->field_58 = a3;

    this->field_48 = a4;
}
