#pragma once

struct vector3d;

struct phys_vector3d {
    float field_0[3]{};

    phys_vector3d() = default;
    phys_vector3d(const vector3d &v);

    float &operator[](int idx)
    {
        return field_0[idx];
    }

    const float &operator[](int idx) const
    {
        return field_0[idx];
    }

    friend phys_vector3d operator*(float a1, const phys_vector3d &a2)
    {
        phys_vector3d result;

        result[0] = a1 * a2[0];
        result[1] = a1 * a2[1];
        result[2] = a1 * a2[2];

        return result;
    }
};
