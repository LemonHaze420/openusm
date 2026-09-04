#pragma once

#include <variable.h>

extern void *arch_memalign_internal(size_t Alignment, size_t Size);

//0x005357B0
extern void *arch_memalign(size_t Alignment, size_t Size);

//0x0051CD10
extern void mem_print_stats(const char *a1);

extern int mem_set_checkpoint();

extern void mem_check_leaks_since_checkpoint(int, uint32_t);

//0x00535780
extern void *arch_malloc(size_t Size);

extern void memory_patch();

//0x0058EC80
extern void mem_freealign(void *Memory);

extern void *mem_alloc(size_t Size);

extern void mem_dealloc(void *a1, size_t Size);

extern bool &mem_first_malloc;
extern bool &mem_first_memalign;
extern bool &mem_first_allocation;

extern int &mem_total_allocated;
