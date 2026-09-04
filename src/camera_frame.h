#pragma once

#include "float.hpp"
#include "vector3d.h"

struct camera_target_info;
struct po;

struct camera_frame {
    vector3d eye;
    vector3d fwd;
    vector3d up;

    camera_frame(const po &a2);

    bool is_valid() const;

    vector3d fix_up_vector(vector3d upn) const;

    po get_po() const;

    vector3d get_right() const;

    void avoid_target(const camera_target_info &a1, Float a2);

    void rotate_to_include_target(vector3d a2, vector3d a5, vector3d a4, Float a11);

    void include_target(vector3d a2, Float a5, Float a6);

    void constrain_pos_relative_to_plane(const vector3d &a3, const vector3d &a1, Float a4, Float a5);

    void smooth_with(const camera_frame &a2, Float a3, Float a4);

    void smooth_dir(const camera_frame &a2, Float a3);
};

extern void sub_4B2680(vector3d &a1, float a2, vector3d a3);
