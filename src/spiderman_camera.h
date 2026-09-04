#pragma once

#include "game_camera.h"

#include "game_button.h"

struct camera_mode;

struct spiderman_camera : game_camera {
    camera_mode *field_1A0;
    vector3d target_pos;
    vector3d target_up;
    bool field_1BC;
    int field_1C0;
    int field_1C4;
    int field_1C8;
    bool field_1CC;
    game_button field_1D0;

    //0x004B78E0
    spiderman_camera(const string_hash &a2, entity *a3);

    void *operator new(size_t size);

    void operator delete(void *ptr);

    void set_fixedstatic(const vector3d &a2, const vector3d &a3);

    void sub_4B3260(bool a2);

    void sub_4B3220(const vector3d &a2);

    //0x004B6310
    //virtual
    void _sync(camera &a2);

    //0x004B6480
    /* virtual */ void adjust_geometry_pipe(bool a1);

    void autocorrect(Float a2);

    /* virtual */ void _autocorrect(Float a2);

    /* virtual */
    void _set_target_entity(entity *a1);

    //virtual
    void _frame_advance(Float a2);
};

extern float g_yaw_mult;

extern float g_pitch_mult;

extern Var<spiderman_camera *> g_spiderman_camera_ptr;

extern void constrain_normal(vector3d &normal, const vector3d &basisA, float a4, float a2);


extern void constrain_relative_to_plane(vector3d &a1, const vector3d &a2, const vector3d &norm, float a4, float a5);

extern void spiderman_camera_patch();
