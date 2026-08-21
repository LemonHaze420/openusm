#include "base_state.h"

#include "actor.h"

#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "common.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "mashed_state.h"

#include <vtbl.h>

namespace ai {

VALIDATE_SIZE(base_state, 0x1C);

base_state::base_state()
{
    this->field_C = nullptr;
    this->my_mashed_state = nullptr;
    this->field_18 = nullptr;
    this->field_14 = nullptr;
}

base_state::base_state(int)
{
    this->field_C = nullptr;
    this->my_mashed_state = nullptr;
    this->field_18 = nullptr;
    this->field_14 = nullptr;
}

actor *base_state::get_actor() const
{
    return this->field_18;
}

ai_core *base_state::get_core() const
{
    return this->field_14;
}

bool base_state::is_subclass_of(mash::virtual_types_enum a1) const
{
    return mash_virtual_base::get_virtual_type_enum() == a1;
}

bool base_state::is_flag_set(int a2) const
{
    auto *mashed_state = this->my_mashed_state;

    return mashed_state->is_flag_set(a2);
}

state_trans_messages base_state::frame_advance(Float a2)
{
    state_trans_messages(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x20));
    return func(this, nullptr, a2);
}

state_trans_action base_state::check_transition(Float a3)
{
    void(__fastcall * func)(base_state *, void *, state_trans_action *, Float) = CAST(func, get_vfunc(m_vtbl, 0x2C));

    state_trans_action action;
    func(this, nullptr, &action, a3);

    return action;
}

state_trans_action base_state::process_message(Float a3, state_trans_messages the_msg) const
{
    void(__fastcall * func)(const base_state *, void *, state_trans_action *, Float, state_trans_messages) =
        CAST(func, get_vfunc(m_vtbl, 0x30));

    state_trans_action action;
    func(this, nullptr, &action, a3, the_msg);

    return action;
}

void base_state::get_info_node_list(info_node_desc_list &) {}

void base_state::get_state_graph_list(state_graph_list &) {}

string_hash base_state::get_name() const
{
    auto result = this->my_mashed_state->get_name();
    return result;
}

void base_state::activate(ai_state_machine *the_state_machine, const mashed_state *a3, const mashed_state *a4,
                          const param_block *a5, base_state::activate_flag_e a6)
{
    if constexpr (0) {
        this->field_4 = a6;
        this->field_C = the_state_machine;
        this->my_mashed_state = a3;
        this->field_10 = a5;
        if (the_state_machine != nullptr) {
            this->field_18 = (actor *)the_state_machine->field_8;
            this->field_14 = the_state_machine->get_core();
        }

        if (a6 != 1) {
            assert(the_state_machine != nullptr);
            assert(the_state_machine->get_core() != nullptr);

            if (a3 != nullptr) {
                string_hash v10 = a3->field_C;

                auto *v8 = this->field_14;
                const auto v9 = 1;

                for (int i = 0; i < v9; ++i) {
                    auto *v12 = v8->field_64;
                    if (v12 != nullptr) {
                        event_manager::raise_event(v10, v12->get_my_handle());
                    }
                }
            }
        }
    } else {
        void(__fastcall * func)(void *,
                                int,
                                ai_state_machine *the_state_machine,
                                const mashed_state *a3,
                                const mashed_state *a4,
                                const param_block *a5,
                                base_state::activate_flag_e a6) = CAST(func, get_vfunc(m_vtbl, 0x18));


        func(this, 0, the_state_machine, a3, a4, a5, a6);
    }
}

void base_state::deactivate(const mashed_state *a2)
{
    void(__fastcall * func)(base_state *, void *, const mashed_state *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    func(this, nullptr, a2);
}

uint32_t base_state::get_virtual_type_enum() const
{
    return 567;
}

int base_state::get_mash_sizeof()
{
    int(__fastcall * func)(base_state *) = CAST(func, get_vfunc(m_vtbl, 0x34));
    return func(this);
}

}  // namespace ai
