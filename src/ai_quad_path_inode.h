#pragma once

#include "info_node.h"

struct ai_path;

namespace ai {
struct quad_path_inode : info_node {
    ai_path *path;

    quad_path_inode();
    explicit quad_path_inode(from_mash_in_place_constructor *tag);
    void _destruct_mashed_class();
    static void *native_vtable();
};
}
