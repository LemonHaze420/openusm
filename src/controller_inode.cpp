#include "controller_inode.h"

#include "actor.h"
#include "base_ai_core.h"
#include "combat_inode.h"
#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"
#include "physics_inode.h"
#include "oldmath_po.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <new>

extern "C" int __cdecl _purecall();


namespace ai {

VALIDATE_SIZE(controller_inode, 0x24);

namespace {
controller_inode *__fastcall controller_delete(controller_inode *self, void *, unsigned int flags)
{
    self->~controller_inode();
    if (flags & 1u) ::operator delete(self);
    return self;
}
int __fastcall controller_type(const controller_inode *) { return 561; }
bool __fastcall controller_subclass(const controller_inode *, void *, mash::virtual_types_enum type)
{
    return type == 537 || type == 573;
}
int __fastcall controller_size(const controller_inode *) { return 0xEC; }
void __fastcall controller_frame(controller_inode *self, void *, Float dt) { self->_frame_advance(dt); }
void __fastcall controller_activate(controller_inode *self, void *, ai_core *core) { self->_activate(core); }
void __fastcall controller_deactivate(controller_inode *self, void *) { self->_deactivate(); }
vector3d *__fastcall controller_facing(controller_inode *self, void *, vector3d *out)
{
    *out = self->controller_inode::facing();
    return out;
}
void __fastcall controller_button_helper(controller_inode *self, void *, controller_inode::eControllerButton button,
                                         unsigned int *held, unsigned int *triggered)
{
    self->button_helper(button, held, triggered);
}
void __fastcall controller_noop(controller_inode *, void *) {}
int __fastcall controller_purecall(controller_inode *, void *) { return _purecall(); }
unsigned int __fastcall controller_trigger(controller_inode *self, void *, vector3d direction)
{
    return self->controller_inode::get_combat_trigger(direction);
}
bool __fastcall controller_neutral(controller_inode *self, void *, controller_inode::eControllerAxis axis)
{
    return self->is_axis_neutral(axis);
}
}

void *controller_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 23> result{};
        std::copy_n(static_cast<void **>(info_node::native_vtable()), 12, result.data());
        result[2] = bit_cast<void *>(&controller_delete);
        result[3] = bit_cast<void *>(&controller_type);
        result[4] = bit_cast<void *>(&controller_subclass);
        result[7] = bit_cast<void *>(&controller_frame);
        result[8] = bit_cast<void *>(&controller_activate);
        result[9] = bit_cast<void *>(&controller_deactivate);

        result[11] = bit_cast<void *>(&controller_size);
        result[12] = bit_cast<void *>(&controller_facing);
        result[13] = bit_cast<void *>(&controller_button_helper);
        result[14] = result[15] = bit_cast<void *>(&controller_noop);
        result[16] = result[18] = result[20] = result[21] = result[22] = bit_cast<void *>(&controller_purecall);
        result[17] = bit_cast<void *>(&controller_trigger);
        result[19] = bit_cast<void *>(&controller_neutral);
        return result;
    }();
    return table.data();
}

void controller_inode::_frame_advance(Float)
{
    using callback = void (__fastcall *)(controller_inode *, void *);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x38))(this, nullptr);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x3C))(this, nullptr);
}

controller_inode::controller_inode()
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[561]);
#else
    m_vtbl = 0x0087D310;
#endif
}

controller_inode::controller_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[561]);
#else
    m_vtbl = 0x0087D310;
#endif
}

void controller_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    held_buttons = 0;
    triggered_buttons = 0;
}

void controller_inode::_deactivate()
{

}

vector3d controller_inode::facing()
{
    return get_actor()->get_abs_po().get_z_facing();
}

void controller_inode::button_helper(eControllerButton button, unsigned int *held, unsigned int *triggered)
{
    const game_button state = get_button(button);
    if (!state.is_flagged(0x20)) {
        if (state.is_flagged(GBFLAG_PRESSED) && state.field_1C >= 0.2f)
            *held |= 1u << static_cast<unsigned int>(button);
        if (state.is_flagged(GBFLAG_TRIGGERED))
            *triggered |= 1u << static_cast<unsigned int>(button);
    }
}

void controller_inode::update_trigger_buttons()
{
    held_buttons = triggered_buttons = 0;
    using callback = void (__fastcall *)(controller_inode *, void *, eControllerButton,
                                        unsigned int *, unsigned int *);
    const auto helper = reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x34));
    for (unsigned int button = 0; button != 6; ++button)
        helper(this, nullptr, static_cast<eControllerButton>(button), &held_buttons, &triggered_buttons);
}

unsigned int controller_inode::get_combat_trigger(vector3d direction)
{
    field_8->get_info_node(physics_inode::default_id, true);
    auto &params = *field_8->get_param_block();
    const bool allow_web_tie = params.get_optional_pb_int(string_hash{"loco_allow_web_tie"}, 1, nullptr) != 0;
    const bool allow_web_splat = params.get_pb_int(string_hash{"loco_allow_web_splat"}) != 0;
    const bool allow_grab = params.get_pb_int(string_hash{"loco_allow_grab"}) != 0;
    const bool allow_punch = params.get_pb_int(string_hash{"loco_allow_punch"}) != 0;
    params.get_pb_int(string_hash{"loco_allow_kick"});
    const bool allow_dodge = params.get_pb_int(string_hash{"loco_allow_dodge"}) != 0;

    direction.y = 0.0f;
    direction.normalize();
    vector3d forward;
    using facing_callback = vector3d *(__fastcall *)(controller_inode *, void *, vector3d *);
    reinterpret_cast<facing_callback>(get_vfunc(m_vtbl, 0x30))(this, nullptr, &forward);
    unsigned int result = 0;
    if (direction.length2() <= EPSILON) {
        result = 0x200000;
    } else {
        const float along = dot(forward, direction);
        if (along > 0.866f) result = 0x60000000;
        else if (along > 0.7071f) result = 0x20000000;
        else if (-along > 0.866f) result = 0x18000000;
        else if (-along > 0.7071f) result = 0x08000000;
        const float across = forward.x * -direction.z + forward.z * direction.x + forward.y * direction.y;
        if (across > 0.866f) result |= 0x06000000;
        else if (across > 0.7071f) result |= 0x02000000;
        else if (-across > 0.866f) result |= 0x01800000;
        else if (-across > 0.7071f) result |= 0x00800000;
    }

    unsigned int jump_triggered = 0, jump_held_or_range_triggered = 0, range_held = 0;
    button_helper(static_cast<eControllerButton>(0), &jump_held_or_range_triggered, &jump_triggered);
    button_helper(static_cast<eControllerButton>(4), &range_held, &jump_held_or_range_triggered);
    if (range_held && jump_triggered)
        result |= 0x10000;
    auto *combat = static_cast<combat_inode *>(field_8->get_info_node(combat_inode::default_id, true));
    using int_query = int (__fastcall *)(combat_inode *, void *);
    using bool_query = bool (__fastcall *)(combat_inode *, void *);
    if (jump_triggered && reinterpret_cast<int_query>(get_vfunc(combat->m_vtbl, 0x58))(combat, nullptr))
        result |= 0x20000;
    if (reinterpret_cast<bool_query>(get_vfunc(combat->m_vtbl, 0x88))(combat, nullptr) &&
        reinterpret_cast<bool_query>(get_vfunc(combat->m_vtbl, 0x84))(combat, nullptr) &&
        (triggered_buttons & 0xC) != 0)
        result |= 0x40000;

    unsigned int triggered = triggered_buttons, held = held_buttons;
    if (!allow_punch) triggered &= ~0xCu;
    if (!allow_web_tie) held &= ~0x10u;
    if (!allow_web_splat) triggered &= ~0x10u;
    if (!allow_grab) triggered &= ~2u;
    if (!allow_dodge) triggered &= ~1u;
    return result | triggered | (held << 16);
}

bool controller_inode::is_axis_neutral(controller_inode::eControllerAxis a2)
{
    return get_axis(a2).length2() < 0.35f;
}

vector3d controller_inode::get_axis(controller_inode::eControllerAxis a3)
{
    void(__fastcall * func)(controller_inode *, void *, vector3d *, controller_inode::eControllerAxis) =
        CAST(func, get_vfunc(m_vtbl, 0x50));

    vector3d result;
    func(this, nullptr, &result, a3);

    return result;
}

game_button controller_inode::get_button(controller_inode::eControllerButton a3)
{
    void(__fastcall * func)(controller_inode *, void *, game_button *, controller_inode::eControllerButton) =
        CAST(func, get_vfunc(m_vtbl, 0x58));

    game_button result;
    func(this, nullptr, &result, a3);

    return result;
}

vector2d controller_inode::get_axis_2d(controller_inode::eControllerAxis axis)
{
    using callback = vector2d *(__fastcall *)(controller_inode *, void *, vector2d *, eControllerAxis);
    vector2d result;
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x54))(this, nullptr, &result, axis);
    return result;
}

}  // namespace ai
