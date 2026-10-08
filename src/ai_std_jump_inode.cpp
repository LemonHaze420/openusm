#include "ai_std_jump_inode.h"

#include "actor.h"
#include "physical_interface.h"
#include "common.h"
#include "mash_info_struct.h"
#include "native_info_node_table.h"
#include "utility.h"

namespace ai {

VALIDATE_SIZE(ai_std_jump_inode, 0x50);
VALIDATE_OFFSET(ai_std_jump_inode, field_48, 0x48);

namespace {
void __fastcall jump_destruct(ai_std_jump_inode *self, void *)
{
    self->_destruct_mashed_class();
}
void __fastcall jump_unmash(ai_std_jump_inode *self, void *, mash_info_struct *info, void *context)
{
    self->_unmash(info, context);
}
void __fastcall jump_activate(ai_std_jump_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall jump_advance(ai_std_jump_inode *self, void *, Float elapsed)
{
    self->_frame_advance(elapsed);
}
void __fastcall jump_velocity(ai_std_jump_inode *self, void *, const vector3d &direction, float speed)
{
    self->set_jump_velocity(direction, speed);
}
void __fastcall jump_target(ai_std_jump_inode *self, void *, const vector3d &start, const vector3d &target, float max_y)
{
    self->set_jump_target(start, target, max_y);
}
void __fastcall jump_parameters(ai_std_jump_inode *self, void *, bool attack, float damage, float radius,
                                const string_hash &category)
{
    self->set_jump_parameters(attack, damage, radius, category);
}
void __fastcall jump_base_destruct(ai_std_jump_inode *self, void *)
{
    self->info_node::_destruct_mashed_class();
}
}

void *ai_std_jump_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<ai_std_jump_inode, 386, 537, 16> result;
        result[0] = reinterpret_cast<void *>(&jump_destruct);
        result[1] = reinterpret_cast<void *>(&jump_unmash);
        result[6] = reinterpret_cast<void *>(&native_inode::always_advance);
        result[7] = reinterpret_cast<void *>(&jump_advance);
        result[8] = reinterpret_cast<void *>(&jump_activate);
        result[12] = reinterpret_cast<void *>(&jump_velocity);
        result[13] = reinterpret_cast<void *>(&jump_target);
        result[14] = reinterpret_cast<void *>(&jump_parameters);
        result[15] = reinterpret_cast<void *>(&jump_base_destruct);
        return result;
    }();
    return table.data();
}


ai_std_jump_inode::ai_std_jump_inode(from_mash_in_place_constructor *tag)
    : info_node(tag), field_20(tag), field_30(tag), field_48(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[386]);
}


void ai_std_jump_inode::_destruct_mashed_class()
{
    field_48.destruct_mashed_class();
    info_node::_destruct_mashed_class();
}


void ai_std_jump_inode::_unmash(mash_info_struct *info, void *context)
{
    info_node::_unmash(info, context);
    info->unmash_class_in_place(field_48, this);
}


void ai_std_jump_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    const float invalid = bit_cast<float>(0xFFFFFFFFu);
    field_20 = {invalid, invalid, invalid};
    field_30 = field_20;
    field_3C = false;
    field_3D = false;
    field_1C = 0.0f;
    field_40 = 1.0f;
    field_44 = 10.0f;
    field_48 = string_hash{0};
    field_4C = 5.0f;
}


void ai_std_jump_inode::_frame_advance(Float elapsed)
{
    field_1C += elapsed;
}


void ai_std_jump_inode::set_jump_velocity(const vector3d &direction, float speed)
{
    const double squared =
        double(direction.x) * direction.x + double(direction.y) * direction.y + double(direction.z) * direction.z;
    if (squared < 0.99f || squared > 1.01f) {
        field_20 = direction;
        const double scale = speed / (double(field_20.x) * field_20.x + double(field_20.y) * field_20.y +
                                      double(field_20.z) * field_20.z);
        field_20 = {float(scale * field_20.x), float(scale * field_20.y), float(scale * field_20.z)};
    } else {
        field_20 = {speed * direction.x, speed * direction.y, speed * direction.z};
    }
}


void ai_std_jump_inode::set_jump_target(const vector3d &start, const vector3d &target, float max_y)
{
    field_20 = physical_interface::calculate_perfect_force_vector(
        start, target, max_y, field_C->physical_ifc()->m_gravity_multiplier);
}


void ai_std_jump_inode::set_jump_parameters(bool attack, float damage, float radius, string_hash category)
{
    field_3C = attack;
    field_40 = damage;
    field_44 = radius;
    field_48 = category;
}

}
