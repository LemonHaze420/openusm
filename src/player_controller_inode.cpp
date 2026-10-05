#include "player_controller_inode.h"

#include "common.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "base_ai_core.h"
#include "combat_inode.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "vtbl.h"

#include <cmath>
#include <algorithm>
#include <array>
#include <new>

namespace ai {

VALIDATE_SIZE(player_controller_inode, 0xEC);

namespace {
player_controller_inode *__fastcall player_delete(player_controller_inode *self, void *, unsigned int flags)
{
    self->~player_controller_inode();
    if (flags & 1u) ::operator delete(self);
    return self;
}
int __fastcall player_type(const player_controller_inode *) { return 358; }
bool __fastcall player_subclass(const player_controller_inode *, void *, mash::virtual_types_enum type)
{
    return type == 561 || type == 537 || type == 573;
}
int __fastcall player_size(const player_controller_inode *) { return sizeof(player_controller_inode); }
void __fastcall player_frame(player_controller_inode *self, void *, Float dt) { self->_frame_advance(dt); }
vector3d *__fastcall player_facing(player_controller_inode *self, void *, vector3d *out)
{
    *out = self->_facing(); return out;
}
void __fastcall player_buttons(player_controller_inode *self, void *) { self->update_trigger_buttons(); }
void __fastcall player_sticks(player_controller_inode *self, void *) { self->update_stick_cache(); }
bool __fastcall player_pending(player_controller_inode *self, void *) { return self->has_pending_trigger(); }
void __fastcall player_set_trigger(player_controller_inode *self, void *, unsigned int trigger) { self->set_combat_trigger(trigger); }
vector3d *__fastcall player_axis(player_controller_inode *self, void *, vector3d *out, controller_inode::eControllerAxis axis)
{
    *out = self->_get_axis(axis); return out;
}
vector2d *__fastcall player_axis_2d(player_controller_inode *self, void *, vector2d *out, controller_inode::eControllerAxis axis)
{
    *out = self->_get_axis_2d(axis); return out;
}
game_button *__fastcall player_button(player_controller_inode *self, void *, game_button *out, controller_inode::eControllerButton button)
{
    ::new (static_cast<void *>(out)) game_button(self->_get_button(button)); return out;
}
}

void *player_controller_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 23> result;
        std::copy_n(static_cast<void **>(controller_inode::native_vtable()), result.size(), result.data());
        result[2] = bit_cast<void *>(&player_delete);
        result[3] = bit_cast<void *>(&player_type);
        result[4] = bit_cast<void *>(&player_subclass);
        result[7] = bit_cast<void *>(&player_frame);
        result[11] = bit_cast<void *>(&player_size);
        result[12] = bit_cast<void *>(&player_facing);
        result[14] = bit_cast<void *>(&player_buttons);
        result[15] = bit_cast<void *>(&player_sticks);
        result[16] = bit_cast<void *>(&player_pending);
        result[18] = bit_cast<void *>(&player_set_trigger);
        result[20] = bit_cast<void *>(&player_axis);
        result[21] = bit_cast<void *>(&player_axis_2d);
        result[22] = bit_cast<void *>(&player_button);
        return result;
    }();
    return table.data();
}

player_controller_inode::player_controller_inode()
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[358]);
#else
    m_vtbl = 0x0087D370;
#endif
}

player_controller_inode::player_controller_inode(from_mash_in_place_constructor *a2)
    : controller_inode(a2)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[358]);
#else
    m_vtbl = 0x0087D370;
#endif
}

float player_controller_inode::get_motion_force()
{
    auto *the_actor = this->get_actor();
    auto *player_controller = the_actor->get_player_controller();
    return player_controller->get_motion_force();
}


game_button player_controller_inode::_get_button(controller_inode::eControllerButton button)
{
    const auto *controller = get_actor()->get_player_controller();
    switch (button) {
    case 0:
    case 7: return controller->gb_jump;
    case 1:
    case 8: return controller->gb_grab;
    case 2:
    case 9: return controller->gb_attack;
    case 3:
    case 10: return controller->gb_attack_secondary;
    case 4:
    case 15: return controller->gb_range;
    case 5:
    case 11:
    case 12: return controller->field_1B8;
    case 13: return controller->gb_swing_raw;
    case 16: return controller->field_254;
    case 17: return controller->field_288;

    default: return game_button{};
    }
}

vector3d player_controller_inode::_get_axis(eControllerAxis axis)
{
    const auto *controller = get_actor()->get_player_controller();
    switch (axis) {
    case 0: return controller->field_3E0;
    case 2: return controller->field_3EC;
    case 3:
    case 4: {
        const auto &trigger = controller->field_2BC[static_cast<int>(axis) - 1];
        return YVEC * (trigger.field_2D ? 0.0f : trigger.field_10);
    }
    default: return ZEROVEC;
    }
}

vector2d player_controller_inode::_get_axis_2d(eControllerAxis)
{
    const auto *controller = get_actor()->get_player_controller();
    return {controller->field_2BC[1].field_10, controller->field_2BC[0].field_10};
}

void player_controller_inode::update_stick_cache()
{
    for (int sample = 9; sample != 0; --sample) {
        stick_cache[sample] = stick_cache[sample - 1];
        for (int component = 0; component != 3; ++component)
            facing_cache[sample][component] = facing_cache[sample - 1][component];
    }
    const auto &forward = get_actor()->get_abs_po().get_z_facing();
    facing_cache[0][0] = forward.x;
    facing_cache[0][1] = forward.y;
    facing_cache[0][2] = forward.z;
    auto stick = get_axis_2d(static_cast<eControllerAxis>(2));
    const float magnitude2 = stick.x * stick.x + stick.y * stick.y;
    if (magnitude2 <= 0.64000004529953f) {
        stick_cache[0] = {0.0f, 0.0f};
    } else {
        if (magnitude2 > 9.9999994e-11f) {
            const float inverse_length = 1.0f / std::sqrt(magnitude2);
            stick.x *= inverse_length;
            stick.y *= inverse_length;
        }
        stick_cache[0] = stick;
        for (int sample = 1; sample != 10; ++sample) {
            if (stick.x * stick_cache[sample].x + stick.y * stick_cache[sample].y < 0.5f)
                stick_cache[sample] = {0.0f, 0.0f};
        }
    }
}

vector3d player_controller_inode::_facing()
{

    constexpr float weights[10] = {0.05f, 0.05f, 0.05f, 0.05f, 0.05f,
                                   0.35f, 0.35f, 0.05f, 0.05f, 0.05f};
    vector3d result = ZEROVEC;
    for (int sample = 0; sample != 10; ++sample) {
        result.x += facing_cache[sample][0] * weights[sample];
        result.y += facing_cache[sample][1] * weights[sample];
        result.z += facing_cache[sample][2] * weights[sample];
    }
    if (result.length2() <= LARGE_EPSILON)
        result = vector3d{facing_cache[3][0], facing_cache[3][1], facing_cache[3][2]};
    else
        result.normalize();
    result.normalize();
    return result;
}

void player_controller_inode::_frame_advance(Float)
{
    using callback = void (__fastcall *)(player_controller_inode *, void *);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x38))(this, nullptr);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x3C))(this, nullptr);
}

bool player_controller_inode::has_pending_trigger()
{
    unsigned int trigger = triggered_buttons | (held_buttons << 16);
    if (get_axis(static_cast<eControllerAxis>(3)).y < -0.0001f)
        trigger |= 0x80000;
    if (get_axis(static_cast<eControllerAxis>(4)).y < -0.0001f)
        trigger |= 0x100000;
    unsigned int jump_triggered = 0, jump_held_or_range_triggered = 0, range_held = 0;
    button_helper(static_cast<eControllerButton>(0), &jump_held_or_range_triggered, &jump_triggered);
    button_helper(static_cast<eControllerButton>(4), &range_held, &jump_held_or_range_triggered);
    if (range_held && jump_triggered)
        trigger |= 0x10000;
    auto *combat = static_cast<combat_inode *>(field_8->get_info_node(combat_inode::default_id, true));
    using int_query = int (__fastcall *)(combat_inode *, void *);
    using bool_query = bool (__fastcall *)(combat_inode *, void *);
    if (jump_triggered && reinterpret_cast<int_query>(get_vfunc(combat->m_vtbl, 0x58))(combat, nullptr))
        trigger |= 0x20000;
    if (reinterpret_cast<bool_query>(get_vfunc(combat->m_vtbl, 0x88))(combat, nullptr) &&
        reinterpret_cast<bool_query>(get_vfunc(combat->m_vtbl, 0x84))(combat, nullptr) &&
        (triggered_buttons & 0xC) != 0)
        trigger |= 0x40000;
    return trigger != 0;
}

void player_controller_inode::set_combat_trigger(unsigned int)
{

}

} // namespace ai


game_button *__fastcall player_controller_inode__get_button(ai::player_controller_inode *self, void *, game_button *out,
        ai::controller_inode::eControllerButton a3)
{
    ::new (static_cast<void *>(out)) game_button(self->_get_button(a3));
    return out;
}


void player_controller_inode_patch()
{
    set_vfunc(0x0087D3C8, player_controller_inode__get_button);
}
