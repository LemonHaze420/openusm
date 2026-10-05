#include "std_puppet_trans_state.h"

#include "func_wrapper.h"
#include "common.h"
#include "base_ai_core.h"
#include "info_node_desc_list.h"
#include "info_node_descriptor.h"
#include "state_trans_action.h"
#include "std_puppet_inode.h"
#include <algorithm>
#include <array>

namespace ai {
VALIDATE_SIZE(std_puppet_trans_state, 0x1C);

namespace {
unsigned __fastcall puppet_transition_type(std_puppet_trans_state *, void *)
{
    return 316;
}
bool __fastcall puppet_transition_subclass(std_puppet_trans_state *, void *, unsigned type)
{
    return type == 567 || type == 573;
}
state_trans_messages __fastcall puppet_transition_advance(std_puppet_trans_state *, void *, Float)
{
    return TRANS_TOTAL_MSGS;
}
void __fastcall puppet_transition_nodes(std_puppet_trans_state *, void *, info_node_desc_list &nodes)
{
    nodes.add_entry(info_node_descriptor{string_hash{"puppet_inode"}, 315});
}
void __fastcall puppet_transition_check(std_puppet_trans_state *self, void *, state_trans_action *out, Float)
{
    auto *node = static_cast<std_puppet_inode *>(self->get_core()->get_info_node(string_hash{"puppet_inode"}, true));
    const string_hash switch_state{"switch_state"};
    const auto next = node->my_param_block.does_parameter_exist(switch_state)
                          ? node->my_param_block.get_pb_hash(switch_state)
                          : string_hash{0};
    *out = state_trans_action{next.source_hash_code != 0 ? GOTO_STATE : NO_ACTION, next, TRANS_TOTAL_MSGS, nullptr};
    if (next.source_hash_code != 0) {
        node->my_param_block.set_pb_hash(switch_state, string_hash{0}, true);
        node->set_current_state(next);
    }
}
void __fastcall puppet_transition_message(std_puppet_trans_state *, void *, state_trans_action *out, Float,
                                          state_trans_messages)
{
    *out = state_trans_action{NO_ACTION, string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
}
}  // namespace

void *std_puppet_trans_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 14> callbacks{};
        std::copy_n(static_cast<void **>(base_state::native_vtable()), callbacks.size(), callbacks.begin());
        callbacks[3] = reinterpret_cast<void *>(&puppet_transition_type);
        callbacks[4] = reinterpret_cast<void *>(&puppet_transition_subclass);
        callbacks[8] = reinterpret_cast<void *>(&puppet_transition_advance);
        callbacks[9] = reinterpret_cast<void *>(&puppet_transition_nodes);
        callbacks[11] = reinterpret_cast<void *>(&puppet_transition_check);
        callbacks[12] = reinterpret_cast<void *>(&puppet_transition_message);
        return callbacks;
    }();
    return table.data();
}

std_puppet_trans_state::std_puppet_trans_state()
{
    if constexpr (STANDALONE_SYSTEM)
        m_vtbl = CAST(m_vtbl, native_vtable());
    else
        THISCALL(0x00438C80, this);
}

void std_puppet_trans_state::_unmash(mash_info_struct *a1, void *a2)
{
    base_state::_unmash(a1, a2);
}
}  // namespace ai
