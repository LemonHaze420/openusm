#pragma once

#include "als_motion_compensator.h"
#include "string_hash.h"

namespace als {

struct animation_logic_system;

struct begin_biped_physics : motion_compensator {
    bool field_14;
    int field_18;
    float field_1C;

    //virtual
    float activate(animation_logic_system *);

    static inline string_hash velocity_left_c_param_hash{int(to_hash("velocity_left_c"))};

    static inline string_hash velocity_y_left_c_param_hash{int(to_hash("velocity_y_left_c"))};

    static inline string_hash velocity_left_deactivate_c_param_hash{int(to_hash("velocity_left_deactivate_c"))};

    static inline string_hash force_scale_param_hash{int(to_hash("force_scale"))};

    static inline string_hash force_y_param_hash{int(to_hash("force_y"))};

    static inline string_hash rotate_xz_ang_param_hash{int(to_hash("rotate_xz_ang"))};
};

}  // namespace als

extern void als_mocomp_patch();
