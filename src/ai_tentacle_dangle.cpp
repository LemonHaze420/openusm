#include "ai_tentacle_dangle.h"

#include "common.h"
#include "func_wrapper.h"
#include "variable.h"

VALIDATE_SIZE(ai_tentacle_dangle, 0x24);

int &ai_tentacle_engine::id_counter = var<int>(0x0095807C);

ai_tentacle_dangle::ai_tentacle_dangle(ai_tentacle_info *a2) : ai_tentacle_engine(a2)
{
    this->m_vtbl = 0x0087EF4C;
    this->tentacle_dangler = nullptr;
    this->field_20 = true;
    this->field_21 = true;
}
