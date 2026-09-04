#include "hull.h"

#include "common.h"

VALIDATE_SIZE(hull, 0x74);

void hull::add_face(const plane &a2)
{
    this->field_0.at(this->field_0.m_size++) = a2;
}

bool hull::sub_5CC030(Float a2, Float a3, Float a4, Float a5)
{
    auto size = this->field_0.m_size;

    if (size == 0) {
        return true;
    }

    for (auto i = 0u; i < size; ++i) {
        auto &p = this->field_0.at(i).arr;
        if (a2 * p[0] + a3 * p[1] + a4 * p[2] + p[3] + a5 < 0.0f) {
            return false;
        }
    }

    return true;
}
