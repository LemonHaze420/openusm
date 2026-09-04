#pragma once

#include "float.hpp"

namespace als {

struct simple_orientation {
    //virtual
    void post_anim_action(Float a2);
};
}  // namespace als

extern void als_simple_orientation_patch();
