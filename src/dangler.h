#pragma once

#include "float.hpp"
#include "vector3d.h"
#include <vector.hpp>

struct dangler {
    struct dangler_particle {
        int field_0;
        vector3d position;
        vector3d previous_position;
        vector3d velocity;
        vector3d acceleration;
        float rest_length;
        bool pinned;
        bool preserve_previous_position;
    };

    _std::vector<dangler_particle> *particles;
    int field_4;
    float damping;
    float length;
    vector3d gravity;
    int constraint_iterations;
    bool collision_enabled;
    bool field_21;

    dangler();
    ~dangler();
    static void *operator new(size_t size);
    static void operator delete(void *memory);
    void frame_advance(Float dt);
    void build_polytube(struct polytube *tube);

    int init_dangle(const vector3d &start, const vector3d &end, const vector3d *intermediate,
                    int intermediate_count, float total_length, const vector3d &velocity, char first_flags);

    void init_line(const vector3d &start, const vector3d &end, int segments,
                   const vector3d &velocity, char first_flags);
    void init_polytube(polytube *tube, const vector3d &velocity, char first_flags);
};
