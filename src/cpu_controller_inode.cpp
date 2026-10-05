#include "cpu_controller_inode.h"

#include "actor.h"
#include "base_ai_core.h"
#include "combat_inode.h"
#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "game_button.h"
#include "input_mgr.h"
#include "vtbl.h"
#include "wds.h"
#include "als_inode.h"
#include "combo_system.h"
#include "core_ai_resource.h"
#include "event.h"
#include "layer_state_machine.h"
#include "oldmath_po.h"

#include <algorithm>
#include <array>
#include <new>

namespace ai {
VALIDATE_SIZE(cpu_controller_inode, 0x108);

namespace {
cpu_controller_inode *__fastcall cpu_delete(cpu_controller_inode *self, void *, unsigned int flags)
{
    self->~cpu_controller_inode();
    if (flags & 1u) ::operator delete(self);
    return self;
}
int __fastcall cpu_type(const cpu_controller_inode *) { return 357; }
bool __fastcall cpu_subclass(const cpu_controller_inode *, void *, mash::virtual_types_enum type)
{
    return type == 561 || type == 537 || type == 573;
}
int __fastcall cpu_size(const cpu_controller_inode *) { return sizeof(cpu_controller_inode); }
void __fastcall cpu_frame(cpu_controller_inode *self, void *, Float dt) { self->_frame_advance(dt); }
void __fastcall cpu_activate(cpu_controller_inode *self, void *, ai_core *core) { self->_activate(core); }
bool __fastcall cpu_pending(cpu_controller_inode *self, void *) { return self->has_pending_trigger(); }
unsigned int __fastcall cpu_trigger(cpu_controller_inode *self, void *, vector3d direction) { return self->get_combat_trigger(direction); }
void __fastcall cpu_set_trigger(cpu_controller_inode *self, void *, unsigned int trigger) { self->set_combat_trigger(trigger); }
vector3d *__fastcall cpu_axis(cpu_controller_inode *self, void *, vector3d *out, controller_inode::eControllerAxis axis)
{
    *out = self->_get_axis(axis); return out;
}
vector2d *__fastcall cpu_axis_2d(cpu_controller_inode *self, void *, vector2d *out, controller_inode::eControllerAxis axis)
{
    *out = self->_get_axis_2d(axis); return out;
}
game_button *__fastcall cpu_button(cpu_controller_inode *self, void *, game_button *out, controller_inode::eControllerButton button)
{
    ::new (static_cast<void *>(out)) game_button(self->cpu_controller_inode::get_button(button)); return out;
}
bool can_follow_move(const combo_system_move *previous, const combo_system_move *next)
{
    const auto &links = next->field_80.field_30;
    if (links.m_size == 0) return true;
    for (int i = 0; i != links.m_size; ++i)
        if (links.m_data[i]->field_4 == previous->field_4.field_4 && links.m_data[i]->field_8 == 0)
            return true;
    return false;
}
}

void *cpu_controller_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 23> result;
        std::copy_n(static_cast<void **>(controller_inode::native_vtable()), result.size(), result.data());
        result[2] = bit_cast<void *>(&cpu_delete);
        result[3] = bit_cast<void *>(&cpu_type);
        result[4] = bit_cast<void *>(&cpu_subclass);
        result[7] = bit_cast<void *>(&cpu_frame);
        result[8] = bit_cast<void *>(&cpu_activate);
        result[11] = bit_cast<void *>(&cpu_size);
        result[16] = bit_cast<void *>(&cpu_pending);
        result[17] = bit_cast<void *>(&cpu_trigger);
        result[18] = bit_cast<void *>(&cpu_set_trigger);
        result[20] = bit_cast<void *>(&cpu_axis);
        result[21] = bit_cast<void *>(&cpu_axis_2d);
        result[22] = bit_cast<void *>(&cpu_button);
        return result;
    }();
    return table.data();
}

vector3d cpu_controller_inode::_get_axis(eControllerAxis axis)
{
    auto direction = get_actor()->get_abs_po().get_z_facing();
    if (axis == 2) {
        auto target = vhandle_type<actor>{combat->field_20};
        if (target.get_volatile_ptr()) {
            auto *actor_target = target.get_volatile_ptr();
            if (actor_target) {
                direction = actor_target->get_abs_position() - get_actor()->get_abs_position();
                direction.normalize();
            }
        }
    }
    return direction;
}

bool cpu_controller_inode::has_more_moves_in_chain()
{
    return combo_chain_index >= 0 &&
        ++next_chain_index < field_8->field_6C->field_10->field_14.at(static_cast<uint16_t>(combo_chain_index))->field_1C.m_size;
}

void cpu_controller_inode::engage_current_move()
{
    auto *system = field_8->field_6C->field_10;
    auto *chain = system->field_14.at(static_cast<uint16_t>(combo_chain_index));
    auto *move = system->field_0.at(static_cast<uint16_t>(chain->field_1C.at(static_cast<uint16_t>(next_chain_index))));
    const unsigned int input = move->field_80.field_4.field_8;
    constexpr unsigned int converted[] = {4, 8, 0x10, 0x20, 2, 1, 0x40000, 0x80000,
        0x100000, 0x200000, 0x20000, 0x10000, 0x100000, 0x80000, 0x40004, 0x40008, 0x20000, 0x10000};
    unsigned int trigger = move->field_80.field_4.field_4;
    for (unsigned int bit = 0; bit != std::size(converted); ++bit)
        if (input & (1u << bit)) trigger |= converted[bit];
    using callback = void (__fastcall *)(cpu_controller_inode *, void *, unsigned int);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x48))(this, nullptr, trigger);
    move->field_4.field_4.to_string();
}

void cpu_controller_inode::stop_chain()
{
    if (combo_chain_index >= 0) {
        const auto *chain = field_8->field_6C->field_10->field_14.at(static_cast<uint16_t>(combo_chain_index));
        if (chain->field_18 == 0) next_chain_index = chain->field_1C.m_size + 1;
        else if (chain->field_18 == 1) next_chain_index = 0;
    }
    pending_trigger = 0;
}

void cpu_controller_inode::_frame_advance(Float time_step)
{
    controller_inode::_frame_advance(time_step);
    using query = bool (__fastcall *)(info_node *, void *);
    const auto has_next = [&] { return reinterpret_cast<query>(get_vfunc(combat->m_vtbl, 0xB4))(combat, nullptr); };
    const auto has_current = [&] { return reinterpret_cast<query>(get_vfunc(combat->m_vtbl, 0xA4))(combat, nullptr); };
    if (combo_chain_index >= 0) {
        auto *system = field_8->field_6C->field_10;
        auto *chain = system->field_14.at(static_cast<uint16_t>(combo_chain_index));
        if (next_chain_index + 1 < chain->field_1C.m_size && !has_next()) {
            auto *previous = system->field_0.at(static_cast<uint16_t>(chain->field_1C.at(static_cast<uint16_t>(next_chain_index))));
            auto *next = system->field_0.at(static_cast<uint16_t>(chain->field_1C.at(static_cast<uint16_t>(next_chain_index + 1))));
            using eligible = bool (__fastcall *)(combat_inode *, void *, combo_system_chain *, combo_system_move *);
            if (reinterpret_cast<eligible>(get_vfunc(combat->m_vtbl, 0xA0))(combat, nullptr, chain, next)) {
                if (can_follow_move(previous, next)) {
                    has_more_moves_in_chain();
                    engage_current_move();
                } else {
                    auto *animation = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
                    Float signal_time{0.0f};
                    string_hash category{};
                    using signal = void (__fastcall *)(als_inode *, void *, Float *, string_hash *);
                    reinterpret_cast<signal>(get_vfunc(animation->m_vtbl, 0x34))(animation, nullptr, &signal_time, &category);
                    const float remaining = static_cast<float>(signal_time) - g_world_ptr->time_manager.get_level_time();
                    if (field_104) {
                        if (remaining > 0.0f) field_104 = false;
                    } else if (get_actor()->event_raised_last_frame(event::ATTACK) ||
                               get_actor()->event_raised_last_frame(event::ANIM_ACTION) ||
                               (previous && animation->get_als_layer(static_cast<als::layer_types>(0))->get_time_to_end_of_anim() < time_step)) {
                        has_more_moves_in_chain();
                        engage_current_move();
                        field_104 = true;
                    }
                }
            } else stop_chain();
        }
    }
    if (has_next() || has_current()) field_104 = false;
    if (hero_combat && combo_chain_index >= 0) {
        if (auto *hero = g_world_ptr->get_hero_ptr(0)) {
            auto *hero_node = hero->get_ai_core()->get_info_node(combat_inode::default_id, true);
            using int_query = int (__fastcall *)(info_node *, void *);
            if (hero_node && (has_next() || has_current() ||
                reinterpret_cast<int_query>(get_vfunc(hero_node->m_vtbl, 0x58))(hero_node, nullptr))) {
                auto *target = vhandle_type<actor>{combat->field_20}.get_volatile_ptr();
                if (target == hero || !target) {
                    auto *chain = field_8->field_6C->field_10->field_14.at(static_cast<uint16_t>(combo_chain_index));
                    const float elapsed = g_world_ptr->time_manager.get_level_time() - level_time;
                    for (int i = 0; i != chain->field_0.m_size; ++i) {
                        const auto *telegraph = chain->field_0.m_data[i];
                        const float begin = bit_cast<float>(telegraph->field_4);
                        const float end = bit_cast<float>(telegraph->field_8);
                        if (begin <= elapsed && end >= elapsed) {
                            using callback = void (__fastcall *)(info_node *, void *);
                            reinterpret_cast<callback>(get_vfunc(hero_node->m_vtbl, 0x5C))(hero_node, nullptr);
                            break;
                        }
                    }
                }
            }
        }
    } else level_time = -1.0f;
}

cpu_controller_inode::cpu_controller_inode()
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[357]);
#else
    m_vtbl = 0x0087D3D0;
#endif
    hero_combat = false;
    field_104 = false;
    pending_trigger = 0;
    next_chain_index = -1;
    combo_chain_index = -1;
}

cpu_controller_inode::cpu_controller_inode(from_mash_in_place_constructor *a2)
    : controller_inode(a2)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[357]);
#else
    m_vtbl = 0x0087D3D0;
#endif
}

void cpu_controller_inode::_activate(ai_core *core)
{
    controller_inode::_activate(core);
    combat = static_cast<combat_inode *>(core->get_info_node(combat_inode::default_id, true));
    hero_combat = false;
    auto *hero = g_world_ptr->get_hero_ptr(0);
    if (hero) {
        auto *hero_node = hero->get_ai_core()->get_info_node(combat_inode::default_id, true);
        using callback = bool (__fastcall *)(info_node *, void *);
        hero_combat = reinterpret_cast<callback>(get_vfunc(hero_node->m_vtbl, 0x54))(hero_node, nullptr);
    }
}

unsigned int cpu_controller_inode::get_combat_trigger(vector3d)
{
    const unsigned int result = pending_trigger;
    pending_trigger = 0;
    return result;
}

void cpu_controller_inode::set_combat_trigger(unsigned int trigger)
{
    pending_trigger = trigger;
}

bool cpu_controller_inode::has_pending_trigger() const
{
    return pending_trigger != 0;
}

vector2d cpu_controller_inode::_get_axis_2d(eControllerAxis)
{
    return {0.0f, 0.0f};
}

game_button cpu_controller_inode::get_button(ai::controller_inode::eControllerButton)
{
    game_button result{};

    return result;
}
}  // namespace ai
