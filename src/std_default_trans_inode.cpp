#include "std_default_trans_inode.h"

#include "actor.h"
#include "event.h"
#include "event_manager.h"
#include "wds.h"
#include "common.h"
#include "native_info_node_table.h"
#include "base_ai_core.h"
#include "combat_inode.h"
#include "damage_interface.h"
#include "info_node_desc_list.h"
#include "physical_interface.h"
#include "vtbl.h"
#include <algorithm>
#include <array>
#include "web_interface.h"

namespace ai {

VALIDATE_SIZE(std_default_trans_inode, 0x34);
VALIDATE_SIZE(std_default_state_set_base, 0x1C);

namespace {
void __fastcall native_activate(std_default_trans_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall native_deactivate(std_default_trans_inode *self, void *)
{
    self->_deactivate();
}
void __fastcall native_advance(std_default_trans_inode *self, void *, Float dt)
{
    self->_frame_advance(dt);
}
}  // namespace

void *std_default_trans_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<std_default_trans_inode, 371> result;
        result[6] = reinterpret_cast<void *>(&native_inode::always_advance);
        result[7] = reinterpret_cast<void *>(&native_advance);
        result[8] = reinterpret_cast<void *>(&native_activate);
        result[9] = reinterpret_cast<void *>(&native_deactivate);
        return result;
    }();
    return table.data();
}

std_default_trans_inode::std_default_trans_inode()
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[371]);
#else
    m_vtbl = 0x0087CC7C;
#endif
    enabled = hit_react_enabled = hit_avoid_enabled = false;
    field_24 = true;
    set_enabled(true);
    my_param_block.set_pb_int(string_hash{to_hash("hit_react_enabled")}, 1, true);
    hit_react_enabled = true;
    my_param_block.set_pb_int(string_hash{to_hash("hit_avoid_enabled")}, 1, true);
    hit_avoid_enabled = true;
    my_param_block.set_pb_int(string_hash{to_hash("unconscious_trigger")}, 0, true);
    unconscious_trigger = 0.0f;
    field_25 = field_26 = true;
}

std_default_trans_inode::std_default_trans_inode(from_mash_in_place_constructor *constructor) : info_node(constructor)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[371]);
#else
    m_vtbl = 0x0087CC7C;
#endif
    enabled = hit_react_enabled = hit_avoid_enabled = false;
}

namespace {
void throw_start(event *, entity_base_vhandle, void *context)
{
    auto *node = static_cast<std_default_trans_inode *>(context);
    if (node->field_30)
        static_cast<unsigned char *>(node->field_30)[0xD8] = 1;
}

void throw_end(event *, entity_base_vhandle, void *context)
{
    auto *node = static_cast<std_default_trans_inode *>(context);
    if (node->field_30)
        static_cast<unsigned char *>(node->field_30)[0xD8] = 0;
}
}  // namespace

void std_default_trans_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    field_30 = nullptr;
    const auto actor_handle = get_actor()->get_my_vhandle();
    field_28 = event_manager::add_callback(event::THROW_START, actor_handle, throw_start, this, false);
    field_2C = event_manager::add_callback(event::THROW_END, actor_handle, throw_end, this, false);
    refresh_parameters();
}

void std_default_trans_inode::_deactivate()
{
    const auto actor_handle = get_actor()->get_my_vhandle();
    event_manager::remove_callback(field_28, event::THROW_START, actor_handle);
    event_manager::remove_callback(field_2C, event::THROW_END, actor_handle);
}

void std_default_trans_inode::refresh_parameters()
{
    enabled = my_param_block.get_optional_pb_int(string_hash{to_hash("enabled")}, 1, nullptr) != 0;
    hit_react_enabled = my_param_block.get_optional_pb_int(string_hash{to_hash("hit_react_enabled")}, 1, nullptr) != 0;
    hit_avoid_enabled = my_param_block.get_optional_pb_int(string_hash{to_hash("hit_avoid_enabled")}, 1, nullptr) != 0;
    unconscious_trigger =
        static_cast<float>(my_param_block.get_optional_pb_int(string_hash{to_hash("unconscious_trigger")}, 0, nullptr));
}

void std_default_trans_inode::_frame_advance(Float)
{
    if (my_param_block.field_0 >= g_world_ptr->time_manager.field_C - 1)
        refresh_parameters();
}

void std_default_trans_inode::set_enabled(bool a2)
{
    static const string_hash enabled_hash{int(to_hash("enabled"))};

    this->my_param_block.set_pb_int(enabled_hash, a2, true);
    if (a2 != enabled) {
        enabled = a2;
    }
}

namespace {
void default_action(state_trans_action *out)
{
    *out = {NO_ACTION, string_hash{}, static_cast<state_trans_messages>(75), nullptr};
}
void goto_action(state_trans_action *out, const char *name)
{
    *out = {GOTO_STATE, string_hash{name}, static_cast<state_trans_messages>(75), nullptr};
}
bool empty_action(const state_trans_action &action)
{
    return action.the_action == NO_ACTION && action.field_4 == string_hash{} &&
           action.the_message == static_cast<state_trans_messages>(75) && action.field_C == nullptr;
}
unsigned __fastcall default_state_type(base_state *, void *)
{
    return 370;
}
bool __fastcall default_state_subclass(base_state *, void *, unsigned type)
{
    return type == 567 || type == 573;
}
state_trans_messages __fastcall default_state_advance(base_state *, void *, Float)
{
    return static_cast<state_trans_messages>(75);
}
void __fastcall default_state_message(base_state *, void *, state_trans_action *out, Float, state_trans_messages)
{
    default_action(out);
}
void __fastcall default_state_nodes(base_state *, void *, info_node_desc_list *list)
{
    list->add_entry({std_default_trans_inode::default_id, 371});
    list->add_entry({combat_inode::default_id, 342});
}
void __fastcall default_falling(base_state *, void *, state_trans_action *out, actor *, std_default_trans_inode *node,
                                physical_interface *physics)
{
    default_action(out);
    if (node->field_24 && physics != nullptr && (physics->field_C & 0x200) && !physics->is_effectively_standing() &&
        g_world_ptr->time_manager.field_8 - physics->field_180 > 0.25f)
        goto_action(out, "falling");
}
void __fastcall default_react(base_state *, void *, state_trans_action *out, actor *owner,
                              std_default_trans_inode *node, combat_inode *combat, Float elapsed)
{
    default_action(out);
    if (!node->hit_react_enabled || combat == nullptr)
        return;
    auto needs_react =
        reinterpret_cast<bool(__fastcall *)(combat_inode *, void *, Float)>(get_vfunc(combat->m_vtbl, 0xE0));
    if (!needs_react(combat, nullptr, elapsed))
        return;

    if (owner->field_88 != nullptr && owner->field_88->field_18 != 1) {
        bool found = false;
        auto incoming = reinterpret_cast<combat_inode::incoming_move *(__fastcall *)(combat_inode *, void *, int)>(
            get_vfunc(combat->m_vtbl, 0xCC));
        for (int i = 0; i < 4 && !found; ++i) {
            auto *move = incoming(combat, nullptr, i);
            if (move->field_10 != 0)
                found = move->field_14.field_28 == 1;
        }
    }
    goto_action(out, "hit_react");
}
void __fastcall default_avoid(base_state *, void *, state_trans_action *out, actor *, std_default_trans_inode *node,
                              combat_inode *combat, Float elapsed)
{
    default_action(out);
    if (node->hit_avoid_enabled && combat != nullptr) {
        auto needs_avoid =
            reinterpret_cast<bool(__fastcall *)(combat_inode *, void *, Float)>(get_vfunc(combat->m_vtbl, 0xE4));
        if (needs_avoid(combat, nullptr, elapsed))
            goto_action(out, "hit_avoid");
    }
}
void __fastcall default_unconscious(base_state *, void *, state_trans_action *out, actor *owner,
                                    std_default_trans_inode *node, damage_interface *)
{
    default_action(out);
    if (node->field_25 && owner->damage_ifc() != nullptr &&
        owner->damage_ifc()->field_1FC.field_0[0] <= node->unconscious_trigger)
        goto_action(out, "unconscious");
}
void __fastcall default_subdued(base_state *, void *, state_trans_action *out, actor *owner,
                                std_default_trans_inode *node, damage_interface *)
{
    default_action(out);
    if (node->field_26 && owner->damage_ifc() != nullptr && owner->damage_ifc()->is_subdued())
        goto_action(out, "subdued");
}
void __fastcall default_state_check(base_state *self, void *, state_trans_action *out, Float elapsed)
{
    auto *core = self->get_core();
    auto *node = static_cast<std_default_trans_inode *>(core->get_info_node(std_default_trans_inode::default_id, true));
    default_action(out);
    if (!node->enabled)
        return;
    auto *owner = core->get_actor(0);
    auto *combat = static_cast<combat_inode *>(core->get_info_node(combat_inode::default_id, true));
    using physical_check = void(__fastcall *)(
        base_state *, void *, state_trans_action *, actor *, std_default_trans_inode *, physical_interface *);
    using combat_check = void(__fastcall *)(
        base_state *, void *, state_trans_action *, actor *, std_default_trans_inode *, combat_inode *, Float);
    using damage_check = void(__fastcall *)(
        base_state *, void *, state_trans_action *, actor *, std_default_trans_inode *, damage_interface *);
    if (owner->has_physical_ifc())
        reinterpret_cast<physical_check>(get_vfunc(self->m_vtbl, 0x38))(
            self, nullptr, out, owner, node, owner->physical_ifc());
    if (!empty_action(*out))
        return;
    reinterpret_cast<combat_check>(get_vfunc(self->m_vtbl, 0x3C))(self, nullptr, out, owner, node, combat, elapsed);
    if (!empty_action(*out))
        return;
    if (owner->has_damage_ifc())
        reinterpret_cast<damage_check>(get_vfunc(self->m_vtbl, 0x48))(
            self, nullptr, out, owner, node, owner->damage_ifc());
    if (!empty_action(*out))
        return;
    if (owner->has_damage_ifc())
        reinterpret_cast<damage_check>(get_vfunc(self->m_vtbl, 0x44))(
            self, nullptr, out, owner, node, owner->damage_ifc());
    if (!empty_action(*out))
        return;
    reinterpret_cast<combat_check>(get_vfunc(self->m_vtbl, 0x40))(self, nullptr, out, owner, node, combat, elapsed);
}
}  // namespace

void *std_default_state_set_base::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 19> result;
        std::copy_n(static_cast<void **>(base_state::native_vtable()), 14, result.begin());
        result[3] = reinterpret_cast<void *>(&default_state_type);
        result[4] = reinterpret_cast<void *>(&default_state_subclass);
        result[8] = reinterpret_cast<void *>(&default_state_advance);
        result[9] = reinterpret_cast<void *>(&default_state_nodes);
        result[11] = reinterpret_cast<void *>(&default_state_check);
        result[12] = reinterpret_cast<void *>(&default_state_message);
        result[14] = reinterpret_cast<void *>(&default_falling);
        result[15] = reinterpret_cast<void *>(&default_react);
        result[16] = reinterpret_cast<void *>(&default_avoid);
        result[17] = reinterpret_cast<void *>(&default_unconscious);
        result[18] = reinterpret_cast<void *>(&default_subdued);
        return result;
    }();
    return const_cast<void **>(table.data());
}

std_default_state_set_base::std_default_state_set_base() : base_state()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}
std_default_state_set_base::std_default_state_set_base(from_mash_in_place_constructor *) : base_state(0)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

}  // namespace ai
