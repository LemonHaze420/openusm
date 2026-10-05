#include "ai_action_processor_inode.h"

#include "common.h"
#include "memory.h"
#include "mash_virtual_base.h"
#include <cstring>

template <>
void mVectorBasic<ai::ai_action_nugget *>::reserve(int capacity)
{
    if (capacity <= m_max_size)
        return;
    auto **data = static_cast<ai::ai_action_nugget **>(
        ::operator new(capacity * sizeof(ai::ai_action_nugget *)));
    if (m_size != 0)
        std::memcpy(data, m_data, m_size * sizeof(ai::ai_action_nugget *));
    if (m_data != nullptr && !is_pointer_in_mash_image(m_data))
        ::operator delete[](m_data);
    m_data = data;
    m_max_size = capacity;
}

template <>
void mVectorBasic<ai::ai_action_nugget *>::destruct_mashed_class()
{
    if (m_data != nullptr && !is_pointer_in_mash_image(m_data))
        ::operator delete[](m_data);
    m_data = nullptr;
    m_max_size = 0;
    clear();
    mContainer_base::destruct_mashed_class();
}

namespace ai {
VALIDATE_SIZE(ai_action_processor_inode, 0x20u);
VALIDATE_SIZE(ai_action_nugget, 0x10u);

ai_action_processor_inode::ai_action_processor_inode()
    : m_active_actions_list(nullptr)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[11]);
}

ai_action_processor_inode::ai_action_processor_inode(
    from_mash_in_place_constructor *constructor)
    : info_node(constructor), m_active_actions_list(nullptr)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[11]);
}

void ai_action_processor_inode::add_action(ai_action_nugget *action)
{
    if (m_active_actions_list == nullptr) {
        m_active_actions_list = new (mem_alloc(sizeof(*m_active_actions_list)))
            mVectorBasic<ai_action_nugget *>{};
        m_active_actions_list->m_data = nullptr;
        m_active_actions_list->m_max_size = 0;
    }
    action->paused = false;
    m_active_actions_list->push_back(action);
}

void ai_action_processor_inode::frame_advance(Float elapsed)
{
    if (m_active_actions_list == nullptr)
        return;
    for (int index = 0; index < m_active_actions_list->m_size;) {
        auto *action = m_active_actions_list->m_data[index];
        if (action->paused || action->frame_advance(elapsed) != 1) {
            ++index;
            continue;
        }
        mem_dealloc(action, sizeof(ai_action_nugget));
        auto *list = m_active_actions_list;
        --list->m_size;
        std::memmove(list->m_data + index, list->m_data + index + 1,
            (list->m_size - index) * sizeof(ai_action_nugget *));
    }
    if (m_active_actions_list->m_size == 0) {
        m_active_actions_list->destruct_mashed_class();
        mem_dealloc(m_active_actions_list, sizeof(*m_active_actions_list));
        m_active_actions_list = nullptr;
    }
}

void ai_action_processor_inode::clear_actions()
{
    if (m_active_actions_list == nullptr)
        return;
    for (int index = 0; index < m_active_actions_list->m_size; ++index)
        mem_dealloc(m_active_actions_list->m_data[index], sizeof(ai_action_nugget));
    m_active_actions_list->destruct_mashed_class();
    mem_dealloc(m_active_actions_list, sizeof(*m_active_actions_list));
    m_active_actions_list = nullptr;
}

void ai_action_processor_inode::destruct_mashed_class()
{
    clear_actions();
    info_node::_destruct_mashed_class();
}

}
