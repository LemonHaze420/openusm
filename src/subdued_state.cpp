#include "subdued_state.h"

#include "actor.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "colgeom_alter_sys.h"
#include "common.h"
#include "damage_inode.h"
#include "damage_interface.h"
#include "event.h"
#include "event_manager.h"
#include "info_node_desc_list.h"
#include "mashed_state.h"
#include "param_list.h"
#include "pendulum.h"
#include "physical_interface.h"
#include "std_default_trans_inode.h"
#include "std_fear_inode.h"
#include "vtbl.h"
#include "web_interface.h"

#include <algorithm>
#include <array>

namespace ai {
VALIDATE_SIZE(subdued_state, 0x44);
namespace {
unsigned __fastcall subdued_type(const subdued_state *)
{
    return 372;
}
int __fastcall subdued_size(const subdued_state *)
{
    return sizeof(subdued_state);
}
bool __fastcall subdued_subclass(const subdued_state *, void *, unsigned type)
{
    return type == 331 || type == 535 || type == 567 || type == 573;
}
void *__fastcall delete_subdued(subdued_state *self, void *, unsigned flags)
{
    self->~subdued_state();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
void __fastcall activate_subdued(subdued_state *self, void *, ai_state_machine *machine, const mashed_state *state,
                                 const mashed_state *previous, const param_block *params,
                                 base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, params, flags);
}
void __fastcall deactivate_subdued(subdued_state *self, void *, const mashed_state *next)
{
    self->_deactivate(next);
}
state_trans_messages __fastcall frame_subdued(subdued_state *self, void *, Float delta)
{
    return self->_frame_advance(delta);
}
state_trans_action *__fastcall transition_subdued(subdued_state *self, void *, state_trans_action *out, Float delta)
{
    *out = self->_check_transition(delta);
    return out;
}
void __fastcall subdued_info(subdued_state *, void *, info_node_desc_list &list)
{
    list.add_entry({std_default_trans_inode::default_id, 371});
    list.add_entry({damage_inode::default_id, 343});
    list.add_entry({als_inode::default_id, 333});
}
void update_webbing(actor *owner)
{
    if (owner->field_88 && owner->field_88->field_18 != 1 && owner->has_damage_ifc() &&
        owner->damage_ifc()->field_21C.field_0[0] > 0.75f)
        owner->field_88->set_webbed(true);
}
}

void *subdued_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(std_interrupt_state::native_vtable()), result.size(), result.data());
        result[2] = bit_cast<void *>(&delete_subdued);
        result[3] = bit_cast<void *>(&subdued_type);
        result[4] = bit_cast<void *>(&subdued_subclass);
        result[6] = bit_cast<void *>(&activate_subdued);
        result[7] = bit_cast<void *>(&deactivate_subdued);
        result[8] = bit_cast<void *>(&frame_subdued);
        result[9] = bit_cast<void *>(&subdued_info);
        result[11] = bit_cast<void *>(&transition_subdued);
        result[13] = bit_cast<void *>(&subdued_size);
        return result;
    }();
    return table.data();
}
subdued_state::subdued_state()
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
subdued_state::subdued_state(from_mash_in_place_constructor *tag) : std_interrupt_state(tag)
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
void subdued_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                              const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    auto *core = get_core();
    core->stop_movement();
    if (core->field_70)
        core->field_70->set_avoid_floor(true);
    category_requested = false;
    hold_time = 5.0f;
    field_38 = 2.0f;
    field_3C = 0;
    if (auto *fear = static_cast<std_fear_inode *>(core->get_info_node(std_fear_inode::default_id, false)))
        fear->post_event_to_others(5, 1.0f, true, false, false);
    auto *owner = get_actor();
    event_manager::raise_event(event::SUBDUED, owner->get_my_handle());
    webbing = owner->damage_ifc()->field_21C.field_0[0];
    update_webbing(owner);
}
void subdued_state::_deactivate(const mashed_state *next)
{
    auto *owner = get_actor();
    if (owner->has_physical_ifc()) {
        auto *physics = owner->physical_ifc();
        if (auto *constraint = physics->get_pendulum(4)) {
            combat_pendulum_manager::release_pendulum(constraint);
            physics->set_pendulum(4, nullptr);
        }
    }
    base_state::_deactivate(next);
}
state_trans_messages subdued_state::_frame_advance(Float delta)
{
    auto *core = get_core();
    auto *animation = static_cast<als_inode *>(core->get_info_node(als_inode::default_id, true));
    auto *owner = get_actor();
    if (owner->has_physical_ifc()) {
        if (auto *constraint = owner->physical_ifc()->get_pendulum(4)) {
            constraint->m_active = true;
            constraint->m_constraint = (1.0f - 2.0f * delta) * constraint->m_constraint + delta * 8.0f;
            constraint->set_attach_limb(9);
            als::param_list params;
            params.add_param({0x36, 4.0f});
            animation->set_desired_params(params, static_cast<als::layer_types>(0));
            params.clear();
        }
    }
    const auto message = enhanced_state::frame_advance(delta);
    if (!category_requested && animation->is_layer_interruptable(static_cast<als::layer_types>(0))) {
        auto *arrest = core->get_info_node(string_hash{int(to_hash("AI_ARRESTED_INODE"))}, false);
        const bool arrested =
            arrest && reinterpret_cast<entity_base_vhandle *>(reinterpret_cast<unsigned char *>(arrest) + 0x1C)
                          ->get_volatile_ptr();
        animation->request_category_transition(string_hash{int(to_hash(arrested ? "Arrested_Idle" : "Subdued"))},
                                               static_cast<als::layer_types>(0),
                                               true,
                                               false,
                                               false);
        category_requested = true;
    }
    if (!category_requested || field_1C <= hold_time)
        update_webbing(owner);
    if (auto *damage = owner->damage_ifc())
        damage->field_21C.field_0[0] = std::clamp(webbing, damage->field_21C.field_0[1], damage->field_21C.field_0[2]);
    return message;
}
state_trans_action subdued_state::_check_transition(Float delta)
{
    get_core()->get_info_node(std_default_trans_inode::default_id, true);
    const auto target = my_mashed_state->field_0.does_parameter_exist(to_state_always_hash)
                            ? my_mashed_state->field_0.get_pb_hash(to_state_always_hash)
                            : string_hash{0};
    if (get_actor()->damage_ifc()->field_1FC.field_0[0] > 0.0f && target != string_hash{0})
        return {GOTO_STATE, target, TRANS_TOTAL_MSGS, nullptr};
    using callback = state_trans_action *(__fastcall *)(subdued_state *, void *, state_trans_action *, Float);
    state_trans_action out;
    reinterpret_cast<callback>(static_cast<void **>(std_interrupt_state::native_vtable())[11])(
        this, nullptr, &out, delta);
    return out;
}
}  // namespace ai
