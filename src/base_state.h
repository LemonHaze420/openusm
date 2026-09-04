#pragma once

#include "float.hpp"
#include "mash_virtual_base.h"
#include "state_trans_action.h"
#include "state_trans_messages.h"

#include <cstdint>

struct actor;
struct string_hash;

namespace ai {

struct ai_core;
struct ai_state_machine;
struct param_block;
struct mashed_state;
struct state_trans_action;
struct info_node_desc_list;
struct state_graph_list;

struct base_state : mash_virtual_base {
    enum activate_flag_e {
        ACTIVATE_FLAG_FROM_INTERUPT = 2,
    };

    activate_flag_e field_4;
    const mashed_state *my_mashed_state;
    ai_state_machine *field_C;
    const param_block *field_10;
    ai_core *field_14;
    actor *field_18;

    base_state();

    //0x006BD900
    base_state(int a2);

    actor *get_actor() const;

    ai_core *get_core() const;

    //0x006BD4D0
    string_hash get_name() const;

    bool is_flag_set(int a2) const;

    ai_state_machine *get_machine() const
    {
        return this->field_C;
    }

    /* virtual */ ~base_state() = default;

    //0x006C4BD0
    /* virtual */ void activate(ai_state_machine *the_state_machine, const mashed_state *a3, const mashed_state *a4,
                                const param_block *a5, activate_flag_e a6);

    /* virtual */ void deactivate(const mashed_state *);

    void _deactivate(const mashed_state *) {}

    /* virtual */ state_trans_messages frame_advance(Float a2) /* = 0 */;

    /* virtual */ void get_info_node_list(info_node_desc_list &);

    /* virtual */ void get_state_graph_list(state_graph_list &);

    //virtual
    state_trans_action check_transition(Float a3) /* = 0 */;

    //virtual
    state_trans_action process_message(Float a3, state_trans_messages the_msg) const /* = 0 */;

    //0x0043C5E0
    //virtual
    bool is_subclass_of(mash::virtual_types_enum a1) const;

    //0x006BD940
    uint32_t get_virtual_type_enum() const;

    int get_mash_sizeof();
};
}  // namespace ai
