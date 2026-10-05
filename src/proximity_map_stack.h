#pragma once

#include "variable.h"
#include "stack_allocator.h"

#include <cstdint>

static constexpr auto number_of_district_proximity_map_stacks = 8;

struct dynamic_proximity_map_stack {
    std::intptr_t m_vtbl;
    int alignment;
    stack_allocator storage;

    dynamic_proximity_map_stack();
    void *alloc(int size);
};

//0x0053B860
extern void init_proximity_map_stacks();

extern Var<dynamic_proximity_map_stack *[number_of_district_proximity_map_stacks]> district_proximity_map_stacks;
