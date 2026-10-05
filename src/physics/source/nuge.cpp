#include "nuge.h"

#include "rigid_body.h"
#include "phys_vector3d.h"
#include <cmath>

static Var<math::VecClass<3, 1>> stru_8BFDCC{0x008BFDCC};

static Var<math::VecClass<3, 0>> stru_8BFCA4{0x008BFCA4};

void nuge::calc_box_inertia(const phys_vector3d &dimensions,
    phys_vector3d &inertia, float &mass)
{
    mass = dimensions[0] * dimensions[1] * dimensions[2];
    const float factor = mass / 12.0f;
    const float x_squared = dimensions[0] * dimensions[0];
    const float y_squared = dimensions[1] * dimensions[1];
    const float z_squared = dimensions[2] * dimensions[2];
    inertia[0] = factor * (y_squared + z_squared);
    inertia[1] = factor * (x_squared + z_squared);
    inertia[2] = factor * (x_squared + y_squared);
}

void nuge::calc_velocities(const matrix4x4 &previous, const matrix4x4 &current,
    float elapsed,
    phys_vector3d &linear, phys_vector3d &angular)
{
    float rotation[3][3];
    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 3; ++column)
            rotation[row][column] = previous[0][row] * current[0][column] +
                previous[1][row] * current[1][column] +
                previous[2][row] * current[2][column];
    float quaternion[4];
    const float trace = rotation[0][0] + rotation[1][1] + rotation[2][2];
    if (trace >= -0.33333299f) {
        const float scale = 0.5f / std::sqrt(trace + 1.0f);
        quaternion[0] = (rotation[2][1] - rotation[1][2]) * scale;
        quaternion[1] = (rotation[0][2] - rotation[2][0]) * scale;
        quaternion[2] = (rotation[1][0] - rotation[0][1]) * scale;
        quaternion[3] = (trace + 1.0f) * scale;
    } else {
        const int axis = rotation[0][0] >= rotation[1][1]
            ? (rotation[2][2] < rotation[0][0] ? 0 : 2)
            : (rotation[2][2] >= rotation[1][1] ? 2 : 1);
        const int next = (axis + 1) % 3;
        const int last = (axis + 2) % 3;
        const float component = rotation[axis][axis] -
            rotation[next][next] - rotation[last][last] + 1.0f;
        const float scale = 0.5f / std::sqrt(component);
        quaternion[axis] = component * scale;
        quaternion[next] = (rotation[axis][next] + rotation[next][axis]) * scale;
        quaternion[last] = (rotation[axis][last] + rotation[last][axis]) * scale;
        quaternion[3] = (rotation[last][next] - rotation[next][last]) * scale;
    }
    const float t = std::fabs(quaternion[3]);
    float eased;
    if (t >= 0.5f) {
        const float falloff = std::sqrt(std::fabs((1.0f - t) * 0.5f));
        const float squared = falloff * falloff;
        const float cubed = falloff * squared;
        eased = cubed * squared * squared * -0.10796249657869339f -
            cubed * squared * 0.15000000596046448f -
            cubed * 0.3333333134651184f - 2.0f * falloff + 1.5707963267948966f;
    } else {
        const float squared = t * t;
        eased = squared * (t * squared * squared) * 0.0539812408387661f +
            t * squared * squared * 0.07500000298023224f +
            t * squared * 0.16666670143604279f + t;
    }
    if (quaternion[3] < 0.0f)
        eased = -eased;
    const float magnitude = std::sqrt(quaternion[0] * quaternion[0] +
        quaternion[1] * quaternion[1] + quaternion[2] * quaternion[2]);
    const float inverse_time = 1.0f / elapsed;
    if (magnitude <= EPSILON) {
        for (int axis = 0; axis < 4; ++axis)
            angular[axis] = 0.0f;
    } else {
        const float factor = (3.1415926535897932f - 2.0f * eased) *
            (-1.0f / magnitude) * inverse_time;
        for (int axis = 0; axis < 4; ++axis)
            angular[axis] = quaternion[axis] * factor;
    }
    for (int axis = 0; axis < 4; ++axis)
        linear[axis] = (current[3][axis] - previous[3][axis]) * inverse_time;
}

void nuge::calc_velocities(const matrix4x4 &previous, const matrix4x4 &current,
    const phys_vector3d &local_pivot, float elapsed,
    phys_vector3d &linear, phys_vector3d &angular)
{
    calc_velocities(previous, current, elapsed, linear, angular);
    const vector3d world_pivot{
        local_pivot[0] * current[0][0] + local_pivot[1] * current[1][0] + local_pivot[2] * current[2][0],
        local_pivot[0] * current[0][1] + local_pivot[1] * current[1][1] + local_pivot[2] * current[2][1],
        local_pivot[0] * current[0][2] + local_pivot[1] * current[1][2] + local_pivot[2] * current[2][2]};
    const auto pivot_velocity = vector3d::cross(
        vector3d{angular[0], angular[1], angular[2]}, world_pivot);
    for (int axis = 0; axis < 3; ++axis)
        linear[axis] += pivot_velocity[axis];
}

void nuge::get_ballistic_info(rigid_body *const *a1, int a2, math::VecClass<3, 1> *a3, math::VecClass<3, 0> *a4,
                              float *a5)
{
    *a3 = stru_8BFDCC();
    *a4 = stru_8BFCA4();

    *a5 = 0.0;
    for (auto i = 0; i < a2; ++i) {
        auto *v8 = a1[i];
        if (v8 != nullptr) {
            auto a2a = 1.f / v8->field_130;

            math::VecClass<3, 1> a3a;
            a3a[0] = a2a * v8->field_0[3][0];
            a3a[1] = a2a * v8->field_0[3][1];
            a3a[2] = a2a * v8->field_0[3][2];
            a3a[3] = a2a * v8->field_0[3][3];
            *a3 = *a3 + a3a;

            math::VecClass<3, 0> v21;
            v21[0] = a2a * v8->field_D0[0];
            v21[1] = a2a * v8->field_D0[1];
            v21[2] = a2a * v8->field_D0[2];
            v21[3] = a2a * v8->field_D0[3];
            *a4 = *a4 + v21;

            *a5 = a2a + *a5;
        }
    }

    auto v10 = 1.f / *a5;

    *a3 = v10 * (*a3);
}
