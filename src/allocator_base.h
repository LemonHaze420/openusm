#pragma once

struct allocator_base
{
    int m_vtbl;
    int m_alignment;

    allocator_base(int align);

    //virtual
    ~allocator_base() = default;

    //virtual
    void *alloc(int size);

    //virtual
    void release(void *mem);
};
