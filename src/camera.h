#pragma once

#include "actor.h"

struct vector3d;
struct string_hash;
struct entity;
struct mic;

struct camera : actor {
    mic *field_C0;
    float field_C4;
    float field_C8;

    //0x00577970
    camera(entity *a2, const string_hash &a3);

    void *operator new(size_t size);

    float compute_xz_projected_fov();

    bool is_externally_controlled() const;

    //0x00577BE0
    void get_look_and_up(vector3d *a2, vector3d *a3);

    //0x0057EFD0
    //virtual
    void sync(camera &a2);

    //virtual
    void set_fov(Float fov);

    /* virtual */ float get_fov();

    /* virtual */ float get_far_plane_factor();

    //0x00577AF0
    /* virtual */ void adjust_geometry_pipe(bool a2);

    //virtual
    bool _is_a_camera() const;
};

inline Var<camera *> g_camera_link{0x0095C720};

struct region;

extern vector3d collide_with_world(camera *, const vector3d &a3, float a2, const vector3d &arg10, region *reg);

extern void camera_patch();
