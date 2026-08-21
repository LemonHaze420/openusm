#pragma once

#include "nal_anim_controller.h"

struct ped_anim_controller : nal_anim_controller {
    int field_54;
    ped_anim_controller *field_58;
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod field_5C;
    ped_anim_controller *field_60;

    ped_anim_controller(actor *a2, nalBaseSkeleton *a3, uint32_t a4, als::als_meta_anim_table_shared *a5);
};
