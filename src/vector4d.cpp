#include "vector4d.h"

#include "func_wrapper.h"
#include "oldmath_po.h"
#include "vector3d.h"

#ifdef USE_GLM
#include <glm/glm.hpp>
#endif

#include <cmath>

vector4d::vector4d(const vector3d &v) {
    x = v[0];
    y = v[1];
    z = v[2];
}

vector4d::vector4d(const vector3d &v, float a2) {
    x = v[0];
    y = v[1];
    z = v[2];
    w = a2;
}

bool vector4d::is_valid() const {
    return x > -1.0e30 && x < 1.0e30 && y > -1.0e30 &&
        y < 1.0e30 && z > -1.0e30 && z < 1.0e30 &&
        w > -1.0e30 && w < 1.0e30;
}

float vector4d::length2() const {
    return ((x * x) + (y * y) +
            (z * z) + (w * w));
}

float vector4d::length() const {
    return std::sqrt((x * x) + (y * y) +
                     (z * z) + (w * w));
}

vector4d vector4d::sub_41CF30() const {
    vector4d out;
    out[0] = this->x;
    out[1] = this->y;
    out[2] = this->z;
    out[3] = 0.0;

    return out;
}

vector4d vector4d::sub_74B1C0(Float a3)
{
    const auto &self = *this;
    vector4d result = self * a3;
    return result;
}

vector4d operator-(const vector4d &a2, const vector4d &a3)
{
    vector4d result;
    result[0] = a2[0] - a3[0];
    result[1] = a2[1] - a3[1];
    result[2] = a2[2] - a3[2];
    result[3] = a2[3] - a3[3];
    return result;
}

float vector4d::dot() const {
    return this->x * this->x + this->y * this->y + this->z * this->z +
        this->w * this->w;
}

float vector4d::dot(const vector4d &a1, const vector4d &a2) {
#ifndef USE_GLM
    return a1[3] * a2[3] + a1[2] * a2[2] + a1[1] * a2[1] + a1[0] * a2[0];
#else
    return glm::dot(glm::vec4{a1[0], a1[1], a1[2], a1[3]}, glm::vec4{a2[0], a2[1], a2[2], a2[3]});
#endif
}

void vector4d::sub_413530(const vector4d &x_axis, const vector4d &a3)
{
    auto &self = *this;
    self += x_axis * a3.x;
}

void vector4d::sub_411A50(const vector4d &y_axis, const vector4d &a3)
{
    auto &self = *this;
    self += y_axis * a3.y;
}

vector4d vector4d::sub_413E90(const vector4d &x_axis,
                              const vector4d &a2,
                              const vector4d &y_axis,
                              const vector4d &a3,
                              const vector4d &z_axis,
                              const vector4d &a7,
                              const vector4d &a8) {
    vector4d v14 = a8;

    v14.sub_413530(x_axis, a2);
    v14.sub_411A50(y_axis, a3);

    vector4d result = v14 + z_axis * a7.z;

    return result;
}

vector4d sub_4126E0(const vector4d &x_axis,
                      const vector4d &a3,
                      const vector4d &y_axis,
                      const vector4d &a5,
                      const vector4d &z_axis,
                      const vector4d &a7)
{
    vector4d v11 = x_axis * a3.x;
    v11.sub_411A50(y_axis, a5);

    vector4d result = v11 + z_axis * a7.z;
    return result;
}

bool sub_55F1D0(const vector4d &a1, const vector4d &a2) {
    return (bool) CDECL_CALL(0x0055F1D0, &a1, &a2);
}

vector4d sub_55DA40(const float *a2, const vector4d *a3) {
    vector4d result;
    CDECL_CALL(0x0055DA40, &result, a2, a3);

    return result;
}

void vector4d::operator+=(const vector4d &a3) {
    auto &self = *this;
    self[0] += a3[0];
    self[1] += a3[1];
    self[2] += a3[2];
    self[3] += a3[3];
}

void vector4d::operator*=(const vector4d &a3) {
    this->x *= a3[0];
    this->y *= a3[1];
    this->z *= a3[2];
    this->w *= a3[3];
}

vector4d operator*(const vector4d &a2, const vector4d &a3) {
    auto v5 = a2[0] * a3[0];
    auto v6 = a2[1] * a3[1];
    auto v7 = a2[2] * a3[2];
    auto v4 = a2[3] * a3[3];

    vector4d out;
    out[0] = v5;
    out[1] = v6;
    out[2] = v7;
    out[3] = v4;
    return out;
}

vector4d vector4d::max(const vector4d &a2, const vector4d &a3) {
    float w = (a2.w <= a3.w ? a3.w : a2.w);

    float z = (a2[2] <= a3[2] ? a3[2] : a2[2]);

    float y = (a2[1] <= a3[1] ? a3[1] : a2[1]);

    float x = (a2[0] <= a3[0] ? a3[0] : a2[0]);

    vector4d result;
    result[0] = x;
    result[1] = y;
    result[2] = z;
    result[3] = w;
    return result;
}

vector4d vector4d::min(const vector4d &a2, const vector4d &a3) {
    float w = (a2.w >= a3.w ? a3.w : a2.w);

    float z = (a2[2] >= a3[2] ? a3[2] : a2[2]);

    float y = (a2[1] >= a3[1] ? a3[1] : a2[1]);

    float x = (a2[0] >= a3[0] ? a3[0] : a2[0]);

    vector4d result;
    result[0] = x;
    result[1] = y;
    result[2] = z;
    result[3] = w;
    return result;
}

vector4d vector4d::floor(const vector4d &a2) {
    auto v8 = std::floor(a2[2]);
    auto v7 = std::floor(a2[1]);
    auto v6 = std::floor(a2[0]);
    auto v3 = std::floor(a2[3]);

    vector4d result;
    result[0] = v6;
    result[1] = v7;
    result[2] = v8;
    result[3] = v3;

    return result;
}

vector4d vector4d::ceil(const vector4d &a2) {
    auto v8 = std::ceil(a2[2]);
    auto v7 = std::ceil(a2[1]);
    auto v6 = std::ceil(a2[0]);
    auto v3 = std::ceil(a2[3]);

    vector4d result;
    result[0] = v6;
    result[1] = v7;
    result[2] = v8;
    result[3] = v3;

    return result;
}

vector4d operator*(const vector4d &a2, float a3) {
    auto v5 = a3 * a2[0];
    auto v6 = a3 * a2[1];
    auto v7 = a3 * a2[2];
    auto v3 = a3 * a2[3];

    vector4d out;
    out[0] = v5;
    out[1] = v6;
    out[2] = v7;
    out[3] = v3;

    return out;
}

void vector4d::operator*=(float a3)
{
    this->x *= a3;
    this->y *= a3;
    this->z *= a3;
    this->w *= a3;
}

vector4d operator-(const vector4d &a2) {
    vector4d out;
    out[0] = -a2[0];
    out[1] = -a2[1];
    out[2] = -a2[2];
    out[3] = -a2[3];
    return out;
}

bool vector4d::operator==(const vector4d &a2) const
{
    return equal(this->x, a2.x)
            && equal(this->y, a2.y)
            && equal(this->z, a2.z)
            && equal(this->w, a2.w);
}

float AbsSquared(const vector4d &a1) {
    return a1.length2();
}
