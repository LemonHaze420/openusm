#pragma once

#include "float.hpp"
#include "mash_virtual_base.h"
#include "state_trans_messages.h"
#include "string_hash.h"

#include <vector.hpp>

struct resource_key;
struct actor;

namespace ai {
struct base_state;
struct mashed_state;
struct param_block;
struct state_trans_action;
struct state_graph;
struct ai_core;

struct ai_state_machine {
    enum {
        PRE_TEST = 0,
        PROCESSING_EXIT_REQUEST = 4,
    } my_curr_mode;
    ai_core *my_core;
    actor *field_8;
    const state_graph *m_state_graph;
    base_state *my_curr_state;
    const mashed_state *field_14;
    ai_state_machine *my_parent;
    _std::vector<ai_state_machine *> field_1C;
    int field_2C;
    string_hash field_30;
    bool field_34;
    void *field_38;
    const mashed_state *field_3C;
    const param_block *field_40;
    int field_44;

    //0x0069EB60
    ai_state_machine(ai::ai_core *a2, const ai::state_graph *a3, string_hash a4);

    //0x0069BCE0
    void process_machine_exit(ai::state_trans_messages a2);

    [[nodiscard]] state_trans_action check_trans_on_interrupt(Float a3, const state_trans_action &a4);

    //0x0069F870
    void advance_curr_state(Float a2, bool a3);

    //0x0069ED60
    void process_transition(Float a2);

    //0x006A1590
    void process_mode(Float a2, bool a3);

    //0x0069ECA0
    void process_transition_message(Float a2, state_trans_messages the_msg);

    //0x00687FD0
    void process_return();

    //0x0069BA60
    void request_exit();

    //0x0069BAA0
    void external_request_exit();

    state_trans_action process_msg_on_interrupt(Float a3, state_trans_messages a4, const ai::state_trans_action &a5);

    bool has_default_transition(mash::virtual_types_enum a2, bool a3) const;

    //0x00688100
    bool has_state(string_hash a2) const;

    //0x00688120
    bool can_switch_to_state(string_hash a2) const;

    //0x0069BAC0
    bool transition_state(string_hash arg0, const param_block *a3);

    //0x0068FD60
    [[nodiscard]] state_trans_action check_keyword_overrides(const state_trans_action &a3) const;

    //0x006880E0
    string_hash get_initial_state_id() const;

    //0x00687F90
    resource_key get_name() const;

    //0x006A1530
    void add_as_child(ai_state_machine *a2);

    auto *get_prev_mashed_state() const
    {
        return this->field_14;
    }

    auto *get_curr_state() const
    {
        return this->my_curr_state;
    }

    inline auto get_core()
    {
        return this->my_core;
    }

    static inline Var<string_hash> prev_state_id_hash{0x0096C124};

    static inline Var<string_hash> initial_state_id_hash{0x0096C460};
};
}  // namespace ai

extern void ai_state_machine_patch();
