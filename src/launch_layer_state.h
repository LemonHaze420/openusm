#pragma once

#include "signal_enhanced_state.h"

#include "resource_key.h"

namespace ai {
struct launch_layer_state : signal_enhanced_state {
    resource_key field_34;
    int field_3C;
    int field_40;

    launch_layer_state();
    static void *native_vtable();
    void _destruct_mashed_class();
    void _unmash(mash_info_struct *info, void *base);
    void activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                  const param_block *params, activate_flag_e flags);
    void deactivate(const mashed_state *next);
    state_trans_messages frame_advance(Float time);
    void get_state_graph_list(state_graph_list &graphs);
    int get_block_level() const;

    launch_layer_state(from_mash_in_place_constructor *a2);

    //virtual
    [[nodiscard]] resource_key get_layer_resource_key();

    static inline Var<string_hash> layer_to_launch_hash{0x0096C0E4};
};
}  // namespace ai
