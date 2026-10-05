#pragma once

#include "enhanced_state.h"

namespace ai {


struct traffic_base_state : enhanced_state {

    traffic_base_state();
    explicit traffic_base_state(from_mash_in_place_constructor *constructor);
    ~traffic_base_state() = default;

    static void *native_vtable();


    void get_info_node_list(info_node_desc_list &nodes);
};

}
