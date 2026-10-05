#include "rigid_body.h"

#include "common.h"
#include "func_wrapper.h"
#include "phys_vector3d.h"
#include "utility.h"
#include "physics_system.h"
#include <cmath>
#include <cstring>

VALIDATE_OFFSET(rigid_body, field_E0, 0xE0);

VALIDATE_OFFSET(rigid_body, field_130, 0x130);

VALIDATE_SIZE(user_rigid_body, 0x1B4);

namespace {
void rotate_basis(matrix4x4 &result, const matrix4x4 &pose, const vector4d &angular_velocity, float elapsed)
{
    const float speed =
        std::sqrt(angular_velocity[0] * angular_velocity[0] + angular_velocity[1] * angular_velocity[1] +
                  angular_velocity[2] * angular_velocity[2]);
    if (speed < 0.00001f) {
        for (int row = 0; row < 3; ++row)
            result[row] = pose[row];
        return;
    }
    const float x = angular_velocity[0] / speed;
    const float y = angular_velocity[1] / speed;
    const float z = angular_velocity[2] / speed;
    const float sine = std::sin(speed * elapsed);
    const float cosine = std::cos(speed * elapsed);
    const float complement = 1.0f - cosine;
    const float rotation[3][3]{
        {x * x * complement + cosine, x * y * complement + z * sine, x * z * complement - y * sine},
        {x * y * complement - z * sine, y * y * complement + cosine, y * z * complement + x * sine},
        {x * z * complement + y * sine, y * z * complement - x * sine, z * z * complement + cosine}};
    for (int row = 0; row < 3; ++row) {
        const float source[3]{pose[row][0], pose[row][1], pose[row][2]};
        for (int column = 0; column < 3; ++column)
            result[row][column] =
                source[0] * rotation[0][column] + source[1] * rotation[1][column] + source[2] * rotation[2][column];
        result[row][3] = 0.0f;
    }
}

vector4d transform_inertia(const rigid_body &body, const vector4d &vector, bool inverse)
{
    if ((body.field_144 & 2u) != 0) {
        const float factor = inverse ? body.field_B0 : 1.0f / body.field_B0;
        return vector4d{vector[0] * factor, vector[1] * factor, vector[2] * factor, 0.0f};
    }
    const float inertia[3]{body.field_B0, body.field_B4, body.field_B8};
    float local[3];
    for (int row = 0; row < 3; ++row) {
        local[row] =
            vector[0] * body.field_0[row][0] + vector[1] * body.field_0[row][1] + vector[2] * body.field_0[row][2];
        local[row] *= inverse ? inertia[row] : 1.0f / inertia[row];
    }
    vector4d result;
    for (int axis = 0; axis < 4; ++axis)
        result[axis] =
            local[0] * body.field_0[0][axis] + local[1] * body.field_0[1][axis] + local[2] * body.field_0[2][axis];
    return result;
}
}  // namespace

rigid_body::rigid_body() {}

void rigid_body::set(float mass, const phys_vector3d &inertia, const matrix4x4 &pose, const phys_vector3d &velocity,
                     const phys_vector3d &angular_velocity, float collision_scale, int collision_group)
{
    field_130 = 1.0f / mass;
    field_B0 = 1.0f / inertia[0];
    field_B4 = 1.0f / inertia[1];
    field_B8 = 1.0f / inertia[2];
    field_BC = 0.0f;
    field_0 = pose;
    field_D0 = vector4d{velocity[0], velocity[1], velocity[2], 0.0f};
    field_E0 = vector4d{angular_velocity[0], angular_velocity[1], angular_velocity[2], 0.0f};
    field_148 = collision_scale;
    field_160 = collision_group;
    std::memset(field_110, 0, sizeof(field_110));
    field_120 = field_124 = field_128 = field_12C = 0;
    std::memcpy(field_F0, &field_D0, sizeof(field_F0));
    std::memcpy(field_100, &field_E0, sizeof(field_100));
    field_134 = field_13C = field_140 = 1.0f;
    field_C0 = field_C8 = field_CC = 0.0f;
    field_C4 = -1.0f;
    field_144 = field_14C = field_164 = 0;
    field_138 = field_168 = 1000.0f;
    if ((g_physics_system->field_0 & 1) != 0)
        predict_pose(bit_cast<float>(g_physics_system->field_4));
}

void rigid_body::predict_pose(float elapsed)
{
    const float step = field_13C * elapsed;
    for (int axis = 0; axis != 4; ++axis)
        field_40[3][axis] = field_0[3][axis] + step * (field_D0[axis] + step * field_130 * field_110[axis]);
    rotate_basis(field_40, field_0, field_E0, step);
}

void rigid_body::prolog_frame_advance(float elapsed)
{
    const float step = elapsed * field_13C;
    const float angular_speed_squared =
        field_E0[0] * field_E0[0] + field_E0[1] * field_E0[1] + field_E0[2] * field_E0[2];
    if (angular_speed_squared > field_138 * field_138) {
        const float factor = field_138 / std::sqrt(angular_speed_squared) - 1.0f;
        vector4d correction;
        if ((field_144 & 2u) != 0) {
            for (int axis = 0; axis < 4; ++axis)
                correction[axis] = factor * field_E0[axis] / field_B0;
        } else {
            const float inverse_inertia[3]{field_B0, field_B4, field_B8};
            float local[3];
            for (int axis = 0; axis < 3; ++axis)
                local[axis] =
                    factor *
                    (field_E0[0] * field_0[axis][0] + field_E0[1] * field_0[axis][1] + field_E0[2] * field_0[axis][2]) /
                    inverse_inertia[axis];
            for (int axis = 0; axis < 4; ++axis)
                correction[axis] =
                    local[0] * field_0[0][axis] + local[1] * field_0[1][axis] + local[2] * field_0[2][axis];
        }
        auto *torque = reinterpret_cast<float *>(&field_120);
        for (int axis = 0; axis < 4; ++axis)
            torque[axis] += correction[axis];
    }
    const float inverse_step = 1.0f / step;
    auto *torque = reinterpret_cast<float *>(&field_120);
    for (int axis = 0; axis < 4; ++axis) {
        field_110[axis] *= inverse_step;
        torque[axis] *= inverse_step;
    }
    const float gravity = field_134 / field_130 * 9.800000190734863f;
    const float gravity_direction[4]{field_C0, field_C4, field_C8, field_CC};
    for (int axis = 0; axis < 4; ++axis)
        field_110[axis] += gravity * gravity_direction[axis];
}

void rigid_body::advance_forces(float elapsed)
{
    std::memcpy(field_F0, &field_D0, sizeof(field_F0));
    std::memcpy(field_100, &field_E0, sizeof(field_100));
    const float linear_scale = elapsed * field_130;
    vector4d angular_increment;
    const auto *torque = reinterpret_cast<const float *>(&field_120);
    if ((field_144 & 2u) != 0) {
        for (int axis = 0; axis < 4; ++axis)
            angular_increment[axis] = elapsed * field_B0 * torque[axis];
    } else {
        const auto *tensor = reinterpret_cast<const float *>(&field_80);
        for (int axis = 0; axis < 4; ++axis)
            angular_increment[axis] =
                elapsed * (torque[0] * tensor[axis] + torque[1] * tensor[4 + axis] + torque[2] * tensor[8 + axis]);
    }
    for (int axis = 0; axis < 4; ++axis) {
        field_D0[axis] += linear_scale * field_110[axis];
        field_E0[axis] += angular_increment[axis];
    }
}

void rigid_body::integrate(float elapsed)
{
    const auto momentum = transform_inertia(*this, field_E0, false);
    for (int axis = 0; axis < 4; ++axis)
        field_0[3][axis] += elapsed * field_D0[axis];
    rotate_basis(field_0, field_0, field_E0, elapsed);
    field_E0 = transform_inertia(*this, momentum, true);
    if (++field_14C > 5) {
        field_14C = 0;
        auto x = vector3d{field_0[0]};
        x *= 1.0f / x.length();
        auto y = vector3d{field_0[1]};
        y -= x * dot(x, y);
        y *= 1.0f / y.length();
        const auto z = vector3d::cross(x, y);
        field_0[0] = vector4d{x.x, x.y, x.z, 0.0f};
        field_0[1] = vector4d{y.x, y.y, y.z, 0.0f};
        field_0[2] = vector4d{z.x, z.y, z.z, 0.0f};
    }
    field_0[0][3] = field_0[1][3] = field_0[2][3] = 0.0f;
    field_0[3][3] = 1.0f;
}

void rigid_body::update_sleep(float elapsed)
{
    field_168 = field_D0[0] * field_D0[0] + field_D0[1] * field_D0[1] + field_D0[2] * field_D0[2] +
                field_E0[0] * field_E0[0] + field_E0[1] * field_E0[1] + field_E0[2] * field_E0[2];
    if (field_168 > 0.1224999949336052f) {
        field_164 = 0.0f;
        field_144 &= ~4u;
    } else if ((field_144 & 4u) == 0) {
        field_164 += elapsed;
        if (field_164 >= 0.5f && field_15C >= field_160)
            field_144 |= 4u;
    }
}

void rigid_body::update_world_inverse_inertia(const matrix4x4 &pose)
{
    const float inertia[3]{field_B0, field_B4, field_B8};
    auto *tensor = reinterpret_cast<float *>(&field_80);
    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 4; ++column)
            tensor[row * 4 + column] = pose[0][row] * inertia[0] * pose[0][column] +
                                       pose[1][row] * inertia[1] * pose[1][column] +
                                       pose[2][row] * inertia[2] * pose[2][column];
}

void user_rigid_body::predict_pose(float elapsed)
{
    const float step = elapsed * field_13C;
    for (int axis = 0; axis < 4; ++axis)
        field_40[3][axis] = field_0[3][axis] + step * field_D0[axis];
    rotate_basis(field_40, field_0, field_E0, step);
}

void user_rigid_body::integrate(float elapsed)
{
    for (int axis = 0; axis < 4; ++axis)
        field_0[3][axis] += elapsed * field_D0[axis];
    rotate_basis(field_0, field_0, field_E0, elapsed);
}

void rigid_body::sub_5B2D50(const phys_vector3d &force)
{
    for (int axis = 0; axis < 3; ++axis)
        field_110[axis] += force[axis];
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
