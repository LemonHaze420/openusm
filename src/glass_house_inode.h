#pragma once

#include "info_node.h"

#include "vector3d.h"

namespace ai {

struct glass_house_inode : info_node {
    entity_base *field_1C;
    bool field_20;
    bool field_21;
    vector3d field_24;
    float field_30;

    glass_house_inode();

    //0x00455F00
    void show_glass_house_message();
};

}  // namespace ai
