#pragma once

#include "subdivision_visitor.h"

#include "float.hpp"
#include "vector3d.h"

struct region;

struct occlusion_visitor : subdivision_visitor {
    vector3d field_4;
    vector3d field_10;
    float field_1C;

    occlusion_visitor(const vector3d &a1, const vector3d &a3, Float a4, region *a5);


    int visit(const subdivision_node &node);
};
