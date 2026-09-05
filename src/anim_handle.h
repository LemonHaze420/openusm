#pragma once

#include "cstdint"

struct nal_anim_control;

struct anim_handle_t {
    uint32_t field_0;

    //0x00492440
    nal_anim_control *get_anim();
};

extern void anim_handle_patch();
