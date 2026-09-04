#include "gravity_generator.h"

#include "common.h"

VALIDATE_SIZE(gravity_generator, 0x8);

gravity_generator::gravity_generator()
{
    this->field_4 = false;
    this->field_5 = false;
    this->m_vtbl = 0x0088817C;
    this->field_4 = true;
}
