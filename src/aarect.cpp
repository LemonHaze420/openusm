#include "aarect.h"

#include "vector2d.h"

template <>
bool aarect<float, vector2d>::sub_560880() const
{
    if (this->field_0[1][0] - this->field_0[0][0] > 0.0f) {
        auto v2 = this->field_0[1][1] - this->field_0[0][1];
        if (v2 > 0.0f) {
            return false;
        }
    }

    return true;
}
