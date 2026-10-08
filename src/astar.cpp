#include "astar.h"
#include "astar_node.h"
#include "func_wrapper.h"
#include "vtbl.h"
#include "common.h"

VALIDATE_SIZE(astar_search_record, 0x24);
VALIDATE_SIZE(astar_node, 0x18);

slot_pool<astar_node, unsigned int> &astar_search_record::default_node_pool()
{
#if STANDALONE_SYSTEM
    static slot_pool<astar_node, unsigned int> pool(192);
    return pool;
#else
    return var<slot_pool<astar_node, unsigned int>>(0x00958BF0);
#endif
}

namespace {
void raise_priority(astar_priority_queue &queue, unsigned int index, astar_node *node)
{
    while (index != 0) {
        const auto parent = (index - 1) / 2;
        if (queue[parent]->total_cost <= node->total_cost)
            break;
        queue[index] = queue[parent];
        index = parent;
    }
    queue[index] = node;
}

void push_priority(astar_priority_queue &queue, astar_node *node)
{
    queue.push_back(node);
    raise_priority(queue, queue.size() - 1, node);
}

bool assign_handle(astar_search_record *search, void *node, unsigned int handle)
{
    using callback = bool(__fastcall *)(astar_search_record *, void *, void *, unsigned int);
    return reinterpret_cast<callback>(get_vfunc(search->m_vtbl, 0))(search, nullptr, node, handle);
}
}

astar_node *pop_star_priority_queue(astar_priority_queue &a1)
{
    assert(!a1.empty());
    auto *result = a1[0];
    auto *replacement = a1.back();
    a1.pop_back();
    if (!a1.empty()) {
        unsigned int index = 0;
        unsigned int child = 2;
        const auto count = a1.size();
        while (child < count) {
            if (a1[child]->total_cost > a1[child - 1]->total_cost)
                --child;
            a1[index] = a1[child];
            index = child;
            child = 2 * child + 2;
        }
        if (child == count) {
            a1[index] = a1[count - 1];
            index = count - 1;
        }
        raise_priority(a1, index, replacement);
    }
    return result;
}

bool is_astar_priority_queue_empty(astar_priority_queue &a1)
{
    return a1.empty();
}

void astar_search_record::setup(void *search_start, void *a3, _std::vector<void *> *a4,
                                slot_pool<astar_node, uint32_t> *a5)
{
    this->field_4 = a3;
    this->m_node_pool = a5 != nullptr ? a5 : &default_node_pool();
    this->field_8.clear();
    this->path_goal_to_start = a4;
    this->field_1C = false;
    this->goal_found = false;
    assert(get_astar_node_handle(search_start) == 0);
    this->create_or_update_astar_node(search_start, 0, 0.0f);
}

bool astar_search_record::search(unsigned int a2)
{
    if (this->field_1C) {
        return true;
    }

    assert(path_goal_to_start->size() == 0);

    assert(!goal_found);

    if (a2) {
        assert(m_node_pool != &default_node_pool() && "Can't use default node pool on a multi-pass a-star search");
    } else {
        a2 = -1;
    }

    astar_node *v5 = nullptr;
    unsigned int v10 = 0;
    while (1) {
        if (is_astar_priority_queue_empty(this->field_8)) {
            this->field_1C = true;
            this->clean_up();
            return true;
        }

        v5 = pop_star_priority_queue(this->field_8);
        if (this->get_cost_estimate_to_goal(v5->field_0, this->field_4) <= 0.0f)
            break;

        auto v6 = this->reset_neighbor_iterator(v5->field_0);
        for (auto *i = (void *)this->get_next_neighbor(v5->field_0, v6); i != nullptr;
             i = (void *)this->get_next_neighbor(v5->field_0, v6)) {
            auto v8 = v5->parent_handle;
            if (!v8 || i != (void *)this->m_node_pool->slots[this->m_node_pool->field_0 & v8].field_4.field_0) {
                auto v9 = this->get_travel_cost(v5->field_0, i);
                if (v9 >= 0.0f) {
                    auto v11 = v9;
                    this->create_or_update_astar_node(i, v5->field_4, v11);
                }
            }
        }

        if (++v10 >= a2)
            return 0;
    }

    this->goal_found = true;
    this->field_1C = true;
    this->construct_path_to_goal(v5->field_4);
    this->clean_up();
    return true;
}

float astar_search_record::get_cost_estimate_to_goal(void *a1, void *a2)
{
    float(__fastcall * func)(void *, int, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0x14));
    return func(this, 0, a1, a2);
}

void *astar_search_record::reset_neighbor_iterator(void *a1)
{
    void *(__fastcall * func)(astar_search_record *, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0x8));
    return func(this, nullptr, a1);
}

int astar_search_record::get_next_neighbor(void *a1, void *a2)
{
    int(__fastcall * func)(astar_search_record *, void *, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    return func(this, nullptr, a1, a2);
}

float astar_search_record::get_travel_cost(void *a1, void *a2)
{
    float(__fastcall * func)(astar_search_record *, void *, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0x10));
    return func(this, nullptr, a1, a2);
}

int astar_search_record::get_astar_node_handle(void *a1)
{
    int(__fastcall * func)(astar_search_record *, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    return func(this, nullptr, a1);
}

void astar_search_record::clean_up()
{
    if (m_node_pool == nullptr)
        return;
    for (int i = 0; i < m_node_pool->MAX_SLOTS; ++i) {
        auto &slot = m_node_pool->slots[i];
        if ((slot.id & m_node_pool->field_8) != 0)
            assign_handle(this, slot.field_4.field_0, 0);
    }
    m_node_pool->sub_64A510();
}

void astar_search_record::construct_path_to_goal(unsigned int a2)
{
    while (a2 != 0) {
        auto *node = m_node_pool->get_slot_contents_ptr(a2);
        assert(node != nullptr);
        path_goal_to_start->push_back(node->field_0);
        a2 = node->parent_handle;
    }
}

void astar_search_record::create_or_update_astar_node(void *a2, unsigned int a3, Float a4)
{
    auto &pool = *m_node_pool;
    const float parent_cost = a3 != 0 ? pool.slots[a3 & pool.field_0].field_4.travel_cost : 0.0f;
    const float travel_cost = parent_cost + a4;
    const auto existing_handle = static_cast<unsigned int>(get_astar_node_handle(a2));
    if (existing_handle != 0) {
        auto &node = pool.slots[existing_handle & pool.field_0].field_4;
        const float total_cost = node.total_cost - node.travel_cost + travel_cost;
        if (total_cost >= node.total_cost)
            return;
        node.travel_cost = travel_cost;
        node.field_4 = existing_handle;
        node.parent_handle = a3;
        node.total_cost = total_cost;
        if (node.in_priority_queue) {
            for (unsigned int i = 0; i < field_8.size(); ++i) {
                if (field_8[i]->field_0 == node.field_0) {
                    raise_priority(field_8, i, &node);
                    break;
                }
            }
        } else {
            push_priority(field_8, &node);
            node.in_priority_queue = true;
        }
        return;
    }
    if (pool.field_38 == 0) {
        for (int i = 0; i < pool.MAX_SLOTS && pool.field_38 < 8; ++i) {
            if ((pool.slots[i].id & pool.field_8) == 0)
                pool.field_18[pool.field_38++] = i;
        }
    }
    assert(pool.field_38 != 0);
    const auto index = pool.field_18[--pool.field_38];
    auto &slot = pool.slots[index];
    slot.id = pool.field_4 + (slot.id | pool.field_8);
    ++pool.field_10;
    if (!assign_handle(this, a2, slot.id)) {
        slot.id &= ~pool.field_8;
        if (pool.field_38 < 8)
            pool.field_18[pool.field_38++] = index;
        --pool.field_10;
        return;
    }
    auto &node = slot.field_4;
    node.field_0 = a2;
    node.field_4 = slot.id;
    node.parent_handle = a3;
    node.travel_cost = travel_cost;
    node.total_cost = get_cost_estimate_to_goal(a2, field_4) + travel_cost;
    node.in_priority_queue = true;
    push_priority(field_8, &node);
}
