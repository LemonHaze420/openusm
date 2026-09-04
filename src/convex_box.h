#pragma once

#include "vector3d.h"
#include "vector4d.h"

struct po;

struct bounding_box {
    vector3d field_0[2];

    bounding_box();

    vector3d size()
    {
        if (std::equal_to<float>{}(this->field_0[0][0], 3.4028235e38) ||
            std::equal_to<float>{}(this->field_0[1][0], -3.4028235e38)) {
            return ZEROVEC;
        }

        auto result = this->field_0[1] - this->field_0[0];

        return result;
    }
};

struct convex_box {
    vector4d field_0[6];

    bounding_box bbox;

    bool sub_55EDB0(const vector3d &a2, const vector3d &a3);

    bool sub_55EE20(const vector3d &a2, const po &a3);

    bool set_box_coords(const vector3d *a2);
};
