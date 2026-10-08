#include "fall_death_inode.h"

#include "actor.h"
#include "common.h"
#include "damage_interface.h"
#include "glass_house_manager.h"
#include "native_info_node_table.h"
#include "oldmath_po.h"

#include <cmath>
#include <cstdlib>

namespace ai {

VALIDATE_SIZE(fall_death_inode, 0x38);
VALIDATE_OFFSET(fall_death_inode, check_index, 0x34);

namespace {
void __fastcall fall_activate(fall_death_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall fall_advance(fall_death_inode *self, void *, Float elapsed)
{
    self->_frame_advance(elapsed);
}
}

void *fall_death_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<fall_death_inode, 345> result;
        result[6] = reinterpret_cast<void *>(&native_inode::always_advance);
        result[7] = reinterpret_cast<void *>(&fall_advance);
        result[8] = reinterpret_cast<void *>(&fall_activate);
        return result;
    }();
    return table.data();
}


fall_death_inode::fall_death_inode()
    : allow_fall_death(false), allow_house_death(false), allow_water_death(false), allow_obb_death(false),
      death_requested(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[345]);
}


fall_death_inode::fall_death_inode(from_mash_in_place_constructor *tag) : info_node(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[345]);
}


void fall_death_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    allow_fall_death =
        my_param_block.get_optional_pb_int(string_hash{static_cast<int>(to_hash("allow_fall_death"))}, 1, nullptr) != 0;
    allow_house_death = my_param_block.get_optional_pb_int(
                            string_hash{static_cast<int>(to_hash("allow_house_death"))}, 1, nullptr) != 0;
    allow_water_death = my_param_block.get_optional_pb_int(
                            string_hash{static_cast<int>(to_hash("allow_water_death"))}, 0, nullptr) != 0;
    allow_obb_death =
        my_param_block.get_optional_pb_int(string_hash{static_cast<int>(to_hash("allow_obb_death"))}, 0, nullptr) != 0;
    terminal_velocity = -std::fabs(
        my_param_block.get_optional_pb_float(string_hash{static_cast<int>(to_hash("terminal_vel"))}, 10.0f, nullptr));
    terminal_time =
        my_param_block.get_optional_pb_float(string_hash{static_cast<int>(to_hash("terminal_time"))}, 0.5f, nullptr);
    previous_height = -1000.0f;
    falling_time = 0.0f;
    check_index = std::rand() % 5;
    death_requested = false;
}


void fall_death_inode::_frame_advance(Float elapsed)
{
    vector3d position = field_C->get_abs_position();
    const float height = position.y;
    if (!field_C->has_damage_ifc())
        return;
    auto *damage = field_C->damage_ifc();
    check_index = (check_index + 1) % 5;
    if (death_requested && damage != nullptr) {
        const string_hash none{0};
        damage->apply_damage(nullptr, 32000.0f, 1, ZEROVEC, YVEC, 0, none, none, none, false, ZEROVEC, 17, false);
        auto &meter = damage->field_21C.field_0;
        meter[0] = 0.5f;
        if (!(meter[2] >= 0.5f))
            meter[0] = meter[2];
        if (meter[0] < meter[1])
            meter[0] = meter[1];
        if (height < -1000.0001220703125f && check_index == 0) {
            position.y = -900.0f;
            entity_set_abs_position(field_C, position);
        }
    }
    if (allow_fall_death) {
        if ((height - previous_height) / elapsed >= terminal_velocity) {
            falling_time = 0.0f;
        } else {
            falling_time += elapsed;
            if (falling_time > terminal_time)
                death_requested = true;
        }
    }
    previous_height = height;
    if (allow_house_death && check_index == 0 && !glass_house_manager::is_point_in_glass_house(position))
        death_requested = true;
}

}
