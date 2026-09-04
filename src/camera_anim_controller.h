#pragma once

#include "nal_anim_controller.h"

struct camera_anim_controller : nal_anim_controller {
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod field_54;
    camera_anim_controller *field_58;
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod field_5C;
    camera_anim_controller *field_60;

    camera_anim_controller(actor *a2, nalBaseSkeleton *a3, unsigned int a4, als::als_meta_anim_table_shared *a5);
};
