#pragma once

#include "info_node_descriptor.h"

#include <vector.hpp>

namespace ai {

struct info_node_desc_list {
    _std::vector<info_node_descriptor> field_0;

    info_node_desc_list();

    //0x006D6E20
    void add_entry(info_node_descriptor a2);
};

}  // namespace ai

extern void info_node_desc_list_patch();
