#pragma once

#include "debug_renderable.h"

struct conglomerate;

namespace ai {

struct als_inode;

struct als_inode_render_debug : debug_renderable {
    char *field_4;
    int field_8[67];

    //0x00693720
    als_inode_render_debug(als_inode *a2, conglomerate *a3);
};
}  // namespace ai
