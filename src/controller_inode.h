#pragma once

#include "game_button.h"
#include "info_node.h"
#include "vector2d.h"

class game_button;

namespace ai {

struct controller_inode : info_node {
    enum eControllerButton {};

    enum eControllerAxis {};

    unsigned int triggered_buttons;
    unsigned int held_buttons;


    controller_inode();
    explicit controller_inode(from_mash_in_place_constructor *constructor);
    static void *native_vtable();
    void _frame_advance(Float time_step);


    void _activate(ai_core *core);
    void _deactivate();
    vector3d facing();
    void button_helper(eControllerButton button, unsigned int *held, unsigned int *triggered);
    void update_trigger_buttons();
    unsigned int get_combat_trigger(vector3d direction);

    //0x00445CB0
    //virtual
    bool is_axis_neutral(controller_inode::eControllerAxis a2);

    [[nodiscard]] /* virtual */ vector3d get_axis(controller_inode::eControllerAxis a3) /* = 0 */;

    [[nodiscard]] /* virtual */ game_button get_button(controller_inode::eControllerButton a3) /* = 0 */;

    [[nodiscard]] /* virtual */ vector2d get_axis_2d(controller_inode::eControllerAxis a3) /* = 0 */;

    static const inline string_hash default_id{static_cast<int>(to_hash("controller_inode"))};
};
}  // namespace ai
