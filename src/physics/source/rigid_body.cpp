#include "rigid_body.h"

#include "common.h"
#include "func_wrapper.h"
#include "phys_vector3d.h"
#include "utility.h"

VALIDATE_OFFSET(rigid_body, field_E0, 0xE0);

VALIDATE_OFFSET(rigid_body, field_130, 0x130);

VALIDATE_SIZE(user_rigid_body, 0x1B4);

rigid_body::rigid_body() {}

void rigid_body::sub_5B2D50(const phys_vector3d &a2)
{
    if constexpr (0) {
        math::VecClass<3, 0> v5;
        v5[0] = a2[0];
        v5[1] = a2[1];
        v5[2] = a2[2];

        auto *v3 = (math::VecClass<3, 0> *)this->field_110;

        math::VecClass<3, 0> v6;
        v6[0] = v5[0];
        v6[1] = v5[1];
        v6[2] = v5[2];
        v6[3] = v5[3];
        auto result = *bit_cast<math::VecClass<3, 0> *>(&this->field_110) + v6;
        *v3 = result;

    } else {
        sp_log("%f %f %f %f", field_110[0], field_110[1], field_110[2], field_110[3]);

        THISCALL(0x005B2D50, this, &a2);

        sp_log("%f %f %f %f", field_110[0], field_110[1], field_110[2], field_110[3]);

        assert(0);
    }
}

void rigid_body::sub_502600(const vector3d &a2)
{
    auto &v2 = this->field_D0;
    v2[0] = a2[0];
    v2[1] = a2[1];
    v2[2] = a2[2];
    v2[3] = 0.0f;
}

void rigid_body::sub_502640(const vector3d &a2)
{
    auto &v2 = this->field_110;

    v2[0] = a2[0];
    v2[1] = a2[1];
    v2[2] = a2[2];
    v2[3] = 0.0f;
}

vector3d rigid_body::sub_503B80()
{
    auto &v2 = this->field_110;

    vector3d result;
    result[0] = v2[0];
    result[1] = v2[1];
    result[2] = v2[2];
    return result;
}

void user_rigid_body::set(const math::MatClass<4, 3> *dictator)
{
    assert(dictator != nullptr);

    this->field_144 = 0;
    this->field_0 = *dictator;

    static Var<vector4d> stru_8BFAB8{0x8BFAB8};
    this->field_D0 = stru_8BFAB8();
    this->field_E0 = stru_8BFAB8();
    this->field_13C = 1.0;
    this->field_140 = 1.0;
    this->m_dictator = dictator;
    this->field_150 = 0;
    this->field_154 = 0;
    this->field_144 |= 0x20;
}

void rigid_body_patch()
{
    FUNC_ADDRESS(address, &rigid_body::sub_5B2D50);
    REDIRECT(0x007A09DD, address);
}
