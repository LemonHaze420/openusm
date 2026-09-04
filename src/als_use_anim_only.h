#pragma once

#include "als_motion_compensator.h"
#include "float.hpp"

namespace als {
struct use_anim_only : motion_compensator {
    use_anim_only()
    {
        this->m_vtbl = 0x00878410;
    }

    void post_anim_action(Float arg0);
};

}  // namespace als
