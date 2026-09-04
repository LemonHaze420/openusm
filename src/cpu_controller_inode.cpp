#include "cpu_controller_inode.h"

#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "game_button.h"
#include "input_mgr.h"

namespace ai {
VALIDATE_SIZE(cpu_controller_inode, 0x108);

cpu_controller_inode::cpu_controller_inode(from_mash_in_place_constructor *a2)
{
    THISCALL(0x00481450, this, a2);
}

game_button cpu_controller_inode::get_button(ai::controller_inode::eControllerButton)
{
    game_button result{};

    return result;
}
}  // namespace ai
