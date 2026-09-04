#include "enhanced_state.h"

#include "base_ai_state_machine.h"
#include "common.h"
#include "func_wrapper.h"
#include "mashed_state.h"
#include "param_block.h"
#include "state_trans_action.h"
#include "trace.h"
#include "vtbl.h"

#include <cassert>

namespace ai {

VALIDATE_SIZE(enhanced_state, 0x30);

enhanced_state::enhanced_state()
{
    this->field_C = nullptr;
    this->my_mashed_state = nullptr;
    this->field_18 = nullptr;
    this->field_14 = nullptr;
    this->field_28 = false;
}

ai::enhanced_state::enhanced_state(from_mash_in_place_constructor *)
{
    this->field_C = nullptr;
    this->my_mashed_state = nullptr;
    this->field_18 = nullptr;
    this->field_14 = nullptr;
}

float enhanced_state::get_timeout_timer()
{
    return this->field_24;
}

bool enhanced_state::can_handle_message(state_trans_messages the_msg, bool a3) const
{
    TRACE("enhanced_state::can_handle_message");

    assert(the_msg >= 0);

    assert(the_msg < TRANS_TOTAL_MSGS);

    if constexpr (1) {
        bool result = false;
        const auto &pb = this->my_mashed_state->field_0;
        if (pb.does_parameter_exist(to_state_hashes()[the_msg])) {
            auto pb_hash = pb.get_pb_hash(to_state_hashes()[the_msg]);
            result = this->get_machine()->can_switch_to_state(pb_hash);
        } else {
            if (a3 && pb.does_parameter_exist(to_state_always_hash)) {
                auto v11 = pb.get_pb_hash(to_state_always_hash);
                result = this->get_machine()->can_switch_to_state(v11);
            } else {
                auto v13 = exit_layer_hashes()[the_msg];
                if (pb.does_parameter_exist(v13)) {
                    return true;
                }

                if (a3) {
                    if (pb.does_parameter_exist(exit_layer_always_hash)) {
                        return false;
                    }
                }
            }
        }

        return result;
    } else {
        bool(__fastcall * func)(const void *, void *edx, state_trans_messages, bool) = CAST(func, 0x006CE3D0);
        return func(this, nullptr, the_msg, a3);
    }
}

void enhanced_state::activate(ai_state_machine *a2, const mashed_state *a3, const mashed_state *a4,
                              const param_block *a5, base_state::activate_flag_e a6)
{
    if constexpr (0) {
        base_state::activate(a2, a3, a4, a5, a6);

        this->field_1C = 0.0;
        this->field_28 = 0;
        auto *state = this->my_mashed_state;
        this->field_20 = TRANS_TOTAL_MSGS;
        this->field_24 = -1.0;
        this->field_2C = static_cast<state_trans_actions>(3);

        assert(!is_default_transition_state() && "transition state should inherit from base_state to save CPU cycle");

        if (!is_default_transition_state()) {
            bool v10;

            auto *p_block = &state->field_0;
            for (int i = 0; i < 4; ++i) {
                this->field_24 = p_block->get_optional_pb_float(timeout_hashes()[i], -1.0, &v10);

                if (v10) {
                    this->field_20 = static_cast<state_trans_messages>(i);
                }
            }

            this->field_28 = p_block->does_parameter_exist(to_state_hashes()[3]) ||
                             p_block->does_parameter_exist(exit_layer_hashes()[3]);

            this->field_2C =
                static_cast<state_trans_actions>(p_block->get_optional_pb_int(process_default_trans_hash, 3, &v10));
        }

    } else {
        THISCALL(0x006CE1D0, this, a2, a3, a4, a5, a6);
    }
}

state_trans_action enhanced_state::check_transition(Float a3)
{
    TRACE("ai::enhanced_state::check_transition");

    if (!this->is_default_transition_state()) {
        if (this->field_28 && this->field_4 == 2) {
            state_trans_action trans_action = this->process_message(a3, static_cast<state_trans_messages>(3));
            return trans_action;
        }

        if (this->field_20 != TRANS_TOTAL_MSGS) {
            assert(get_timeout_timer() >= 0.0f);

            if (this->field_1C > this->get_timeout_timer()) {
                state_trans_action trans_action = this->process_message(a3, this->field_20);
                return trans_action;
            }
        }
    }

    return this->get_default_return_code();
}

state_trans_action enhanced_state::process_message(Float a3, state_trans_messages a4) const
{
    TRACE("enhanced_state::process_message");

    sp_log("the_msg = %d", int(a4));

    state_trans_action result;

    if constexpr (0) {
        auto default_return_code = this->get_default_return_code();

        if (this->is_default_transition_state()) {
            result = default_return_code;
        } else if (a4 != 0) {
            auto &pb = this->my_mashed_state->field_0;
            if (pb.does_parameter_exist(to_state_always_hash)) {
                string_hash v11{default_return_code.the_message};

                result = this->state_exit(to_state_always_hash, v11, a4, default_return_code);

            } else {
                auto &pb = this->my_mashed_state->field_0;
                auto v8 = pb.does_parameter_exist(exit_layer_always_hash);

                if (v8) {
                    result = this->exit_layer(exit_layer_always_hash, a4, default_return_code);
                } else {
                    result = this->state_exit(to_state_hashes()[a4], exit_layer_hashes()[a4], a4, default_return_code);
                }
            }
        } else {
            result = this->process_exit_message(a3, TRANS_MACHINE_EXIT_REQUEST_MSG);
        }

    } else {
        void(__fastcall *
             func)(const enhanced_state *, void *, state_trans_action *out, Float a3, state_trans_messages a5) =
            CAST(func, get_vfunc(m_vtbl, 0x30));

        func(this, nullptr, &result, a3, a4);
    }

    return result;
}

state_trans_action enhanced_state::get_default_return_code() const
{
    TRACE("ai::enhanced_state::get_default_return_code");

    state_trans_action result{this->field_2C, {}, TRANS_TOTAL_MSGS, nullptr};

    return result;
}

state_trans_action enhanced_state::process_exit_message([[maybe_unused]] Float a3, state_trans_messages the_msg) const
{
    assert(the_msg == TRANS_MACHINE_EXIT_REQUEST_MSG);

    state_trans_action action{state_trans_actions::MACHINE_EXIT, string_hash{0}, TRANS_SUCCESS_MSG, nullptr};
    return action;
}

ai::state_trans_messages enhanced_state::frame_advance([[maybe_unused]] Float dt)
{
    assert(!is_default_transition_state());

    this->field_1C += dt;
    return TRANS_TOTAL_MSGS;
}

state_trans_action enhanced_state::state_exit(string_hash a3, string_hash a4, state_trans_messages a5,
                                              state_trans_action a6) const
{
    TRACE("ai::enhanced_state::state_exit");

    if constexpr (0) {
    } else {
        void *(__fastcall * func)(const enhanced_state *,
                                  void *edx,
                                  state_trans_action *,
                                  string_hash a3,
                                  string_hash a4,
                                  state_trans_messages a5,
                                  state_trans_action a6) = CAST(func, 0x006D0C10);

        state_trans_action result;
        func(this, nullptr, &result, a3, a4, a5, a6);

        return result;
    }
}

state_trans_action enhanced_state::exit_layer(string_hash a3, state_trans_messages a4, state_trans_action a5) const
{
    TRACE("ai::enhanced_state::exit_layer");

    if constexpr (0) {
    } else {
        ai::state_trans_action result;
        THISCALL(0x006CE2F0, this, &result, a3, a4, a5);

        return result;
    }
}

}  // namespace ai


ai::state_trans_action *__fastcall ai_enhanced_state_check_transition(ai::enhanced_state *self, void *,
                                                                      ai::state_trans_action *out, Float a3)
{
    *out = self->check_transition(a3);
    return out;
}

ai::state_trans_action *__fastcall ai_enhanced_state_state_exit(ai::enhanced_state *self, void *,
                                                                ai::state_trans_action *out, string_hash a3,
                                                                string_hash a4, ai::state_trans_messages a5,
                                                                ai::state_trans_action a6)
{
    *out = self->state_exit(a3, a4, a5, a6);
    return out;
}

void enhanced_state_patch()
{
    {
        FUNC_ADDRESS(address, &ai::enhanced_state::can_handle_message);
        //SET_JUMP(0x006CE3D0, address);
    }

    {
        auto address = int(&ai_enhanced_state_check_transition);
        SET_JUMP(0x006BD600, address);
    }

    {
        FUNC_ADDRESS(address, &ai::enhanced_state::exit_layer);
        //REDIRECT(0x006D0CFB, address);
        //REDIRECT(0x006D3642, address);
    }

    {
        [[maybe_unused]] auto address = int(&ai_enhanced_state_state_exit);
        //REDIRECT(0x006D35D2, address);
        //REDIRECT(0x006D369E, address);
    }
}
