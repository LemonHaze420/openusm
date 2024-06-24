#include "scratchpad_stack.h"

#include "func_wrapper.h"
#include "memory.h"
#include "stack_allocator.h"
#include "trace.h"
#include "utility.h"

namespace scratchpad_stack {
Var<stack_allocator> stk {0x0095C724};
}

Var<bool> tlScratchpadLocked{0x00970D60};

void scratchpad_stack::save_state(stack_allocator *a1)
{
    TRACE("scratchpad_stack::save_state");

    *a1 = scratchpad_stack::stk();
}

void scratchpad_stack::restore_state(const stack_allocator &a1)
{
    TRACE("scratchpad_stack::restore_state");

    stk() = a1;
    if (stk().get_total_allocated_bytes() == 0 && tlScratchpadLocked()) {
        unlock();
    }
}

int scratchpad_stack::get_total_allocated_bytes()
{
    return stk().get_total_allocated_bytes();
}

void scratchpad_stack::lock() {
    assert(!tlScratchpadLocked() && "Scratchpad is already locked!");

    tlScratchpadLocked() = true;
}

void scratchpad_stack::unlock() {
    assert(tlScratchpadLocked() && "Scratchpad is already unlocked!");

    tlScratchpadLocked() = false;
}

void scratchpad_stack::reset() {
    return scratchpad_stack::stk().reset();
}

void scratchpad_stack::pop(void *a1, int n_bytes) {
    stk().pop(a1, n_bytes);
    if (get_total_allocated_bytes() == 0) {
        unlock();
    }
}

void *scratchpad_stack::alloc(int n_bytes)
{
    TRACE("scratchpad_stack::alloc");

    assert(n_bytes != 0 && "Invalid scratchpad stack allocation (size 0)");

    if (get_total_allocated_bytes() == 0) {
        lock();
    }

    return stk().push(n_bytes);
}

void scratchpad_stack::initialize()
{
    TRACE("scratchpad_stack::initialize");

    if constexpr (1) {
        auto status = stk().allocate(16384u, 16, 16);
        assert(status);
    } else {
        CDECL_CALL(0x00538D10);
    }
}

void scratchpad_stack::term()
{
    TRACE("scratchpad_stack::term");

    stk().free();
    stk().segment = nullptr;
}

bool sub_512730(void *a1) {
    return a1 >= scratchpad_stack::stk().segment &&
        a1 < &scratchpad_stack::stk().segment[scratchpad_stack::stk().segment_size_bytes];
}

void scratchpad_stack_patch()
{
    REDIRECT(0x005576B0, scratchpad_stack::initialize);
}
