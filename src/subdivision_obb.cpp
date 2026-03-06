#include "subdivision_obb.h"

#include "common.h"
#include "func_wrapper.h"
#include "osassert.h"
#include "utility.h"
#include "vector4d.h"

#include <cassert>

VALIDATE_SIZE(subdivision_node_obb_base, 0x16);

VALIDATE_SIZE(subdivision_node_aabb, 0x1C);
VALIDATE_OFFSET(subdivision_node_aabb, m_size, 0x16);

VALIDATE_SIZE(subdivision_node_obb, 0x2E);
VALIDATE_OFFSET(subdivision_node_obb, x_length, 0x28);

VALIDATE_SIZE(subdivision_node_large_aabb, 0x24);
VALIDATE_OFFSET(subdivision_node_large_aabb, m_size, 0x18);

VALIDATE_SIZE(subdivision_node_large_obb, 0x48);
VALIDATE_OFFSET(subdivision_node_large_obb, x_axis, 0x18);
VALIDATE_OFFSET(subdivision_node_large_obb, x_length, 0x3C);

subdivision_node_obb_base::subdivision_node_obb_base() {}

void subdivision_node_obb_base::init(uint16_t flags_arg) {
    this->flags = flags_arg;
    memset(this->terrain_type_info, 0, 3);
    this->center = ZEROVEC;
    this->set_type(UNDEFINED_NODE);
}

bool subdivision_node_obb_base::point_inside_or_on(const vector3d &a2) const
{
    if constexpr (1)
    {
        vector4d v15, v24, a4, a6;

        auto v3 = this->unpack_xform(v15, v24, a4, a6);
        auto v4 = this->center[1];

        float v13[4]{};
        v13[0] = this->center[0];
        auto v5 = this->center[2];
        v13[1] = v4;
        v13[2] = v5;

        vector4d v20;
        v20[1] = v4;
        v20[0] = v13[0];
        v20[3] = v13[3];
        v20[2] = v5;

        vector4d a3{};
        a3[0] = a2[0];
        auto v6 = a2[2];
        a3[1] = a2[1];
        v13[0] = a3[0];
        a3[0] = a3[0] - v20[0];
        v13[1] = a3[1];
        v13[2] = v6;
        v13[3] = a3[3];
        a3[1] = a3[1] - v4;
        a3[2] = v6 - v20[2];
        a3[3] = a3[3] - v20[3];

        if (v3)
        {
            auto v7 = sub_4126E0(v24, a3, a4, a3, a6, a3);
            auto v8 = v7[1];
            a3[0] = v7[0];
            auto v9 = v7[2];
            a3[1] = v8;
            auto v10 = v7[3];
            a3[2] = v9;
            a3[3] = v10;
        }

        return v15[0] >= std::abs(a3[0]) && v15[1] >= std::abs(a3[1]) && v15[2] >= std::abs(a3[2]);

    } else {
        return (bool) THISCALL(0x0052BD30, this, &a2);
    }
}

void subdivision_node_obb_base::get_extents(vector3d *min_extent, vector3d *max_extent)
{
    assert(min_extent != nullptr);
    assert(max_extent != nullptr);

    THISCALL(0x0052C580, this, min_extent, max_extent);
}

void subdivision_node_obb_base::get_vertices(vector3d *out) const
{
    THISCALL(0x00513100, this, out);
}

float *subdivision_node_obb_base::sub_564D50(float *a2) {
    return (float *) THISCALL(0x00564D50, this, a2);
}

float subdivision_node_obb_base::sub_52CA80() {
    float v2[4];

    this->sub_564D50(v2);
    return v2[0] * v2[1] * v2[2] * 8.0f;
}

bool subdivision_node_obb_base::unpack_xform(vector4d &a2, vector4d &a3, vector4d &a4, vector4d &a5) const
{
    if constexpr (0)
    {
        bool result;

        vector4d v43;

        constexpr auto flt_888B98 = 0.0010681315f;

        switch (this->get_type()) {
        case AABB_LEAF_NODE: {
            auto *self = bit_cast<subdivision_node_aabb *>(this);
            int v20 = self->m_size.y;
            int v44 = self->m_size.x;
            v43[2] = (float) self->m_size.z;
            auto v26 = (double) v44 * flt_888B98;
            a2[0] = v26;

            auto v21 = v43[2];
            a2[1] = v20 * flt_888B98;
            auto v38 = v21 * flt_888B98;
            auto v22 = v43[3];
            a2[2] = v38;
            auto v42 = v22 * flt_888B98;
            a2[3] = v42;
            result = false;
            break;
        }
        case AABB_LARGE_LEAF_NODE: {
            auto *self = bit_cast<subdivision_node_large_aabb *>(this);
            a2 = self->m_size.sub_48D010();
            result = false;
            break;
        }
        case OBB_LEAF_NODE: {
            auto *self = bit_cast<subdivision_node_obb *>(this);

            constexpr auto flt_87E694 = 0.000030517578f;

            auto v6 = (double) self->x_axis.x;
            auto v7 = (double) self->y_axis.x;
            v43[2] = (float) self->z_axis.x;

            a3[0] = v6 * flt_87E694;

            auto v8 = v43[2];
            a3[1] = v7 * flt_87E694;

            auto v9 = v43[3];
            a3[2] = v8 * flt_87E694;
            auto v40 = v9 * flt_87E694;
            a3[3] = v40;
            auto v10 = (double) self->x_axis.y;
            auto v11 = (double) self->y_axis.y;
            v43[2] = (float) self->z_axis.y;

            a4[0] = v10 * flt_87E694;
            auto v31 = v11 * flt_87E694;
            auto v12 = v43[2];
            a4[1] = v31;
            auto v37 = v12 * flt_87E694;
            auto v13 = v43[3];
            a4[2] = v37;
            auto v41 = v13 * flt_87E694;
            a4[3] = v41;
            auto v14 = self->x_axis.z;
            auto v15 = (double) self->y_axis.z;
            v43[2] = (float) self->z_axis.z;

            vector4d v25;
            v25[0] = v15 * flt_87E694;
            v25[1] = (double) v14 * flt_87E694;
            v25[2] = v43[2] * flt_87E694;
            v25[3] = v43[3] * flt_87E694;

            a5 = v25;
            auto v16 = self->y_length;
            auto v17 = (double) self->x_length;
            v43[2] = (float) self->z_length;
            v25[0] = v17 * flt_888B98;
            a2[0] = v25[0];
            v25[1] = (double) v16 * flt_888B98;
            auto v18 = v43[2];
            a2[1] = v25[1];
            v25[2] = v18 * flt_888B98;
            auto v19 = v43[3];
            a2[2] = v25[2];
            v25[3] = v19 * flt_888B98;
            a2[3] = v25[3];

            result = true;
            break;
        }
        case OBB_LARGE_LEAF_NODE:
        case AUDIO_OBB_LEAF_NODE: {
            auto *self = bit_cast<subdivision_node_large_obb *>(this);
            a3[0] = self->x_axis[0];
            a3[1] = self->y_axis[0];
            a3[2] = self->z_axis[0];
            a3[3] = 1.f;

            a4[0] = self->x_axis[1];
            a4[1] = self->y_axis[1];
            a4[2] = self->z_axis[1];
            a4[3] = 1.f;

            a5[0] = self->x_axis[2];
            a5[1] = self->y_axis[2];
            a5[2] = self->z_axis[2];
            a5[3] = 1.f;
            a2 = bit_cast<vector3d *>(&self->x_length)->sub_48D010();
            result = true;
            break;
        }
        default:

            result = false;
            break;
        }

        return result;

    } else {
        return (bool) THISCALL(0x00564E80, this, &a2, &a3, &a4, &a5);
    }
}

bool sub_562450(const vector4d &a1, const vector4d &a2, const vector4d &a3) {
    return (bool) CDECL_CALL(0x00562450, &a1, &a2, &a3);
}

bool subdivision_node_obb_base::line_segment_intersection(const vector3d &arg0,
                                                          const vector3d &arg4)
{
    if constexpr (1) {
        if ((this->flags & 0x101) != 0) {
            return false;
        }

        vector4d v29;
        vector4d a2;
        vector4d a4;
        vector4d a6;

        auto v5 = this->unpack_xform(v29, a2, a4, a6);
        auto v6 = this->center[1];

        vector4d a1;
        a1[0] = this->center[0];

        auto v7 = this->center[2];
        a1[1] = v6;
        a1[2] = v7;

        a1[0] = arg0[0];

        auto v9 = arg0[2];
        a1[1] = arg0[1];

        vector4d a5;
        a5[0] = a1[0];
        a1[2] = v9;

        vector4d a3;
        a3[0] = a1[0] - a1[0];
        a5[1] = a1[1];
        a5[2] = v9;
        a5[3] = a1[3];
        a3[1] = v6 - a1[1];
        a5[0] = arg4[0];

        auto v10 = arg4[1];
        auto v11 = arg4[2];
        a3[2] = v7 - a1[2];
        a1[0] = a5[0];
        a3[3] = a1[3] - a1[3];
        a1[1] = v10;
        a1[2] = v11;
        a5[0] = a5[0] - a1[0];
        a5[1] = v10 - v6;
        a5[2] = v11 - v7;
        a5[3] = a3[3];

        if (v5) {
            auto v12 = sub_4126E0(a2, a3, a4, a3, a6, a3);

            a3[0] = v12[0];
            a3[1] = v12[1];
            a3[2] = v12[2];
            a3[3] = v12[3];

            auto v16 = sub_4126E0(a2, a5, a4, a5, a6, a5);
            auto v17 = v16[1];
            a5[0] = v16[0];
            auto v18 = v16[2];
            a5[1] = v17;
            auto v19 = v16[3];
            a5[2] = v18;
            a5[3] = v19;
        }

        return sub_562450(a3, a5, v29);

    } else {
        return THISCALL(0x0052B880, this, &arg0, &arg4);
    }
}

bool subdivision_node_obb_base::line_segment_intersection(
        const vector3d &a1,
        const vector3d &a2,
        vector3d *a3,
        vector3d *a4,
        float *a5,
        bool a6)
{
    return THISCALL(0x00538D60, this, a1, a2, a3, a4, a5, a6);
}

bool subdivision_node_obb_base::sphere_intersection(
    const vector3d &arg0, Float arg4, vector3d *arg8, vector3d *argC, float *arg10) {
    return (bool) THISCALL(0x0052C180, this, &arg0, arg4, arg8, argC, arg10);
}

bool subdivision_node_obb_base::sphere_intersection(const vector3d &center, Float radius) {
    if constexpr (0) {
        if ((this->flags & 0x101) != 0) {
            return false;
        }

        vector4d v22, a2, a4, a6;
        auto v5 = this->unpack_xform(v22, a2, a4, a6);
        auto v6 = this->center[1];
        auto v14 = this->center[0];
        auto v7 = this->center[2];
        [[maybe_unused]] auto v15 = v6;
        [[maybe_unused]] auto v16 = v7;

        vector4d a1{};
        a1[1] = v6;
        a1[0] = v14;
        a1[3] = 0.0;
        a1[2] = v7;

        vector4d a3{};
        a3[0] = center[0];
        auto v8 = center[2];
        a3[1] = center[1];
        v14 = a3[0];
        a3[0] = a3[0] - a1[0];
        v15 = a3[1];
        v16 = v8;
        [[maybe_unused]] auto v17 = a3[3];
        a3[1] = a3[1] - v6;
        a3[2] = v8 - a1[2];
        a3[3] = a3[3] - a1[3];
        if (v5)
        {
            auto v9 = sub_4126E0(a2, a3, a4, a3, a6, a3);

            a3[0] = v9[0];
            a3[1] = v9[1];
            a3[2] = v9[2];
            a3[3] = v9[3];
        }

        auto v13 = radius * radius;
        return sub_55F1D0(v22, a3, v13);

    } else {
        return (bool) THISCALL(0x0052C440, this, &center, radius);
    }
}

bool subdivision_node_obb_base::find_closest_point_on_visible_faces(
    const vector3d &sweet_spot,
    const vector3d &ent_pos,
    fixed_vector<obb_closest_point_entry_t, 3> *results) {
    assert(results != nullptr);
    assert(results->size() == 0);

    if constexpr (0) {
#if 0
        
        if ((this->flags & 0x101) != 0) {
            return false;
        }

        vector4d v64, a2, a4, a6;
        auto v6 = this->unpack_xform(v64, a2, a4, a6);
        auto v7 = this->center[0];
        auto v8 = this->center[1];
        auto v9 = v6;
        auto v10 = !v6;

        vector4d a8;
        a8[2] = this->center[2];

        vector4d v59{};
        v59[2] = a8[2];
        a8[0] = v7;
        v59[0] = v7;
        a8[1] = v8;
        v59[1] = v8;

        auto v11 = sweet_spot[0];
        a8[3] = v59[3];

        auto v12 = sweet_spot[1];
        v59[0] = v11;

        auto v13 = sweet_spot[2];

        vector4d v62{};
        v62[0] = v59[0];
        v62[1] = v12;
        v62[2] = v13;

        vector4d a3{};
        a3[0] = v59[0] - a8[0];
        v62[3] = v59[3];
        v59[1] = v12;
        v62[0] = ent_pos[0];
        a3[1] = v12 - a8[1];
        v59[2] = v13;

        auto v14 = ent_pos[2];
        v62[1] = ent_pos[1];
        v62[2] = v14;
        a3[2] = v13 - a8[2];
        v59[0] = v62[0];
        v62[3] = v59[3];

        char v57[19]{};
        v57[0] = v9;

        v59[1] = v62[1];
        a3[3] = v59[3] - v59[3];
        v59[2] = v14;
        auto a5 = v62[0] - a8[0];
        auto v68 = v62[1] - a8[1];
        auto v69 = v14 - a8[2];
        auto v70 = a3[3];

        vector4d arg4a{};
        vector4d v72{};
        if (!v10) {
            auto v15 = vector4d::sub_4126E0(a2, a3.arr, a4, a3.arr, a6, a3.arr);
            auto v16 = v15[1];
            a3[0] = v15[0];
            auto v17 = v15[2];
            a3[1] = v16;
            auto v18 = v15[3];
            a3[2] = v17;
            a3[3] = v18;

            auto v19 = vector4d::sub_4126E0(a2, &a5, a4, &a5, a6, &a5);
            auto v20 = v19[1];
            a5 = v19[0];
            auto v21 = v19[2];
            v68 = v20;
            auto v22 = v19[3];
            v69 = v21;
            v70 = v22;
            arg4a[0] = a2[0];
            arg4a[1] = a4[0];
            arg4a[2] = a6[0];
            v72[0] = a2[1];
            v72[1] = a4[1];
            v72[2] = a6[1];

            v62[0] = a2[2];
            v62[1] = a4[2];
            v62[2] = a6[2];
        }

        vector4d a1{};
        a1[0] = v64[0] - std::abs(a5);
        a1[1] = v64[1] - std::abs(v68);
        a1[2] = v64[2] - std::abs(v69);
        a2[0] = -v64[0];
        a2[1] = -v64[1];
        a2[2] = -v64[2];
        a2[3] = -v64[3];

        auto v23 = vector4d::min(v64, a3);
        a3 = vector4d::max(a2, v23);
        a2 = sub_55DA40(&a5, &v64);
        int v24 = 0;
        v57[1] = a1[0] < 0.0f;
        v57[2] = a1[1] < 0.0f;
        if (a1[0] < 0.0f) {
            *(float *) &v57[7] = a3[1];
            *(float *) &v57[11] = a3[2];
            *(float *) &v57[15] = a3[3];
            *(float *) &v57[3] = a2[0];
            v64[0] = a2[0];
            v64[1] = a3[1];
            v64[2] = a3[2];
            v64[3] = a3[3];
            if (v9) {
                v25 = sub_413E90((math::VecClass__3_1 *) &v59,
                                 &arg4a,
                                 (float *) &v57[3],
                                 &v72,
                                 (float *) &v57[3],
                                 &v62,
                                 (float *) &v57[3],
                                 &a8);
                v26 = v25->field_0[1];
                v27 = v25->field_0[2];
                *(float *) &v57[3] = v25->field_0[0];
                v60 = arg4a;
            } else {
                sub_4119B0((float *) &v57[3], a8.base.arr);
                sub_56A8E0(&v60, (int) v57);
                v27 = *(float *) &v57[11];
                v26 = *(float *) &v57[7];
            }
            if (a5 < (double) float_NULL) {
                v28 = sub_5610A0(v59.base.arr, v60.base.arr);
                v29 = v28[1];
                v60.base.arr[0] = *v28;
                v30 = v28[2];
                v31 = v28[3];
                v60.base.arr[1] = v29;
                v60.base.arr[2] = v30;
                v60.field_C = v31;
            }
            results->m_data[0].field_0.arr[0] = *(float *) &v57[3];
            v32 = v60.base.arr[0];
            results->m_data[0].field_0.arr[2] = v27;
            v33 = v60.base.arr[2];
            results->m_data[0].field_0.arr[1] = v26;
            v59.base.arr[0] = v32;
            v9 = v57[0];
            v59.base.arr[1] = v60.base.arr[1];
            v59.base.arr[2] = v33;
            v34 = v60.base.arr[1];
            v59.field_C = v60.field_C;
            v35 = v59.base.arr[2];
            results->m_data[0].field_C.arr[0] = v59.base.arr[0];
            results->m_data[0].field_C.arr[1] = v34;
            results->m_data[0].field_C.arr[2] = v35;
            v24 = 1;
        }
        if (v57[2]) {
            *(float *) &v57[7] = a2.base.arr[1];
            *(float *) &v57[3] = a3.base.arr[0];
            *(float *) &v57[11] = a3.base.arr[2];
            *(float *) &v57[15] = a3.field_C;
            if (v57[1] &&
                (a4.base.arr[0] = v64.base.arr[0] - *(float *) &v57[3],
                 a4.base.arr[1] = v64.base.arr[1] - a2.base.arr[1],
                 a4.base.arr[2] = v64.base.arr[2] - *(float *) &v57[11],
                 a4.field_C = v64.field_C - *(float *) &v57[15],
                 a6 = a4,
                 a4.base.arr[0] * a4.base.arr[0] + a4.base.arr[2] * a4.base.arr[2] +
                         a4.base.arr[1] * a4.base.arr[1] <=
                     LARGE_EPSILON)) {
                v57[2] = 0;
            } else {
                v59.base.arr[0] = a3.base.arr[0];
                v59.base.arr[1] = *(float *) &v57[7];
                v59.base.arr[2] = a3.base.arr[2];
                v59.field_C = a3.field_C;
                if (v9) {
                    v36 = sub_413E90((math::VecClass__3_1 *) &a4,
                                     &arg4a,
                                     (float *) &v57[3],
                                     &v72,
                                     (float *) &v57[3],
                                     &v62,
                                     (float *) &v57[3],
                                     &a8);
                    v37 = v36->field_C;
                    v38 = v36->field_0[1];
                    v39 = v36->field_0[2];
                    *(float *) &v57[3] = v36->field_0[0];
                    *(float *) &v57[15] = v37;
                    v60 = v72;
                } else {
                    sub_4119B0((float *) &v57[3], a8.base.arr);
                    sub_56A9D0(&v60, (int) v57);
                    v39 = *(float *) &v57[11];
                    v38 = *(float *) &v57[7];
                }
                if (v68 < (double) float_NULL) {
                    v40 = sub_5610A0(a4.base.arr, v60.base.arr);
                    v41 = v40[1];
                    v60.base.arr[0] = *v40;
                    v42 = v40[2];
                    v43 = v40[3];
                    v60.base.arr[1] = v41;
                    v60.base.arr[2] = v42;
                    v60.field_C = v43;
                }
                v44 = results->m_data[v24].field_0.arr;
                *v44 = *(float *) &v57[3];
                v44[1] = v38;
                v45 = v60.base.arr[0];
                v44[2] = v39;
                a4.base.arr[0] = v45;
                v9 = v57[0];
                a4.base.arr[1] = v60.base.arr[1];
                a4.base.arr[2] = v60.base.arr[2];
                a4.field_C = v60.field_C;
                v46 = v60.base.arr[1];
                v44[3] = v45;
                v47 = a4.base.arr[2];
                v44[4] = v46;
                v44[5] = v47;
                ++v24;
            }
        }
        if (a1.base.arr[2] < (double) float_NULL) {
            *(float *) &v57[7] = a3.base.arr[1];
            *(float *) &v57[11] = a2.base.arr[2];
            *(float *) &v57[3] = a3.base.arr[0];
            *(float *) &v57[15] = a3.field_C;
            if (!v57[1] ||
                (a4.base.arr[0] = v64.base.arr[0] - *(float *) &v57[3],
                 a4.base.arr[1] = v64.base.arr[1] - *(float *) &v57[7],
                 a4.base.arr[2] = v64.base.arr[2] - a2.base.arr[2],
                 a4.field_C = v64.field_C - *(float *) &v57[15],
                 a6 = a4,
                 a4.base.arr[0] * a4.base.arr[0] + a4.base.arr[2] * a4.base.arr[2] +
                         a4.base.arr[1] * a4.base.arr[1] >
                     LARGE_EPSILON)) {
                if (!v57[2] ||
                    (a4.base.arr[0] = v59.base.arr[0] - *(float *) &v57[3],
                     a4.base.arr[1] = v59.base.arr[1] - *(float *) &v57[7],
                     a4.base.arr[2] = v59.base.arr[2] - a2.base.arr[2],
                     a4.field_C = v59.field_C - *(float *) &v57[15],
                     a6 = a4,
                     a4.base.arr[2] * a4.base.arr[2] + a4.base.arr[1] * a4.base.arr[1] +
                             a4.base.arr[0] * a4.base.arr[0] >
                         LARGE_EPSILON)) {
                    if (v9) {
                        v48 = sub_413E90((math::VecClass__3_1 *) &a1,
                                         &arg4a,
                                         (float *) &v57[3],
                                         &v72,
                                         (float *) &v57[3],
                                         &v62,
                                         (float *) &v57[3],
                                         &a8);
                        v49 = v48->field_0[0];
                        v50 = v48->field_0[1];
                        v51 = v48->field_0[2];
                        v52 = v48->field_C;
                        *(float *) &v57[3] = v49;
                        *(float *) &v57[15] = v52;
                        v60 = v62;
                    } else {
                        sub_4119B0((float *) &v57[3], a8.base.arr);
                        sub_56AA20(&v60, (int) v57);
                        v51 = *(float *) &v57[11];
                        v50 = *(float *) &v57[7];
                    }
                    if (v69 < (double) float_NULL) {
                        v53 = sub_5610A0(a1.base.arr, v60.base.arr);
                        v54 = v53[1];
                        v60.base.arr[0] = *v53;
                        v55 = v53[2];
                        v60.base.arr[1] = v54;
                        v56 = v53[3];
                        v60.base.arr[2] = v55;
                        v60.field_C = v56;
                    }
                    a1.base.arr[0] = *(float *) &v57[3];
                    a1.base.arr[2] = v51;
                    a1.field_C = *(float *) &v57[15];
                    a1.base.arr[1] = v50;
                    sub_560B90(&results->m_data[v24].field_0, a1.base.arr);
                    sub_560B90(&results->m_data[v24++].field_C, v60.base.arr);
                }
            }
        }
        results->m_size = v24;
        return v24 > 0;

#endif

    }
    else
    {
        return THISCALL(0x005391F0, this, &sweet_spot, &ent_pos, results);
    }
}

bool subdivision_node_obb_base::is_obb_node() const {
    return this->get_type() == 4 || this->get_type() == 5 || this->get_type() == 6 ||
        this->get_type() == 7 || this->get_type() == 8;
}

void subdivision_node_obb_base::unpack_axii(vector3d *axii) const
{
    THISCALL(0x00512980, this, axii);
}

subdivision_node_obb::subdivision_node_obb() {
    set_type( OBB_LEAF_NODE );
}

subdivision_node_aabb::subdivision_node_aabb() {
    this->set_type(AABB_LEAF_NODE);
}

void check_for_degeneracies(subdivision_node_obb_base *obb)
{
    vector3d a1[3] {};
    obb->unpack_axii(a1);

    for ( int i = 0; i < 3; ++i )
    {
        if ( dot(a1[i], a1[i]) <= 0.00019999999 )
        {
            auto func = [](subdivision_node_obb_base *self) -> vector3d
            {
                return self->center;
            };

            auto v3 = func(obb)[2];
            auto v2 = func(obb)[1];
            auto v1 = func(obb)[0];
            error("Data error: degenerate (thin) obb found at (%.2f, %.2f, %.2f).\n"
                "Please correct the obb in the MAX file corresponding to these coordinates.\n"
                "(This error can be ignored more or less safely)",
                v1,
                v2,
                v3);
        }
    }
}


bool subdivision_node_large_aabb::init(
        uint16_t a2,
        uint32_t terrain_type_info_arg,
        const vector3d &a4,
        const vector3d &a5)
{
    subdivision_node_obb_base::init(a2);

    this->set_type(AABB_LARGE_LEAF_NODE);
    this->center = a4;
    this->m_size = a5;

    assert(!( terrain_type_info_arg & 0xFF000000 ));

    std::memcpy(this->terrain_type_info, &terrain_type_info_arg, 3);

    check_for_degeneracies(this);

    return true;
}

bool subdivision_node_large_obb::init(
        uint16_t a2,
        uint32_t terrain_type_info_arg,
        const vector3d &a4,
        const vector3d &a5,
        const vector3d &a6,
        const vector3d &a7)
{
    subdivision_node_obb_base::init(a2);

    this->center = a4;
    this->x_axis = a5;
    this->y_axis = a6;
    this->z_axis = a7;

    this->x_length = this->x_axis.length();
    this->y_length = this->y_axis.length();
    this->z_length = this->z_axis.length();

    assert(x_length > EPSILON && y_length > EPSILON && z_length > EPSILON);

    this->x_axis /= this->x_length;

    this->y_axis /= this->y_length;

    this->z_axis /= this->z_length;

    assert(!( terrain_type_info_arg & 0xFF000000 ));

    std::memcpy(this->terrain_type_info, &terrain_type_info_arg, 3);

    check_for_degeneracies(bit_cast<subdivision_node_obb_base *>(this));

    return true;
}
