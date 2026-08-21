#include "proximity_map_stack.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(dynamic_proximity_map_stack, 0x18);

Var<dynamic_proximity_map_stack *[number_of_district_proximity_map_stacks]> district_proximity_map_stacks {
    0x0095C928
};

dynamic_proximity_map_stack::dynamic_proximity_map_stack() {}

void *dynamic_proximity_map_stack::alloc(int size)
{
    auto v2 = ~(this->field_C - 1) & (this->field_C + size - 1);
    auto *v3 = &this->field_14[v2];
    this->field_14 = v3;
    return &v3[-v2];
}

void dynamic_proximity_map_stack::release(void *) {}

void init_proximity_map_stacks()
{
    CDECL_CALL(0x0053B860);
}
