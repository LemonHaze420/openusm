#pragma once

#include "variable.h"

#include <cstdint>

static constexpr auto number_of_district_proximity_map_stacks = 8;

struct dynamic_proximity_map_stack {
    std::intptr_t m_vtbl;
    int field_4;
    int field_8;
    int field_C;
    char *field_10;
    char *field_14;

    dynamic_proximity_map_stack();

    //0x0055E8E0
    //virtual
    void *alloc(int size);

    //0x00787770
    //virtual
    void release(void *);
};

//0x0053B860
extern void init_proximity_map_stacks();

extern Var<dynamic_proximity_map_stack *[number_of_district_proximity_map_stacks]> district_proximity_map_stacks;
