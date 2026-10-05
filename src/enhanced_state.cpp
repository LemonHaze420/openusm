#include "enhanced_state.h"

#include "base_ai_res_state_graph.h"
#include "base_ai_state_machine.h"
#include "common.h"
#include "mashed_state.h"
#include "param_block.h"
#include "vtbl.h"

#include <algorithm>
#include <array>
#include <cassert>

namespace ai {
VALIDATE_SIZE(enhanced_state, 0x30);

namespace {

#define STATE_MESSAGE_NAMES(X) \
    X(exit_request)            \
    X(success)                 \
    X(failure)                 \
    X(interrupt)               \
    X(threat)                  \
    X(patrol)                  \
    X(jump)                    \
    X(jump_1)                  \
    X(jump_2)                  \
    X(jump_3)                  \
    X(jump_4)                  \
    X(jump_5)                  \
    X(webzip)                  \
    X(webzip_1)                \
    X(webzip_2)                \
    X(melee)                   \
    X(melee_1)                 \
    X(melee_2)                 \
    X(melee_3)                 \
    X(melee_4)                 \
    X(melee_5)                 \
    X(melee_6)                 \
    X(melee_7)                 \
    X(melee_8)                 \
    X(melee_9)                 \
    X(melee_10)                \
    X(melee_11)                \
    X(ranged)                  \
    X(ranged_1)                \
    X(ranged_2)                \
    X(ranged_3)                \
    X(ranged_4)                \
    X(ranged_5)                \
    X(ranged_6)                \
    X(ranged_7)                \
    X(ranged_8)                \
    X(ranged_9)                \
    X(ranged_10)               \
    X(ranged_11)               \
    X(grab)                    \
    X(grab_1)                  \
    X(grab_2)                  \
    X(teleport)                \
    X(retaliate_1)             \
    X(retaliate_2)             \
    X(chase)                   \
    X(chase_1)                 \
    X(changed_tgt)             \
    X(launch)                  \
    X(land)                    \
    X(dodge)                   \
    X(dodge_2)                 \
    X(react)                   \
    X(recharge)                \
    X(flee_near)               \
    X(flee_near_1)             \
    X(flee_near_2)             \
    X(flee_near_3)             \
    X(flee_far)                \
    X(wait)                    \
    X(point)                   \
    X(spot)                    \
    X(freeze)                  \
    X(taunt)                   \
    X(cheer) X(subdued) X(reset) X(attach) X(obstacle) X(nav_jump) X(feed) X(stand) X(fall) X(crawl) X(transition)
#define TO_STATE_HASH(name) string_hash{int(to_hash("to_state_on_" #name))},
const string_hash to_state_names[]{STATE_MESSAGE_NAMES(TO_STATE_HASH)};
#undef TO_STATE_HASH
#define EXIT_LAYER_HASH(name) string_hash{int(to_hash("exit_layer_on_" #name))},
const string_hash exit_layer_names[]{STATE_MESSAGE_NAMES(EXIT_LAYER_HASH)};
#undef EXIT_LAYER_HASH
#undef STATE_MESSAGE_NAMES
static_assert(std::size(to_state_names) == TRANS_TOTAL_MSGS);
static_assert(std::size(exit_layer_names) == TRANS_TOTAL_MSGS);

uint32_t __fastcall enhanced_type(const enhanced_state *)
{
    return 535;
}
bool __fastcall enhanced_subclass(const enhanced_state *, void *, mash::virtual_types_enum type)
{
    return type == 567 || type == 573;
}
int __fastcall enhanced_size(const enhanced_state *)
{
    return sizeof(enhanced_state);
}
void __fastcall enhanced_activate(enhanced_state *self, void *, ai_state_machine *machine, const mashed_state *state,
                                  const mashed_state *previous, const param_block *params,
                                  base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, params, flags);
}
state_trans_messages __fastcall enhanced_frame(enhanced_state *self, void *, Float dt)
{
    return self->frame_advance(dt);
}
state_trans_action *__fastcall enhanced_transition(enhanced_state *self, void *, state_trans_action *out, Float dt)
{
    *out = self->check_transition(dt);
    return out;
}
state_trans_action *__fastcall enhanced_message(const enhanced_state *self, void *, state_trans_action *out, Float dt,
                                                state_trans_messages message)
{
    *out = self->enhanced_state::process_message(dt, message);
    return out;
}
state_trans_action *__fastcall enhanced_default(const enhanced_state *self, void *, state_trans_action *out)
{
    *out = self->get_default_return_code();
    return out;
}
state_trans_action *__fastcall enhanced_exit(const enhanced_state *self, void *, state_trans_action *out, Float dt,
                                             state_trans_messages message)
{
    *out = self->process_exit_message(dt, message);
    return out;
}
state_trans_action dispatch_default(const enhanced_state *self)
{
    state_trans_action out;
    auto callback =
        reinterpret_cast<state_trans_action *(__fastcall *)(const enhanced_state *, void *, state_trans_action *)>(
            get_vfunc(self->m_vtbl, 0x38));
    callback(self, nullptr, &out);
    return out;
}
}  // namespace

void *enhanced_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result{};
        std::copy_n(static_cast<void **>(base_state::native_vtable()), 14, result.data());
        result[3] = bit_cast<void *>(&enhanced_type);
        result[4] = bit_cast<void *>(&enhanced_subclass);
        result[6] = bit_cast<void *>(&enhanced_activate);
        result[8] = bit_cast<void *>(&enhanced_frame);
        result[11] = bit_cast<void *>(&enhanced_transition);
        result[12] = bit_cast<void *>(&enhanced_message);
        result[13] = bit_cast<void *>(&enhanced_size);
        result[14] = bit_cast<void *>(&enhanced_default);
        result[15] = bit_cast<void *>(&enhanced_exit);
        return result;
    }();
    return table.data();
}

const string_hash *enhanced_state::to_state_hashes()
{
    return to_state_names;
}
const string_hash *enhanced_state::exit_layer_hashes()
{
    return exit_layer_names;
}
const string_hash *enhanced_state::timeout_hashes()
{
    static const std::array<string_hash, TRANS_TOTAL_MSGS> hashes = [] {
        std::array<string_hash, TRANS_TOTAL_MSGS> result;
        result.fill(string_hash{int(to_hash("<INVALID>"))});
        result[1] = string_hash{int(to_hash("timeout_with_success"))};
        result[2] = string_hash{int(to_hash("timeout_with_failure"))};
        result[3] = string_hash{int(to_hash("timeout_with_interrupt"))};
        return result;
    }();
    return hashes.data();
}

enhanced_state::enhanced_state()
{
    if constexpr (STANDALONE_SYSTEM)
        m_vtbl = bit_cast<std::intptr_t>(native_vtable());
    field_28 = false;
}

enhanced_state::enhanced_state(from_mash_in_place_constructor *)
{
    if constexpr (STANDALONE_SYSTEM)
        m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}

float enhanced_state::get_timeout_timer()
{
    return field_24;
}

bool enhanced_state::can_handle_message(state_trans_messages message, bool include_always) const
{
    assert(message >= 0 && message < TRANS_TOTAL_MSGS);
    const auto &pb = my_mashed_state->field_0;
    if (pb.does_parameter_exist(to_state_hashes()[message]))
        return get_machine()->can_switch_to_state(pb.get_pb_hash(to_state_hashes()[message]));
    if (include_always && pb.does_parameter_exist(to_state_always_hash))
        return get_machine()->can_switch_to_state(pb.get_pb_hash(to_state_always_hash));
    return pb.does_parameter_exist(exit_layer_hashes()[message]) ||
           (include_always && pb.does_parameter_exist(exit_layer_always_hash));
}

void enhanced_state::activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                              const param_block *params, activate_flag_e flags)
{
    base_state::_activate(machine, state, previous, params, flags);
    field_1C = 0.0f;
    field_28 = false;
    field_20 = TRANS_TOTAL_MSGS;
    field_24 = -1.0f;
    field_2C = NO_ACTION;
    if (state) {
        const auto &pb = state->field_0;
        for (int i = 0; i < 4; ++i) {
            bool found;
            field_24 = pb.get_optional_pb_float(timeout_hashes()[i], -1.0f, &found);
            if (found) {
                field_20 = static_cast<state_trans_messages>(i);
                break;
            }
        }
        field_28 = pb.does_parameter_exist(to_state_hashes()[3]) || pb.does_parameter_exist(exit_layer_hashes()[3]);
        field_2C = static_cast<state_trans_actions>(pb.get_optional_pb_int(process_default_trans_hash, 3, nullptr));
    }
}

state_trans_action enhanced_state::check_transition(Float dt)
{
    if (!is_default_transition_state()) {
        if (field_28 && field_4 == 2)
            return base_state::process_message(dt, static_cast<state_trans_messages>(3));
        if (field_20 != TRANS_TOTAL_MSGS && field_1C > field_24)
            return base_state::process_message(dt, field_20);
    }
    return dispatch_default(this);
}

state_trans_action enhanced_state::process_message(Float dt, state_trans_messages message) const
{
    auto result = dispatch_default(this);
    if (is_default_transition_state())
        return result;
    if (message == TRANS_MACHINE_EXIT_REQUEST_MSG) {
        auto callback = reinterpret_cast<state_trans_action *(
            __fastcall *)(const enhanced_state *, void *, state_trans_action *, Float, state_trans_messages)>(
            get_vfunc(m_vtbl, 0x3C));
        callback(this, nullptr, &result, dt, message);
        return result;
    }
    const auto &pb = my_mashed_state->field_0;
    if (pb.does_parameter_exist(to_state_always_hash))
        return state_exit(to_state_always_hash, string_hash{0}, message, result);
    if (pb.does_parameter_exist(exit_layer_always_hash))
        return exit_layer(exit_layer_always_hash, message, result);
    return state_exit(to_state_hashes()[message], exit_layer_hashes()[message], message, result);
}

state_trans_action enhanced_state::get_default_return_code() const
{
    return {field_2C, string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
}

state_trans_action enhanced_state::process_exit_message(Float, state_trans_messages message) const
{
    assert(message == TRANS_MACHINE_EXIT_REQUEST_MSG);
    return {MACHINE_EXIT, string_hash{0}, TRANS_SUCCESS_MSG, nullptr};
}

state_trans_messages enhanced_state::frame_advance(Float dt)
{
    field_1C += dt;
    return TRANS_TOTAL_MSGS;
}

state_trans_action enhanced_state::state_exit(string_hash state_key, string_hash exit_key, state_trans_messages message,
                                              state_trans_action default_action) const
{
    const auto &pb = my_mashed_state->field_0;
    if (pb.does_parameter_exist(state_key))
        return {GOTO_STATE, pb.get_pb_hash(state_key), TRANS_TOTAL_MSGS, nullptr};
    if (pb.does_parameter_exist(exit_key) ||
        (message != TRANS_MACHINE_EXIT_REQUEST_MSG &&
         (default_action.the_action != NO_ACTION || !get_machine()->m_state_graph->field_20.m_size)))
        return exit_layer(exit_key, message, default_action);
    return default_action;
}

state_trans_action enhanced_state::exit_layer(string_hash key, state_trans_messages message, state_trans_action) const
{
    const auto &pb = my_mashed_state->field_0;


    const int result = pb.param_array ? pb.param_array->common_find_data(key)->m_union.i : 0;
    return {MACHINE_EXIT,
            string_hash{0},
            result < 0   ? message
            : result > 0 ? TRANS_SUCCESS_MSG
                         : TRANS_FAILURE_MSG,
            nullptr};
}
}  // namespace ai

void enhanced_state_patch() {}
