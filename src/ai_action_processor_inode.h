#pragma once

#include "info_node.h"

#include "mVectorBasic.h"

namespace ai {
struct ai_action_nugget;

struct ai_action_processor_inode : info_node {
    mVectorBasic<ai_action_nugget *> *m_active_actions_list;

    static inline string_hash default_id{int(to_hash("ai_action_processor"))};
};
}  // namespace ai
