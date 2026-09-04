#pragma once

#include "float.hpp"
#include "matrix4x4.h"
#include "mstring.h"

struct generic_mash_header;
struct generic_mash_data_ptrs;
struct vector3d;
struct quaternion;
struct vector4d;

struct TransformMatrices;

struct po {
    matrix4x4 m;

    po();

    po(float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, float a10, float a11, float a12,
       float a13);

    po(const matrix4x4 &a1);

    po(vector3d const &, quaternion const &, Float);

    po(const po &) = default;
    po &operator=(const po &) = default;

    //0x005BD200
    bool operator==(const po &a2);

    bool is_valid(bool a2 = true, bool a3 = false) const;

    //0x005BE200
    bool has_nonuniform_scaling() const;

    //0x0048AAC0
    vector3d inverse_xform(const vector3d &a3) const;

    //0x005BD230
    vector3d slow_xform(const vector3d &a3) const;

    void un_mash(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3);

    po sub_4BAB00(const po &a3);

    void sub_48D840();

    //0x005BD2B0
    [[nodiscard]] vector3d non_affine_slow_xform(const vector3d &a3);

    //0x005BDE90
    [[nodiscard]] vector3d non_affine_inverse_xform(const vector3d &a3) const;

    matrix4x4 &get_matrix();

    vector3d &get_x_facing() const;

    vector3d &get_y_facing() const;

    vector3d &get_z_facing() const;

    //0x005BD4D0
    void set_rot(const vector3d &a2, Float a1);

    //0x005BD600
    void set_rot(const vector3d &a2);

    //0x005C76F0
    void set_po(const vector3d &pos, const quaternion &a3, Float a4);

    //0x005BD3E0
    void set_rotate_x(Float a2);

    //0x005BD430
    void set_rotate_y(Float a2);

    //0x005BD480
    void set_rotate_z(Float a2);

    //0x005BD350
    void set_scale(const vector3d &scale);

    //0x005BD050
    void set_po(const vector3d &a2, const vector3d &p_ydest, const vector3d &pos);

    //0x005BCFE0
    void set_po(const vector3d &xdest, const vector3d &ydest, const vector3d &zdest, const vector3d &pos);

    //0x005BDFE0
    void set_facing(const vector3d &a2);

    //0x005BD320
    void set_translate(const vector3d &a2);

    [[nodiscard]] vector3d sub_5BD2B0(const vector3d &a3);

    //0x005BD760
    po *inverse() const;

    inline void set_position(const vector3d &v)
    {
        constexpr auto idx = 3;

        this->m[idx] = v;
    }

    inline auto &get_position() const
    {
        constexpr auto idx = 3;

        return *(const vector3d *)&this->m[idx];
    }

    auto &operator[](int idx)
    {
        return m[idx];
    }

    const auto &operator[](int idx) const
    {
        return m[idx];
    }

    mString to_string() const;
};

inline const po po_identity_matrix{1.0, 0, 0, 0, 1.0, 0, 0, 0, 1.0, 0, 0, 0};

extern matrix4x4 sub_507130(const TransformMatrices &arg4);

extern po sub_48F770(const po &arg4, const po &a3);

extern void po_patch();
