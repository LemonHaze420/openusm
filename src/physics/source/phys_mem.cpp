#include "phys_mem.h"

#include <cassert>

unsigned int sub_68B67E(char *a1, int a2)
{
    return ~(a2 - 1) & (unsigned int)&a1[a2 - 1];
}

void *phys_memory_heap::allocate_internal(int size, int align)
{
    assert(size > 0);

    auto *mem = (char *)sub_68B67E(this->m_buffer_cur, align);
    if (&mem[size] > this->m_buffer_end) {
        return nullptr;
    }

    this->m_buffer_cur = &mem[size];
    return mem;
}

void *phys_memory_heap::allocate(int size, int align)
{
    auto *addr = allocate_internal(size, align);
    assert(addr != nullptr && "overflow");

    return addr;
}

void phys_memory_heap::init(void *start, int size, int alignment)
{
    assert(m_buffer_start == nullptr);

    assert(m_buffer_end == nullptr);

    assert(m_buffer_cur == nullptr);

    assert(size > 0);

    assert(int(start) % alignment == 0);

    this->m_buffer_start = static_cast<char *>(start);
    this->m_buffer_end = this->m_buffer_start + size;
    this->m_buffer_cur = this->m_buffer_start;
}
