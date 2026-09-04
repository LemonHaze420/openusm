#include "memory.h"

#include "debugutil.h"
#include "func_wrapper.h"
#include "os_developer_options.h"
#include "slab_allocator.h"
#include "utility.h"
#include "variable.h"
#include "variables.h"

#include <cstdio>
#include <cstdlib>

#ifndef STANDALONE_SYSTEM
#error Macro STANDALONE_SYSTEM must be defined!
#endif

//#if !STANDALONE_SYSTEM
#if 0
bool & mem_first_malloc = var<bool>(0x009224F0);
bool & mem_first_memalign = var<bool>(0x009224F1);
bool & mem_first_allocation = var<bool>(0x009224E8);

int & mem_total_allocated = var<int>(0x00965EC0);
#else

#define make_var(type, name) \
    static type g_##name{};  \
    type &name = g_##name

make_var(bool, mem_first_malloc);
make_var(bool, mem_first_memalign);
make_var(bool, mem_first_allocation);

make_var(int, mem_total_allocated);

#undef make_var

#endif

int mem_set_checkpoint()
{
    return 0;
}

void mem_check_leaks_since_checkpoint(int, uint32_t)
{
    ;
}

void *mem_alloc(size_t Size)
{
    //TRACE("mem_alloc");

    void *mem;

    if (slab_allocator::get_max_object_size() < Size) {
        mem = operator new(Size);
    } else {
        mem = slab_allocator::allocate(Size, nullptr);
    }

    return mem;
}

void mem_dealloc(void *a1, size_t Size)
{
    //TRACE("mem_dealloc");

    if (Size <= slab_allocator::get_max_object_size()) {
        slab_allocator::deallocate(a1, nullptr);
    } else {
        operator delete(a1);
    }
}

//0x0058EC30
void *arch_memalign_internal(size_t Alignment, size_t Size)
{
    //TRACE("arch_memalign_internal");

    if constexpr (1) {
        void *result = _aligned_malloc(Size, Alignment);
        void *v3 = result;
        if (result != nullptr) {
            result = v3;
            mem_total_allocated += _msize(*(void **)(((unsigned int)result & 0xFFFFFFFC) - 4));
        }
        return result;
    } else {
        return bit_cast<void *>(CDECL_CALL(0x0058EC30, Alignment, Size));
    }
}

void mem_on_first_allocation()
{
    if (mem_first_allocation) {
        debug_print_va("MEMTRACK is OFF");
        mem_print_stats("very first allocation");
        mem_first_allocation = false;
    }
}

void *arch_memalign(size_t Alignment, size_t Size)
{
    //TRACE("arch_memalign");

    if constexpr (1) {
        if (mem_first_memalign) {
            mem_on_first_allocation();

            mem_first_memalign = false;
        }

        auto *mem = arch_memalign_internal(Alignment, Size);
        if (mem == nullptr) {
            debug_print_va("tried to allocate %d bytes", Size);
            mem_print_stats("mem_memalign failed");
        }

        return mem;

    } else {
        return (void *)CDECL_CALL(0x005357B0, Alignment, Size);
    }
}

void mem_freealign(void *Memory)
{
    TRACE("mem_freealign");

    if (Memory != nullptr) {
        mem_total_allocated -= _msize(*(void **)(((unsigned int)Memory & 0xFFFFFFFC) - 4));
        _aligned_free(Memory);
    }
}

void mem_print_stats(const char *a1)
{
    debug_print_va("mem_print_stats: %s\n", a1);
    debug_print_va("peak: %10lu   curr: %10lu   free: %10lu\n", 0ul, 0ul, 0ul);
}

void *arch_malloc(size_t Size)
{
    //TRACE("arch_malloc");

    if (mem_first_malloc) {
        mem_on_first_allocation();

        mem_first_malloc = false;
    }

    auto *mem = malloc(Size);
    mem_total_allocated += _msize(mem);

    if (mem == nullptr) {
        debug_print_va("tried to allocate %d bytes", Size);
        mem_print_stats("mem_memalloc failed");

        os_developer_options().set_flag(mString{"ENABLE_LONG_CALLSTACK"}, false);
    }

    return mem;
}

int mem_get_total_alloced(int)
{
    return mem_total_allocated;
}

void memory_patch()
{
    SET_JUMP(0x0051CC90, mem_get_total_alloced);

    SET_JUMP(0x0059F684, arch_malloc);

    SET_JUMP(0x005357B0, arch_memalign);

    SET_JUMP(0x0058EC30, arch_memalign_internal);

    SET_JUMP(0x0058EC80, mem_freealign);
}
