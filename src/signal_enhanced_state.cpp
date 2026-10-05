#include "signal_enhanced_state.h"

#include "actor.h"
#include "common.h"
#include "event_manager.h"
#include "event_recipient_entry.h"
#include "event_type.h"
#include "mashed_state.h"
#include "wds.h"
#include <algorithm>
#include <array>

namespace ai {

VALIDATE_SIZE(signal_enhanced_state, 0x34);

namespace {

struct signal_transition {
    int message;
    string_hash parameter;
};
const signal_transition signal_transitions[]{
    {1, string_hash{int(to_hash("to_success_on_signal"))}},
    {2, string_hash{int(to_hash("to_failure_on_signal"))}},
    {3, string_hash{int(to_hash("to_interrupt_on_signal"))}},
    {27, string_hash{int(to_hash("to_ranged_on_signal"))}},
    {28, string_hash{int(to_hash("to_ranged_1_on_signal"))}},
    {29, string_hash{int(to_hash("to_ranged_2_on_signal"))}},
    {30, string_hash{int(to_hash("to_ranged_3_on_signal"))}},
    {31, string_hash{int(to_hash("to_ranged_4_on_signal"))}},
    {32, string_hash{int(to_hash("to_ranged_5_on_signal"))}},
    {33, string_hash{int(to_hash("to_ranged_6_on_signal"))}},
    {34, string_hash{int(to_hash("to_ranged_7_on_signal"))}},
    {35, string_hash{int(to_hash("to_ranged_8_on_signal"))}},
    {36, string_hash{int(to_hash("to_ranged_9_on_signal"))}},
};
unsigned __fastcall signal_type(signal_enhanced_state *, void *)
{
    return 536;
}
int __fastcall signal_size(signal_enhanced_state *, void *)
{
    return sizeof(signal_enhanced_state);
}
bool __fastcall signal_subclass(signal_enhanced_state *, void *, mash::virtual_types_enum type)
{
    return type == 535 || type == 567 || type == 573;
}
void __fastcall signal_activate(signal_enhanced_state *self, void *, ai_state_machine *machine,
                                const mashed_state *state, const mashed_state *previous, const param_block *params,
                                base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, params, flags);
}
state_trans_action *__fastcall signal_check(signal_enhanced_state *self, void *, state_trans_action *out, Float time)
{
    *out = self->check_transition(time);
    return out;
}
}  // namespace

void *signal_enhanced_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), result.size(), result.data());
        result[3] = bit_cast<void *>(&signal_type);
        result[4] = bit_cast<void *>(&signal_subclass);
        result[6] = bit_cast<void *>(&signal_activate);
        result[11] = bit_cast<void *>(&signal_check);
        result[13] = bit_cast<void *>(&signal_size);
        return result;
    }();
    return table.data();
}

signal_enhanced_state::signal_enhanced_state() : field_30(false)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00873858;
}

signal_enhanced_state::signal_enhanced_state(from_mash_in_place_constructor *a2) : enhanced_state(a2)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00873858;
}

void signal_enhanced_state::activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                     const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    field_30 = false;
    if (my_mashed_state != nullptr) {
        const auto &pb = my_mashed_state->field_0;
        for (const auto &transition : signal_transitions) {
            if (pb.does_parameter_exist(transition.parameter)) {
                field_30 = true;
                event_manager::register_event_type(pb.get_pb_hash(transition.parameter), true);
            }
        }
    }
}

state_trans_action signal_enhanced_state::check_transition(Float time)
{
    if (my_mashed_state != nullptr && field_30) {
        const auto &pb = my_mashed_state->field_0;
        for (const auto &transition : signal_transitions) {
            if (!pb.does_parameter_exist(transition.parameter))
                continue;


            auto *type = event_manager::get_event_type(pb.get_pb_hash(transition.parameter));
            if (type == nullptr || !type->field_28)
                continue;
            auto *recipient = type->find_recipient_entry(get_actor()->my_handle);
            if (recipient == nullptr)
                continue;
            const int ticks = g_world_ptr->time_manager.field_C;
            if (recipient->field_24 == ticks - 1 || (recipient->field_24 != ticks && recipient->field_20 == ticks - 1))
                return base_state::process_message(time, static_cast<state_trans_messages>(transition.message));
        }
    }
    return enhanced_state::check_transition(time);
}
}  // namespace ai
