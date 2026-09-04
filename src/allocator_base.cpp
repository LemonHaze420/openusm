#include "allocator_base.h"

#include "common.h"
#include "memory.h"

VALIDATE_SIZE(allocator_base, 0x8);

allocator_base::allocator_base(int align)
{
    this->m_alignment = align;
}

void *allocator_base::alloc(int size)
{
    return arch_memalign(this->m_alignment, size);
}

void allocator_base::release(void *mem)
{
    mem_freealign(mem);
}
