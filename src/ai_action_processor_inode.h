#pragma once

#include "info_node.h"

#include "mVectorBasic.h"

namespace ai {
struct ai_action_nugget {
    virtual int frame_advance(Float elapsed) = 0;
    string_hash id;
    ai_core *core;
    bool paused;

    ai_action_nugget(ai_core *owner, string_hash name) : id(name), core(owner), paused(false) {}

protected:
    ~ai_action_nugget() = default;
};


struct ai_action_processor_inode : info_node {
    mVectorBasic<ai_action_nugget *> *m_active_actions_list;

    ai_action_processor_inode();
    ai_action_processor_inode(from_mash_in_place_constructor *constructor);
    void add_action(ai_action_nugget *action);
    void frame_advance(Float elapsed);
    void clear_actions();
    void destruct_mashed_class();

    static inline string_hash default_id{int(to_hash("ai_action_processor"))};
};
}  // namespace ai
