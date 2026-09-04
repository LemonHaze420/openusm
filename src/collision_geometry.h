#pragma once

#include "vector3d.h"

#include <cstdint>

struct generic_mash_header;
struct generic_mash_data_ptrs;
struct actor;
struct po;

struct collision_geometry {
    static inline constexpr auto CAPSULE = 1;
    static inline constexpr auto MESH = 2;

    std::intptr_t m_vtbl;
    actor *owner;
    bool field_8;
    bool field_9;
    uint32_t field_C;

    //0x00519580
    collision_geometry();

    /* virtual */ ~collision_geometry() = default;

    /* virtual */ collision_geometry *make_instance(actor *)
    {
        return nullptr;
    } /* = 0 ;*/

    //0x005195C0
    /* virtual */ void xform(const po &a2);

    /* virtual */ void split_xform(const po &a1, const po &a2, const po &a3, int a4, int a5);

    /* virtual */ void split_xform(const po &a1, const po &a2, int a3);

    /* virtual */
    [[nodiscard]] float get_colgeom_radius();

    //0x00560370
    //virtual
    [[nodiscard]] vector3d get_local_space_bounding_sphere_center();

    /* virtual */ float get_bounding_sphere_radius();

    /* virtual */ float get_core_radius();

    /* virtual */ const vector3d &get_abs_position() const;

    /* virtual */ vector3d *get_pivot();

    /* virtual */ int get_type() /* = 0*/;

    /* virtual */
    bool is_pivot_valid();

    //0x005195D0
    /* virtual */ void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);

    //0x00502010
    /* virtual */ bool is_dynamic();
};
