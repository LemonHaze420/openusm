#pragma once

struct astar_node {
    astar_node();

    void *field_0;
    int field_4;
    unsigned int parent_handle;
    float travel_cost;
    float total_cost;
    bool in_priority_queue;
};
