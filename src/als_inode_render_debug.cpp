#include "als_inode_render_debug.h"

#include "als_inode.h"
#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(als_inode_render_debug, 0x114u);

als_inode_render_debug::als_inode_render_debug(als_inode *a2, conglomerate *a3)
{
    THISCALL(0x00693720, this, a2, a3);
}
}  // namespace ai
