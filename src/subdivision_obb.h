#pragma once

#include "subdivision_node.h"

#include <cstdint>

#include "fixed_vector.h"
#include "obb_closest_point_entry.h"
#include "subdivision_types.h"
#include "vector3d.h"

#pragma pack(push, 1)
struct subdivision_node_obb_base : subdivision_node {
    union {
        char terrain_type_info[3];
        uint16_t glass_box_number;
    };
    vector3d center;
    int visited;
    uint16_t flags;

    subdivision_node_obb_base();

    void init(uint16_t flags_arg);

    bool is_flagged(uint32_t a2) const
    {
        return a2 == (a2 & this->flags);
    }

    bool is_obb_node() const;

    //0x0052C580
    void get_extents(vector3d *min_extent, vector3d *max_extent);

    //0x00513100
    void get_vertices(vector3d *) const;

    float *sub_564D50(float *a2);

    float sub_52CA80();

    //0x00564E80
    bool unpack_xform(vector4d &a2, vector4d &a3, vector4d &a4, vector4d &a5) const;

    //0x0052B880
    bool line_segment_intersection(const vector3d &arg0, const vector3d &arg4);

    //0x00538D60
    bool line_segment_intersection(const vector3d &a1, const vector3d &a2, vector3d *a3, vector3d *a4, float *a5,
                                   bool a6);

    bool sphere_intersection(const vector3d &arg0, Float arg4, vector3d *arg8, vector3d *argC, float *arg10);

    //0x0052C440
    bool sphere_intersection(const vector3d &center, Float radius);

    //0x0052BD30
    bool point_inside_or_on(const vector3d &a2) const;

    //0x005391F0
    bool find_closest_point_on_visible_faces(const vector3d &sweet_spot, const vector3d &ent_pos,
                                             fixed_vector<obb_closest_point_entry_t, 3> *results);

    //0x00512980
    void unpack_axii(vector3d *axii) const;

    static inline Var<int> visit_key{0x0095C894};
};
#pragma pack(pop)

#pragma pack(push, 1)
struct subdivision_node_aabb : subdivision_node_obb_base {
    subdivision_node_aabb();

    subdivision_uint16_vector3d m_size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct subdivision_node_obb : subdivision_node_obb_base {
    subdivision_int16_vector3d x_axis;
    subdivision_int16_vector3d y_axis;
    subdivision_int16_vector3d z_axis;
    uint16_t x_length;
    uint16_t y_length;
    uint16_t z_length;


    subdivision_node_obb();
};
#pragma pack(pop)

#pragma pack(push, 1)
struct subdivision_node_large_aabb : subdivision_node_obb_base {
    char pad[2];
    vector3d m_size;


    //0x005136F0
    bool init(uint16_t a2, uint32_t a3, const vector3d &a4, const vector3d &a5);
};
#pragma pack(pop)


#pragma pack(push, 1)
struct subdivision_node_large_obb : subdivision_node_obb_base {
    char pad[2];
    vector3d x_axis;
    vector3d y_axis;
    vector3d z_axis;
    float x_length;
    float y_length;
    float z_length;

    //0x00513770
    bool init(uint16_t a2, uint32_t a3, const vector3d &a4, const vector3d &a5, const vector3d &a6, const vector3d &a7);
};
#pragma pack(pop)
