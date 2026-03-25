#pragma once

struct vector3d;

struct plane {
    float arr[4];

    plane() = default;

    //0x005BCC30
    plane(const vector3d &a2, const vector3d &a3);
};
