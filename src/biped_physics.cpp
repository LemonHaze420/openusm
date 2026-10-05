#include "biped_physics.h"

#include "utility.h"
#include "bone_mass_info.h"
#include "conglom.h"
#include "oldmath_po.h"

void biped_physics::capture_frame(conglomerate *owner, int frame)
{
    auto &matrices = frame == 1 ? frame_1_mats : frame_0_mats;
    (frame == 1 ? is_frame_1 : is_frame_0) = true;
    static const int bone_ids[10] = {2, 0, 12, 13, 5, 6, 15, 16, 8, 9};
    for (int i = 0; i < 10; ++i) {
        const string_hash name{biped_bone_name(bone_ids[i])};
        matrices[i] = owner->get_bone(name, true)->get_abs_po().m;
    }
}

void biped_physics::set_capture_frame_delta(Float a1)
{
    is_frame_delta = true;
    curr_delta_t = a1;
}
