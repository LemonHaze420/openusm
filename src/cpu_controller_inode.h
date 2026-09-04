#pragma once

#include "controller_inode.h"
#include "info_node.h"

struct from_mash_in_place_constructor;

namespace ai {

struct cpu_controller_inode : controller_inode {
    int field_1C[59];

    //0x00481450
    cpu_controller_inode(from_mash_in_place_constructor *a2);

    //0x00467FA0
    //virtual
    [[nodiscard]] game_button get_button(controller_inode::eControllerButton);
};

}  // namespace ai
