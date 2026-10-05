#include "pole_swing_inode.h"

#include "common.h"
#include "func_wrapper.h"
#include "native_info_node_table.h"
#include "base_ai_core.h"
#include "combat_state.h"
#include "controller_inode.h"
#include "physics_inode.h"
#include "oldmath_po.h"

namespace ai {

VALIDATE_SIZE(pole_swing_inode, 0x2C);

void *pole_swing_inode::native_vtable()
{

    static native_inode::table<pole_swing_inode, 304> table;
    return table.data();
}

string_hash &pole_swing_inode::default_id = var<string_hash>(0x009584AC);

pole_swing_inode::pole_swing_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[304]);
    field_1C.field_0 = vhandle_type<entity>{0};
    field_1C.field_4 = 0;
}

pole_swing_inode::pole_swing_inode(from_mash_in_place_constructor *tag) : info_node(tag)
{

    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[304]);
    field_1C.field_0 = vhandle_type<entity>{0};
    field_1C.field_4 = 0;
}

bool pole_swing_inode::is_eligible(string_hash) const
{
    return false;
}

bool pole_swing_inode::can_go_to(string_hash state)
{
    auto *controller = static_cast<controller_inode *>(field_8->get_info_node(controller_inode::default_id, true));
    if (state != combat_state::default_id)
        return controller->get_button(static_cast<controller_inode::eControllerButton>(7)).is_triggered();
    auto *physics = static_cast<physics_inode *>(field_8->get_info_node(physics_inode::default_id, true));
    physics->setup_for_jump(-YVEC);
    const auto &current = field_C->get_abs_po();
    po upright;
    upright.set_po(current.get_y_facing(), YVEC, current.get_position());
    entity_set_abs_po(field_C, upright);
    return true;
}

}  // namespace ai
