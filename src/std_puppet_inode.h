#pragma once

#include "info_node.h"

namespace ai {

struct std_puppet_inode : info_node {
    bool field_1C;

    static void *native_vtable();
    std_puppet_inode();
    explicit std_puppet_inode(from_mash_in_place_constructor *tag);
    void unmash(mash_info_struct *info, void *data);
    void activate(ai_core *core);
    void frame_advance(Float dt);
    void set_current_state(string_hash state);
    uint32_t get_virtual_type_enum() const
    {
        return 315;
    }
    int get_mash_sizeof() const
    {
        return 0x20;
    }
};

}  // namespace ai
