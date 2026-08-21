#pragma once

#include "camera.h"

struct game_camera : camera {
    static inline constexpr auto CAMERA_SHAKES_TOTAL = 4u;

    struct _camera_shake_t {
        int field_0;
        int field_4;
        int field_8;
        int field_C;
        int empty[2];
        short field_18;

        char field_1A;

        _camera_shake_t();

        void clear();
    };

    float field_D0[16];
    int empty[2];

    vhandle_type<entity> field_118;
    vector3d field_11C;
    float field_128;
    bool field_12C;
    char pad[3];

    _camera_shake_t field_130[CAMERA_SHAKES_TOTAL];

    int empty1[1];

    game_camera() = default;

    game_camera(const string_hash &a2, entity *a3);

    entity *get_target_entity() const;

    void set_target_entity(entity *e);

    //0x0057A330
    void blend(vector3d arg0, vector3d eax0, Float arg18);

    //virtual
    //0x0057CC90
    void frame_advance(Float t);

    //virtual
    //0x0057F0D0
    void _sync(camera &a2);
};


extern void game_camera_patch();
