#include "ai_tentacle_engine.h"

#include "common.h"

VALIDATE_SIZE(ai_tentacle_engine, 0x1C);

ai_tentacle_engine::ai_tentacle_engine(ai_tentacle_info *a2)
{
    this->m_vtbl = 0x0087E9F0;
    this->field_10 = ++id_counter;
    if (this->field_10 == 0) {
        this->field_10 = ++id_counter;
    }

    this->field_14 = a2;
    this->field_18 = 0;
}
