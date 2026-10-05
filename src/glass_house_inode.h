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

    static void *native_vtable();
    glass_house_inode();
    glass_house_inode(from_mash_in_place_constructor *tag);
    uint32_t get_virtual_type_enum() const { return 383; }
    int get_mash_sizeof() const { return 0x34; }
    void unmash(mash_info_struct *info, void *data);
    void activate(ai_core *core);
    void frame_advance(Float dt);
    void deactivate();

    //0x00455F00
    void show_glass_house_message();

    static inline string_hash default_id{to_hash("glass_house")};
};

}  // namespace ai
