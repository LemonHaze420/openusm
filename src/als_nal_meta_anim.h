#pragma once

#include "fixedstring.h"
#include "nal_anim.h"
#include "nal_skeleton.h"
#include "nal_system.h"
#include "variable.h"

struct actor;

namespace als {
struct animation_logic_system;
struct state_machine;
}

namespace als {
    struct als_meta_anim_base;

    struct als_nal_meta_anim : nalAnimClass<nalAnyPose> {
        als_meta_anim_base *field_40;

        als_nal_meta_anim();

        bool is_delay_create() const;

        void delay_create(actor *a2);

        //0x00493F20
        void create(als_meta_anim_base *a2);

        void * create_anim_inst(
            nalBaseSkeleton *a2,
            animation_logic_system *a3,
            state_machine *a4);

        static inline int &vtbl_ptr = var<int>(0x0095979C);
    };
}
