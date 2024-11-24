#include "oldmath_po.h"

#include "common.h"
#include "custom_math.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "quaternion.h"
#include "trace.h"
#include "utility.h"
#include "variable.h"
#include "vector3d.h"
#include "vector4d.h"

#ifdef USE_GLM
#include <glm/gtx/string_cast.hpp>
#endif

#include <cmath>

VALIDATE_SIZE(po, 0x40);

po::po() {
    m[0][0] = 1.0;
    m[0][1] = 0.0;
    m[0][2] = 0.0;
    m[0][3] = 0.0;

    m[1][0] = 0.0;
    m[1][1] = 1.0;
    m[1][2] = 0.0;
    m[1][3] = 0.0;

    m[2][0] = 0.0;
    m[2][1] = 0.0;
    m[2][2] = 1.0;
    m[2][3] = 0.0;

    m[3][0] = 0.0;
    m[3][1] = 0.0;
    m[3][2] = 0.0;
    m[3][3] = 1.0;
}

po::po(float a2,
        float a3,
        float a4,
        float a5,
        float a6,
        float a7,
        float a8,
        float a9,
        float a10,
        float a11,
        float a12,
        float a13)
{
    auto &v13 = this->m[0];
    v13[0] = a2;
    v13[1] = a3;
    v13[2] = a4;
    v13[3] = 0.0;

    auto &v17 = this->m[1];
    v17[0] = a5;
    v17[1] = a6;
    v17[2] = a7;
    v17[3] = 0.0;

    auto &v21 = this->m[2];
    v21[0] = a8;
    v21[1] = a9;
    v21[2] = a10;
    v21[3] = 0.0;

    auto &v25 = this->m[3];
    v25[0] = a11;
    v25[1] = a12;
    v25[2] = a13;
    v25[3] = 1.0;
}

po::po(const matrix4x4 &a1) : m(a1) {
    assert(!has_nonuniform_scaling());
}

po::po(const vector3d &a1, const quaternion &a2, Float a3) {
    this->set_po(a1, a2, a3);
}

bool po::operator==(const po &a2) {
    return memcmp(this, &a2, 64u) == 0;
}

bool po::is_valid(bool a2, bool a3) const {
    if (!this->m[0].is_valid()) {
        return false;
    }

    if (!this->m[1].is_valid()) {
        return false;
    }

    if (!this->m[2].is_valid()) {
        return false;
    }

    if (!this->m[3].is_valid()) {
        return false;
    }

    if (!a2) {
        auto v7 = this->m[0].dot();
        if (v7 < EPSILON) {
            return false;
        }

        auto v8 = this->m[1].dot();
        if (v8 < EPSILON) {
            return false;
        }

        auto v9 = this->m[2].dot();
        if (v9 < EPSILON) {
            return false;
        }
    }

    if (!a3) {
        return true;
    }

    auto v4 = this->m[0].length();
    if (std::abs(v4 - 1.0) > LARGE_EPSILON) {
        return false;
    }

    auto v5 = this->m[1].length();
    if (std::abs(v5 - 1.0) > LARGE_EPSILON) {
        return false;
    }

    auto v6 = this->m[2].length();

    bool result = false;
    if (std::abs(v6 - 1.0) > LARGE_EPSILON) {
        result = false;
    } else {
        result = true;
    }

    return result;
}

void po::un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *)
{
    ;
}

po po::sub_4BAB00(const po &a3) {
    po result;
    THISCALL(0x004BAB00, this, &result, &a3);

    return result;
}

//vec4 = mat4 * vec4{vec3, 0};
vector3d po::non_affine_slow_xform(const vector3d &a3) {
    vector3d transformed;
    transformed[0] = this->m[2][0] * a3[2] + this->m[1][0] * a3[1] + this->m[0][0] * a3[0];
    transformed[1] = this->m[2][1] * a3[2] + this->m[1][1] * a3[1] + this->m[0][1] * a3[0];
    transformed[2] = this->m[2][2] * a3[2] + this->m[1][2] * a3[1] + this->m[0][2] * a3[0];

    if constexpr (0) {
        sp_log("\nmatrix: \n%s", this->to_string().c_str());
        sp_log("\npoint = %s", a3.to_string().c_str());
        sp_log("\ntransformed_point = %s", transformed.to_string().c_str());

#ifdef USE_GLM
        glm::mat4 mat{*bit_cast<glm::vec4 *>(&this->m[0]),
                      *bit_cast<glm::vec4 *>(&this->m[1]),
                      *bit_cast<glm::vec4 *>(&this->m[2]),
                      *bit_cast<glm::vec4 *>(&this->m[3])};
        glm::vec4 point{a3[0], a3[1], a3[2], 0.0f};

        auto transformed_point = mat * point;
        sp_log("%s", glm::to_string(transformed_point).c_str());
#endif

        assert(0);
    }

    return transformed;
}

vector3d po::non_affine_inverse_xform(const vector3d &a3) const
{
    assert(m[0][0] >= -1.01f && m[0][0] <= 1.01f);

    vector3d transformed;
    transformed[0] = this->m[0][0] * a3.x + this->m[0][1] * a3.y + this->m[0][2] * a3.z;
    transformed[1] = this->m[1][0] * a3.x + this->m[1][1] * a3.y + this->m[1][2] * a3.z;
    transformed[2] = this->m[2][0] * a3.x + this->m[2][1] * a3.y + this->m[2][2] * a3.z;

    if constexpr (0) {
        sp_log("\nmatrix: \n%s", this->to_string().c_str());
        sp_log("\npoint = %s", a3.to_string().c_str());
        sp_log("\ntransformed_point = %s", transformed.to_string().c_str());

#ifdef USE_GLM
        glm::mat4 mat{*bit_cast<glm::vec4 *>(&this->m[0]),
                      *bit_cast<glm::vec4 *>(&this->m[1]),
                      *bit_cast<glm::vec4 *>(&this->m[2]),
                      *bit_cast<glm::vec4 *>(&this->m[3])};
        glm::vec4 point{a3[0], a3[1], a3[2], 0.0f};

        auto transformed_point = point * mat;
        sp_log("%s", glm::to_string(transformed_point).c_str());
#endif

        assert(0);
    }

    return transformed;
}

matrix4x4 &po::get_matrix() {
    return this->m;
}

vector3d &po::get_x_facing() const {
    return *bit_cast<vector3d *>(&this->m[0]);
}

vector3d &po::get_y_facing() const {
    return *bit_cast<vector3d *>(&this->m[1]);
}

vector3d &po::get_z_facing() const {
    return *bit_cast<vector3d *>(&this->m[2]);
}

void po::set_rot(const vector3d &a2, Float a1) {
    if constexpr (1) {
        float a2a;
        float a3;
        fast_sin_cos_approx(a1, &a2a, &a3);

        auto v4 = a2[0];
        auto v5 = 1.f / sqrt(v4 * v4 + a2[1] * a2[1] + a2[2] * a2[2]);
        auto v6 = v5 * v4;

        auto v8 = v5 * a2[1];

        auto v19 = v5 * a2[2];
        auto a1a = 1.f - a3;
        this->m[0][0] = v6 * v6 * a1a + a3;
        auto v10 = v8 * v6 * a1a;
        auto v11 = v19 * a2a;
        this->m[0][1] = v10 - v11;
        auto v16 = v19 * v6 * a1a;
        auto v12 = v8 * a2a;

        this->m[0][2] = v12 + v16;
        this->m[0][3] = 0.0;

        this->m[1][0] = v11 + v10;
        this->m[1][1] = v8 * v8 * a1a + a3;
        this->m[1][3] = 0.0;

        auto v18 = v19 * v8 * a1a;

        auto v13 = v6 * a2a;
        this->m[1][2] = v18 - v13;
        this->m[2][0] = v16 - v12;
        this->m[2][1] = v13 + v18;
        this->m[2][2] = v19 * v19 * a1a + a3;
        this->m[2][3] = 0.0;

        this->m[3][0] = 0.0;
        this->m[3][1] = 0.0;
        this->m[3][2] = 0.0;
        this->m[3][3] = 1.0;

    } else {
        THISCALL(0x005BD4D0, this, &a2, a1);
    }
}

void po::set_rot(const vector3d &a2) {
    auto tmp = a2;

    auto a1 = tmp.length();

    float v3, v4;
    fast_sin_cos_approx(a1, &v3, &v4);

    if (std::abs(a1) > 0.0000099999997f) {
        tmp *= 1.f / a1;
    }

    auto v2 = 1.f - v4;

    this->m[0][0] = ((tmp[0] * tmp[0]) * (1.0 - v4)) + v4;
    this->m[0][1] = ((tmp[0] * tmp[1]) * v2) - (tmp[2] * v3);
    this->m[0][2] = ((tmp[0] * tmp[2]) * v2) + (tmp[1] * v3);
    this->m[0][3] = 0.0;

    this->m[1][0] = ((tmp[1] * tmp[0]) * v2) + (tmp[2] * v3);
    this->m[1][1] = ((tmp[1] * tmp[1]) * v2) + v4;
    this->m[1][2] = ((tmp[1] * tmp[2]) * v2) - (tmp[0] * v3);
    this->m[1][3] = 0.0;

    this->m[2][0] = ((tmp[2] * tmp[0]) * v2) - (tmp[1] * v3);
    this->m[2][1] = ((tmp[2] * tmp[1]) * v2) + (tmp[0] * v3);
    this->m[2][2] = ((tmp[2] * tmp[2]) * v2) + v4;
    this->m[2][3] = 0.0;

    this->m[3][0] = 0.0;
    this->m[3][1] = 0.0;
    this->m[3][2] = 0.0;
    this->m[3][3] = 1.0;
}

void po::set_po(const vector3d &pos, const quaternion &a3, Float a4) {
    a3.to_matrix(this->m);
    this->m.scale(a4.value);
    this->set_position(pos);
}

void po::set_rotate_x(Float a2) {
    this->m = identity_matrix;

    float v2, v1;
    fast_sin_cos_approx(a2, &v2, &v1);

    this->m[1][1] = v1;
    this->m[1][2] = v2;

    this->m[2][1] = v2;
    this->m[2][2] = v1;
}

void po::set_rotate_y(Float a2) {
    this->m = identity_matrix;

    float v2, v3;
    fast_sin_cos_approx(a2, &v2, &v3);

    this->m[0][0] = v3;
    this->m[0][2] = v2;
    this->m[2][0] = -v2;
    this->m[2][2] = v3;
}

void po::set_rotate_z(Float a2) {
    this->m = identity_matrix;

    float v3, v2;
    fast_sin_cos_approx(a2, &v3, &v2);

    this->m[0][0] = v2;
    this->m[0][1] = -v3;
    this->m[1][0] = v3;
    this->m[1][1] = v2;
}

void po::set_scale(const vector3d &scale) {
    this->m = identity_matrix;
    this->m[0][0] = scale[0];
    this->m[1][1] = scale[1];
    this->m[2][2] = scale[2];
}

void po::set_po(const vector3d &p_zdest, const vector3d &p_ydest, const vector3d &pos) {
    auto ydest = p_ydest.normalized();

    auto zdest = p_zdest.normalized();

    assert(ydest.is_valid() && ydest.length() >= 0.9f && ydest.length() <= 1.1f);

    assert(zdest.is_valid() && zdest.length() >= 0.9f && zdest.length() <= 1.1f);

    assert(pos.is_valid());

    assert(!is_colinear(zdest, ydest));

    vector3d xdest = vector3d::cross(ydest, zdest);
    xdest.normalize();

    assert(xdest.is_valid() && xdest.length() >= 0.9f && xdest.length() <= 1.1f);

    zdest = vector3d::cross(xdest, ydest);

    assert(zdest.is_valid() && zdest.length() >= 0.9f && zdest.length() <= 1.1f);

    this->m[0][0] = xdest[0];
    this->m[0][1] = xdest[1];
    this->m[0][2] = xdest[2];
    this->m[0][3] = 0.0;

    this->m[1][0] = ydest[0];
    this->m[1][1] = ydest[1];
    this->m[1][2] = ydest[2];
    this->m[1][3] = 0.0;

    this->m[2][0] = zdest[0];
    this->m[2][1] = zdest[1];
    this->m[2][2] = zdest[2];
    this->m[2][3] = 0.0;

    this->m[3][0] = pos[0];
    this->m[3][1] = pos[1];
    this->m[3][2] = pos[2];
    this->m[3][3] = 1.0;
}

void po::set_po(const vector3d &xdest,
                const vector3d &ydest,
                const vector3d &zdest,
                const vector3d &pos) {
    this->m[0] = xdest;
    this->m[0][3] = 0.0;

    this->m[1] = ydest;
    this->m[1][3] = 0.0;

    this->m[2] = zdest;
    this->m[2][3] = 0.0;

    this->m[3] = pos;
    this->m[3][3] = 1.0;
}

vector3d po::inverse_xform(const vector3d &a3) const
{
    assert(m[0][0] >= -1.01f && m[0][0] <= 1.01f);

    vector3d v = a3 - *bit_cast<vector3d *>(&this->m[3]);

    vector3d result;
    result[0] = v.x * this->m[0][0] + v.y * this->m[0][1] + v.z * this->m[0][2];
    result[1] = v.x * this->m[1][0] + v.y * this->m[1][1] + v.z * this->m[1][2];
    result[2] = v.x * this->m[2][0] + v.y * this->m[2][1] + v.z * this->m[2][2];

    if constexpr (0) {
        sp_log("\nmatrix: \n%s", this->to_string().c_str());
        sp_log("\npoint = %s", v.to_string().c_str());
        sp_log("\ntransformed_point = %s", result.to_string().c_str());

#ifdef USE_GLM
        glm::mat4 mat{*bit_cast<glm::vec4 *>(&this->m[0]),
                      *bit_cast<glm::vec4 *>(&this->m[1]),
                      *bit_cast<glm::vec4 *>(&this->m[2]),
                      *bit_cast<glm::vec4 *>(&this->m[3])};
        glm::vec4 point{v[0], v[1], v[2], 0.0f};

        auto transformed_point = point * mat;
        sp_log("%s", glm::to_string(transformed_point).c_str());
#endif

        assert(0);
    }

    return result;
}

bool po::has_nonuniform_scaling() const
{
    bool & g_ignore_nonuniform_scaling = var<bool>(0x0096852D);

    if (g_ignore_nonuniform_scaling) { 
        return false;
    }

    float v8[3];
    for (int i = 0; i < 3; ++i) {
        vector3d v6 = this->m[i];
        auto v3 = v6.length2();
        v8[i] = v3;
    }

    for (int i = 0; i < 3; ++i) {
        if ((v8[(i + 1) % 3] * 0.92000002) > v8[i] || v8[i] > (v8[(i + 1) % 3] * 1.08)) {
            return true;
        }
    }

    return false;
}

//vec4 = mat4 * vec4{vec3, 1}
vector3d po::slow_xform(const vector3d &a3) const {
    float x1 = this->m[2][0] * a3[2] + this->m[1][0] * a3[1] + this->m[0][0] * a3[0] +
        this->m[3][0];
    float y1 = this->m[2][1] * a3[2] + this->m[1][1] * a3[1] + this->m[0][1] * a3[0] +
        this->m[3][1];
    float z1 = this->m[2][2] * a3[2] + this->m[1][2] * a3[1] + this->m[0][2] * a3[0] +
        this->m[3][2];

    vector3d result = {x1, y1, z1};

    [[maybe_unused]] auto w = this->m[0][3] * a3[0] + (this->m[1][3] * a3[1]) + (this->m[2][3] * a3[2]) +
        this->m[3][3];

    if constexpr (0) {
        sp_log("\nmatrix: \n%s", this->to_string().c_str());
        sp_log("\npoint = %s", a3.to_string().c_str());
        sp_log("\ntransformed_point = %s", result.to_string().c_str());

#ifdef USE_GLM
        glm::mat4 mat{*bit_cast<glm::vec4 *>(&this->m[0]),
                      *bit_cast<glm::vec4 *>(&this->m[1]),
                      *bit_cast<glm::vec4 *>(&this->m[2]),
                      *bit_cast<glm::vec4 *>(&this->m[3])};
        glm::vec4 point{a3[0], a3[1], a3[2], 1.0f};

        auto transformed_point = mat * point;
        sp_log("%s", glm::to_string(transformed_point).c_str());
#endif

        assert(0);
    }

    //assert((w > 0.999f) && (w < 1.001f));

    return result;
}

vector3d po::sub_5BD2B0(const vector3d &a3) {
    float v3 = this->m[2][0] * a3[2] + this->m[1][0] * a3[1] + this->m[0][0] * a3[0];
    float v4 = this->m[2][1] * a3[2] + this->m[1][1] * a3[1] + this->m[0][1] * a3[0];
    float v5 = this->m[2][2] * a3[2] + this->m[1][2] * a3[1] + this->m[0][2] * a3[0];

    vector3d result;

    result[0] = v3;
    result[1] = v4;
    result[2] = v5;
    return result;
}

static po & po_result = var<po>(0x00969F28);

po *po::inverse() const {
    po_result.m[0][0] = this->m[0][0];
    po_result.m[0][1] = this->m[1][0];
    po_result.m[0][2] = this->m[2][0];
    po_result.m[1][0] = this->m[0][1];
    po_result.m[1][1] = this->m[1][1];
    po_result.m[1][2] = this->m[2][1];
    po_result.m[2][0] = this->m[0][2];
    po_result.m[2][1] = this->m[1][2];
    po_result.m[2][2] = this->m[2][2];

    po_result.m[3][0] = 0.0;
    po_result.m[3][1] = 0.0;
    po_result.m[3][2] = 0.0;
    po_result.m[3][3] = 1.0;

    po_result.m[0][3] = 0.0;
    po_result.m[1][3] = 0.0;
    po_result.m[2][3] = 0.0;

    vector3d a2 = po_result.sub_5BD2B0(this->m[3]);

    po_result.m[3] = -a2;

    return (&po_result);
}

void po::set_facing(const vector3d &a2) {
    vector3d z_facing = a2 - this->get_position();

    z_facing.normalize();

    if (z_facing[1] <= 0.99989998 || (z_facing[1] >= 1.0001)) {
        if (z_facing[1] >= -0.99989998 || (z_facing[1] <= -1.0001)) {
            vector3d v6{0, 1.f, 0};

            auto v5 = vector3d::cross(v6, z_facing);
            v5.normalize();

            v6 = vector3d::cross(z_facing, v5);
            v6.normalize();

            this->m[0][0] = v5[0];
            this->m[0][1] = v5[1];
            this->m[0][2] = v5[2];

            this->m[1][0] = v6[0];
            this->m[1][1] = v6[1];
            this->m[1][2] = v6[2];

            this->m[2][0] = z_facing[0];
            this->m[2][1] = z_facing[1];
            this->m[2][2] = z_facing[2];

        } else {
            this->m[0][0] = 1.0;
            this->m[0][1] = 0.0;
            this->m[0][2] = 0.0;

            this->m[1][0] = 0.0;
            this->m[1][1] = 0.0;
            this->m[1][2] = 1.0;

            this->m[2][0] = 0.0;
            this->m[2][1] = -1.0;
            this->m[2][2] = 0.0;
        }
    } else {
        this->m[0][0] = 1.0;
        this->m[0][1] = 0.0;
        this->m[0][2] = 0.0;

        this->m[1][0] = 0.0;
        this->m[1][1] = 0.0;
        this->m[1][2] = -1.0;

        this->m[2][0] = 0.0;
        this->m[2][1] = 1.0;
        this->m[2][2] = 0.0;
    }
}

void po::set_translate(const vector3d &a2)
{
    this->m = identity_matrix;
    this->m[3] = a2;
}

mString po::to_string() const {
    mString result{0,
                   "%f %f %f %f\n"
                   "%f %f %f %f\n"
                   "%f %f %f %f\n"
                   "%f %f %f %f\n",
                   m[0][0],
                   m[1][0],
                   m[2][0],
                   m[3][0],
                   m[0][1],
                   m[1][1],
                   m[2][1],
                   m[3][1],
                   m[0][2],
                   m[1][2],
                   m[2][2],
                   m[3][2],
                   m[0][3],
                   m[1][3],
                   m[2][3],
                   m[3][3]

    };

    return result;
}

void po::sub_48D840()
{
    THISCALL(0x0048D840, this);
}

po sub_48F770(const po &arg4, const po &a3)
{
    //TRACE("po::operator*");

    //sp_log("args: %s %s", arg4.to_string(), a3.to_string());

    const void * a2[2] {
        &a3,
        &arg4
    };

    po res;
    res.m.sub_415A30(a2);

    //sp_log("res: %s", res.to_string());
    return res;
}

matrix4x4 sub_507130(const TransformMatrices &arg4)
{
    matrix4x4 result;

    if constexpr (0)
    {
        vector4d a2, a3, a4, a5;
        arg4.decomposeAndProjectToScreen(a2, a3, a4, a5);

        vector4d a1a, v5, v4, v3;
        sub_4013C0(a1a, v5, v4, v3, a2, a3, a4, a5);

        result = matrix4x4 {v3, v5, v4, a1a};
    }
    else
    {
        CDECL_CALL(0x00507130, &result, &arg4);
    }

    return result;
}


void po_patch()
{
    {
        auto address = int(&sub_48F770);
        REDIRECT(0x0047B949, address);
        REDIRECT(0x0047C9AA, address);
        REDIRECT(0x004F1943, address);
        REDIRECT(0x004F1D3C, address);
    }

    if constexpr (0) {
        {
            FUNC_ADDRESS(address, &po::slow_xform);

            SET_JUMP(0x005BD230, address);
        }

        {
            FUNC_ADDRESS(address, &po::inverse_xform);

            SET_JUMP(0x0048AAC0, address);
        }

        {
            FUNC_ADDRESS(address, &po::non_affine_slow_xform);

            SET_JUMP(0x005BD2B0, address);
        }

        {
            void (po::*func)(const vector3d &, const vector3d &, const vector3d &) = &po::set_po;

            FUNC_ADDRESS(address, func);
            SET_JUMP(0x005BD050, address);
        }
    }
}
