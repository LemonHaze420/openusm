#pragma once

struct stack_allocator;

namespace scratchpad_stack {

extern stack_allocator &stk;

//0x00512760
extern void save_state(stack_allocator *a1);

//0x00512790
extern void restore_state(const stack_allocator &a1);

extern int get_total_allocated_bytes();

extern void reset();

extern void lock();

extern void unlock();

extern void pop(void *a1, int n_bytes);

extern void *alloc(int n_bytes);

extern void initialize();

extern void term();

}  // namespace scratchpad_stack

extern bool sub_512730(void *a1);

extern bool &tlScratchpadLocked;

extern void scratchpad_stack_patch();
