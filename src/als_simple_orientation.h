#pragma once

#include "als_motion_compensator.h"

namespace als {


struct set_orient_mocomp : motion_compensator {
    void post_anim_action(Float elapsed);
    static void *native_vtable();
};

struct simple_orientation : motion_compensator {
    void post_anim_action(Float elapsed);
    void get_directions(animation_logic_system *, state_machine *, vector3d &, vector3d &);
    float get_turn_rate(float fallback) const;
    static void *native_vtable();
};

struct simple_orientation_ped : simple_orientation {
    void other_po_changes(Float elapsed);
    static void *native_vtable();
};
}  // namespace als

extern void als_simple_orientation_patch();
