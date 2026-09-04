#include "nugget_wait_state.h"

#include "als_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "param_block.h"

namespace ai {

VALIDATE_SIZE(nugget_wait_state, 0x3C);

state_trans_messages nugget_wait_state::frame_advance(Float)
{
    state_trans_messages v13 = TRANS_SUCCESS_MSG;
    auto *the_core = this->get_core();
    auto *the_als_i = (ai::als_inode *)the_core->get_info_node(als_inode::default_id, true);
    string_hash v14{0};
    auto v3 = this->field_34;
    if (v3 != v14 && !this->field_38 && !this->field_39) {
        if (the_als_i->is_cat_our_prev_cat(v3, static_cast<als::layer_types>(0))) {
            this->field_39 = true;
        } else {
            v13 = TRANS_TOTAL_MSGS;
        }
    }

    auto *param_array = this->field_30->param_array;
    if (param_array != nullptr) {
        for (auto &data : param_array->field_0) {
            static string_hash s_goto_radius{to_hash("goto_radius")};

            static string_hash s_goto_speed{to_hash("goto_speed")};

            auto v15 = data->get_name();
            if (v15 != wait_anim_hash && v15 != looping_wait_anim_hash && v15 != s_goto_radius && v15 != s_goto_speed) {
                auto *v9 = this->get_core();
                if (!v9->field_50.does_parameter_match(data)) {
                    v13 = TRANS_TOTAL_MSGS;
                }
            }
        }
    }
    return v13;
}

}  // namespace ai
