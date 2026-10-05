#pragma once

#include "camera.h"

struct game_camera : camera {
    static inline constexpr auto CAMERA_SHAKES_TOTAL = 4u;

    struct _camera_shake_t {
        float field_0;
        float field_4;
        float field_8;
        float field_C;
        float empty[2];
        short field_18;

        char field_1A;

        _camera_shake_t();

        void clear();
    };

    float field_D0[16];
    int empty[3];

    vhandle_type<entity> field_118;
    vector3d field_11C;
    float field_128;
    bool field_12C;
    char pad[3];

    _camera_shake_t field_130[CAMERA_SHAKES_TOTAL];

    game_camera() = default;

    game_camera(const string_hash &a2, entity *a3);
    static void *native_vtable();
    void clear_shakes();
    vector3d frame_advance_shake(vector3d position, Float dt);
    short add_shake(float amplitude, float frequency, float duration, float fade_in, float fade_out);
    void remove_shake(short handle);
    bool is_shake_active(short handle) const;

    entity *get_target_entity() const;

    //0x0057CC50
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
