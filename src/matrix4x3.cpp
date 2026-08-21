#include "matrix4x3.h"

#include "custom_math.h"
#include "mstring.h"
#include "vector3d.h"

bool matrix4x3::operator==(const matrix4x3 &m)
{
    return (approx_equals(this->arr[0], m[0], LARGE_EPSILON) && approx_equals(this->arr[1], m[1], LARGE_EPSILON) &&
            approx_equals(this->arr[2], m[2], LARGE_EPSILON));
}

matrix4x3 matrix4x3::transposed() const
{
    matrix4x3 result{};

    result[0][0] = this->arr[0][0];
    result[0][1] = this->arr[1][0];
    result[0][2] = this->arr[2][0];
    result[0][3] = 0;

    result[1][0] = this->arr[0][1];
    result[1][1] = this->arr[1][1];
    result[1][2] = this->arr[2][1];
    result[1][3] = 0;

    result[2][0] = this->arr[0][2];
    result[2][1] = this->arr[1][2];
    result[2][2] = this->arr[2][2];
    result[2][3] = 0;

    return result;
}

vector4d operator*(const vector4d &a2, const matrix4x3 &a3)
{
    auto &x_axis = a3[0];
    auto &y_axis = a3[1];
    auto &z_axis = a3[2];

    return x_axis * a2.x + y_axis * a2.y + z_axis * a2.z;
}

const char *matrix4x3::to_string() const
{
    static char str[150]{};

    std::memset(str, 0, sizeof(str));

    sprintf(
        str, "mat4x3{%s, %s, %s}", arr[0].to_string().c_str(), arr[1].to_string().c_str(), arr[2].to_string().c_str());

    return str;
}
