#include "launch_layer_state.h"

#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "common.h"
#include "mashed_state.h"
#include "state_graph_list.h"
#include "vtbl.h"
#include <algorithm>
#include <array>

namespace ai {

VALIDATE_SIZE(launch_layer_state, 0x44);
VALIDATE_SIZE(state_graph_list, 0x10);

namespace {
ai_state_machine *find_child(ai_state_machine *machine, const resource_key &graph)
{
    for (auto *child : machine->field_1C)
        if (child->get_name() == graph)
            return child;
    return nullptr;
}
void __fastcall layer_destroy(launch_layer_state *self, void *)
{
    self->_destruct_mashed_class();
}
void __fastcall layer_unmash(launch_layer_state *self, void *, mash_info_struct *info, void *base)
{
    self->_unmash(info, base);
}
unsigned __fastcall layer_type(launch_layer_state *, void *)
{
    return 330;
}
bool __fastcall layer_subclass(launch_layer_state *, void *, mash::virtual_types_enum type)
{
    return type == 536 || type == 535 || type == 567 || type == 573;
}
void __fastcall layer_activate(launch_layer_state *self, void *, ai_state_machine *machine, const mashed_state *state,
                               const mashed_state *previous, const param_block *params,
                               base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, params, flags);
}
void __fastcall layer_deactivate(launch_layer_state *self, void *, const mashed_state *next)
{
    self->deactivate(next);
}
state_trans_messages __fastcall layer_advance(launch_layer_state *self, void *, Float time)
{
    return self->frame_advance(time);
}
void __fastcall layer_graphs(launch_layer_state *self, void *, state_graph_list &graphs)
{
    self->get_state_graph_list(graphs);
}
int __fastcall layer_size(launch_layer_state *, void *)
{
    return sizeof(launch_layer_state);
}
resource_key *__fastcall layer_resource(launch_layer_state *self, void *, resource_key *out)
{
    *out = self->get_layer_resource_key();
    return out;
}
int __fastcall layer_block(launch_layer_state *self, void *)
{
    return self->get_block_level();
}
resource_key dispatch_layer_resource(launch_layer_state *self)
{
    resource_key result;
    auto fn = reinterpret_cast<resource_key *(__fastcall *)(launch_layer_state *, void *, resource_key *)>(
        get_vfunc(self->m_vtbl, 0x40));
    fn(self, nullptr, &result);
    return result;
}
}  // namespace

void *launch_layer_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 18> result;
        std::copy_n(static_cast<void **>(signal_enhanced_state::native_vtable()), 16, result.data());
        result[0] = bit_cast<void *>(&layer_destroy);
        result[1] = bit_cast<void *>(&layer_unmash);
        result[3] = bit_cast<void *>(&layer_type);
        result[4] = bit_cast<void *>(&layer_subclass);
        result[6] = bit_cast<void *>(&layer_activate);
        result[7] = bit_cast<void *>(&layer_deactivate);
        result[8] = bit_cast<void *>(&layer_advance);
        result[10] = bit_cast<void *>(&layer_graphs);
        result[13] = bit_cast<void *>(&layer_size);
        result[16] = bit_cast<void *>(&layer_resource);
        result[17] = bit_cast<void *>(&layer_block);
        return result;
    }();
    return table.data();
}

launch_layer_state::launch_layer_state() : field_34(string_hash{0}, static_cast<resource_key_type>(0))
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00879550;
}

launch_layer_state::launch_layer_state(from_mash_in_place_constructor *a2) : signal_enhanced_state(a2), field_34(a2)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00879550;
}

void launch_layer_state::_destruct_mashed_class()
{
    field_34.destruct_mashed_class();
}

void launch_layer_state::_unmash(mash_info_struct *info, void *base)
{
    field_34.unmash(info, base);
}

resource_key launch_layer_state::get_layer_resource_key()
{
    return {string_hash{my_mashed_state->field_0.get_pb_fixedstring(layer_to_launch_hash())},
            RESOURCE_KEY_TYPE_AI_STATE_GRAPH};
}

int launch_layer_state::get_block_level() const
{
    return my_mashed_state->field_0.get_optional_pb_int(string_hash{int(to_hash("block_on_layer"))}, 1, nullptr);
}

void launch_layer_state::activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                  const param_block *params, activate_flag_e flags)
{
    signal_enhanced_state::activate(machine, state, previous, params, flags);
    field_34 = dispatch_layer_resource(this);
    auto block = reinterpret_cast<int(__fastcall *)(launch_layer_state *, void *)>(get_vfunc(m_vtbl, 0x44));
    field_3C = block(this, nullptr);
    field_40 = 1;
}

void launch_layer_state::deactivate(const mashed_state *next)
{
    if (field_40 == 2 && field_3C != 0) {
        if (auto *child = find_child(get_machine(), field_34)) {
            if (child->my_curr_mode != ai_state_machine::BLOCKED_ON_ALL_CHILDREN &&
                child->my_curr_mode != ai_state_machine::EXITING_WAIT_ON_CHILDREN &&
                child->my_curr_mode != ai_state_machine::PROCESSING_EXIT_REQUEST)
                child->request_exit();
        }
    }
    if (field_3C != 0)
        get_machine()->field_34 = true;
    base_state::_deactivate(next);
}

state_trans_messages launch_layer_state::frame_advance(Float time)
{
    const auto message = enhanced_state::frame_advance(time);
    auto *machine = get_machine();
    auto *child = find_child(machine, field_34);
    if (field_40 == 1) {
        if (child == nullptr) {
            get_core()->spawn_state_machine_internal(machine, field_34, nullptr, string_hash{0});
            child = find_child(machine, field_34);


            assert(child != nullptr);
            if (field_3C == 0) {
                child->field_34 = false;
                return TRANS_SUCCESS_MSG;
            }
            child->field_34 = machine->field_34;
            machine->field_34 = false;
            if (field_3C == 2) {
                machine->my_curr_mode = ai_state_machine::BLOCKED_ON_CHILD_MACHINE;
                machine->field_2C = reinterpret_cast<int>(child);
            }
            field_40 = 2;
        }
        return message;
    }
    if (field_40 != 2 || child != nullptr)
        return message;
    auto result = TRANS_TOTAL_MSGS;
    for (const auto &completed : get_core()->field_20) {
        if (completed.graph == field_34) {
            result = static_cast<state_trans_messages>(completed.message);
            break;
        }
    }
    if (result != TRANS_TOTAL_MSGS &&
        (result != TRANS_MACHINE_EXIT_REQUEST_MSG || can_handle_message(TRANS_MACHINE_EXIT_REQUEST_MSG, false)))
        return result;
    return TRANS_FAILURE_MSG;
}

void launch_layer_state::get_state_graph_list(state_graph_list &graphs)
{
    graphs.add_entry(dispatch_layer_resource(this).m_hash);
}
}  // namespace ai
