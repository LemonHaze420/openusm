#include "astar.h"
#include "astar_node.h"
#include "func_wrapper.h"
#include "vtbl.h"
#include "common.h"

VALIDATE_SIZE(astar_search_record, 0x24);

astar_node *pop_star_priority_queue(astar_priority_queue &a1)
{
    return bit_cast<astar_node *>(CDECL_CALL(0x0047F760, &a1));
}

bool is_astar_priority_queue_empty(astar_priority_queue &a1)
{
    return a1.empty();
}

void astar_search_record::setup(void *search_start,
                                void *a3,
                                _std::vector<void *> *a4,
                                slot_pool<astar_node, uint32_t> *a5) {
    if constexpr (1) {
        this->field_4 = a3;
        if (a5 != nullptr) {
            this->m_node_pool = a5;
        } else {
            this->m_node_pool = &astar_search_record::default_node_pool();
        }

        this->field_8.clear();

        this->path_goal_to_start = a4;
        this->field_1C = false;
        this->goal_found = false;
        assert(get_astar_node_handle( search_start ) == static_cast<int>(SLOT_POOL_INVALID_HANDLE));

        this->create_or_update_astar_node(search_start, 0, 0.0);

    } else {
        THISCALL(0x00487D40, this, search_start, a3, a4, a5);
    }
}

bool astar_search_record::search(unsigned int a2)
{
    if ( this->field_1C )
    {
        return true;
    }

    assert(path_goal_to_start->size() == 0);

    assert(!goal_found);

    if (a2)
    {
        assert(m_node_pool != &default_node_pool() && "Can't use default node pool on a multi-pass a-star search");
    }
    else
    {
        a2 = -1;
    }

    astar_node *v5 = nullptr;
    unsigned int v10 = 0;
    while ( 1 )
    {
        if ( is_astar_priority_queue_empty(this->field_8) )
        {
            this->field_1C = true;
            this->clean_up();
            return true;
        }

        v5 = pop_star_priority_queue(this->field_8);
        if ( this->get_cost_estimate_to_goal(v5->field_0, this->field_4) <= 0.0f )
            break;

        auto v6 = this->reset_neighbor_iterator(v5->field_0);
        for ( auto *i = (void *)this->get_next_neighbor(v5->field_0, v6);
          i != nullptr;
          i = (void *)this->get_next_neighbor(v5->field_0, v6) )
        {
            auto v8 = v5->parent_handle;
            if ( !v8 || i != (void *)this->m_node_pool->slots[this->m_node_pool->field_0 & v8].field_4.field_0 )
            {
                auto v9 = this->get_travel_cost(v5->field_0, i);
                if ( v9 >= 0.0f)
                {
                    auto v11 = v9;
                    this->create_or_update_astar_node(i, v5->field_4, v11);
                }
            }
        }
        
        if ( ++v10 >= a2 )
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
    float (__fastcall * func)(void *, int, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0x14));
    return func(this, 0, a1, a2);
}

void * astar_search_record::reset_neighbor_iterator(void *a1)
{
    void * (__fastcall *func)(astar_search_record *, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0x8));
    return func(this, nullptr, a1);
}

int astar_search_record::get_next_neighbor(void *a1, void *a2)
{
    int (__fastcall *func)(astar_search_record *, void *, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    return func(this, nullptr, a1, a2);
}

float astar_search_record::get_travel_cost(void *a1, void *a2)
{
    float (__fastcall *func)(astar_search_record *, void *, void *, void *) = CAST(func ,get_vfunc(m_vtbl, 0x10));
    return func(this, nullptr, a1, a2);
}

int astar_search_record::get_astar_node_handle(void *a1)
{
    int (__fastcall *func)(astar_search_record *, void *, void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    return func(this, nullptr, a1);
}

void astar_search_record::clean_up()
{
    THISCALL(0x00475E60, this);
}

void astar_search_record::construct_path_to_goal(unsigned int a2)
{
    THISCALL(0x00483330, this, a2);
}

void astar_search_record::create_or_update_astar_node(void *a2, unsigned int a3, Float a4)
{
    THISCALL(0x004872F0, this, a2, a3, a4);
}
