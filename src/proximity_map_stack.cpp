#include "proximity_map_stack.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(dynamic_proximity_map_stack, 0x18);

Var<dynamic_proximity_map_stack *[number_of_district_proximity_map_stacks]> district_proximity_map_stacks {
    0x0095C928
};

static void *__fastcall allocate_map_stack(dynamic_proximity_map_stack *stack, void *, int size)
{
    return stack->alloc(size);
}


dynamic_proximity_map_stack::dynamic_proximity_map_stack()
{
    alignment = 4;
    struct table {
        void *(__fastcall *allocate)(dynamic_proximity_map_stack *, void *, int);
    };
    static const table callbacks{allocate_map_stack};
    m_vtbl = reinterpret_cast<std::intptr_t>(&callbacks);
    storage.allocate(0x4000, 4, 16);
}

void *dynamic_proximity_map_stack::alloc(int size)
{
    return storage.push(size);
}

void init_proximity_map_stacks()
{
    if constexpr (STANDALONE_SYSTEM) {
        auto &stacks = district_proximity_map_stacks();
        if (stacks[0] != nullptr)
            return;

        var<int>(0x00921E40) = number_of_district_proximity_map_stacks;
        var<uint32_t>(0x0095C948) = 0;
        for (auto &stack : stacks) {
            stack = new dynamic_proximity_map_stack{};
        }
    } else {
        CDECL_CALL(0x0053B860);
    }
}
