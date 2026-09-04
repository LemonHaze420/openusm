#pragma once

#include "slot_pool.h"
#include "astar_node.h"
#include "float.hpp"
#include "variable.h"

#include <vector.hpp>

using astar_priority_queue = _std::vector<astar_node *>;

struct astar_search_record {
    int m_vtbl;
    void *field_4;
    astar_priority_queue field_8;
    slot_pool<astar_node, uint> *m_node_pool;
    bool field_1C;
    bool goal_found;
    _std::vector<void *> *path_goal_to_start;

    //0x00475E60
    void clean_up();

    //0x00483330
    void construct_path_to_goal(unsigned int a2);

    //0x004872F0
    void create_or_update_astar_node(void *a2, unsigned int a3, Float a4);

    //virtual
    void *reset_neighbor_iterator(void *a1);

    //virtual
    int get_next_neighbor(void *a1, void *a2);

    //virtual
    float get_travel_cost(void *a1, void *a2);

    //virtual
    float get_cost_estimate_to_goal(void *a1, void *a2);

    //virtual
    int get_astar_node_handle(void *a1);

    //0x00487D40
    void setup(void *a2, void *a3, _std::vector<void *> *a4, slot_pool<astar_node, uint32_t> *a5);

    //0x00487DA0
    //virtual
    bool search(unsigned int a2);

    static inline Var<slot_pool<astar_node, unsigned int>> default_node_pool{0x00958BF0};
};
