#pragma once

#include "vector3d.h"

struct swing_anchor_finder;

struct sweet_cone_t {
    vector3d m_position;
    vector3d direction;
    vector3d sweet_spot;
    float field_24;

    //0x00451D20
    sweet_cone_t(const swing_anchor_finder *finder, const vector3d &pos, const vector3d &a3, const vector3d &a4,
                 const vector3d &a5);

    //0x00451F00
    bool contains_point(const vector3d &a2) const;

    const char *to_string() const;
};
