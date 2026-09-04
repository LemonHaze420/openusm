#include "fixed_allocator_base.h"

#include "common.h"
#include "memory.h"

#include <cassert>

VALIDATE_SIZE(fixed_allocator_base, 0x8);

fixed_allocator_base::fixed_allocator_base(int align) : allocator_base(align)
{
    this->m_vtbl = 0x00882CC8;
}

void *fixed_allocator_base::alloc(int size)
{
    return arch_memalign(this->m_alignment, size);
}

void fixed_allocator_base::release(void *)
{
    assert(nullptr && "fixed_allocator_base cannot release memory");
}
