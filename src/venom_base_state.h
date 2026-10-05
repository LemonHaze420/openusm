#pragma once

#include "hero_base_state.h"

namespace ai {


struct venom_base_state : hero_base_state {
    venom_base_state();
    explicit venom_base_state(int mode);

    static void *native_vtable();
    void _get_info_node_list(info_node_desc_list &list);
    string_hash get_desired_state_id(Float dt) const;
};

}  // namespace ai
