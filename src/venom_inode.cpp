#include "venom_inode.h"

#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"

VALIDATE_SIZE(venom_inode, 0xBC);

venom_inode::venom_inode(from_mash_in_place_constructor *a2) : ai::info_node(a2)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[440]);
#else
    m_vtbl = 0x0087DE38;
#endif


    field_1C[(0x98 - 0x1C) / 4] = 0;
    field_1C[(0x9C - 0x1C) / 4] = 0;
    field_1C[(0xA0 - 0x1C) / 4] = 0;
    field_1C[(0xA4 - 0x1C) / 4] = 0;
}
