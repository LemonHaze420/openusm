#include "environment_rigid_body.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(environment_rigid_body, 0x16C);

environment_rigid_body::environment_rigid_body() {}

void environment_rigid_body::set()
{
    if constexpr (0) {
        this->field_144 = 0;
        this->field_0 = matrix4x4{};
        this->field_D0 = vector4d{};
        this->field_E0 = vector4d{};

        this->field_150 = 0;
        this->field_154 = 0;
        this->field_144 |= 0x10;

    } else {
        THISCALL(0x007A5900, this);
    }
}
