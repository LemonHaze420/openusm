#pragma once

#include "vector3d.h"

struct entity_base;

struct quick_anchor_info {
    vector3d m_position;
    vector3d m_normal;
    vector3d field_18;
    float field_24;
    entity_base *field_28;
    entity_base *field_2C;

    //0x0048BBB0
    quick_anchor_info() = default;

    quick_anchor_info(const quick_anchor_info &) = default;

    quick_anchor_info &operator=(const quick_anchor_info &a2) = default;

    bool operator<(const quick_anchor_info &a2);
};

template <typename T>
void sort(T *begin, T *end, int size);

extern void quick_anchor_info_patch();
