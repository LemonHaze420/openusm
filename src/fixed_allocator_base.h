#pragma once

#include "allocator_base.h"

struct fixed_allocator_base : allocator_base
{
    fixed_allocator_base(int align);

    //virtual
    void *alloc(int size) /* override */;

    //virtual
    void release(void *mem) /* override */;
};
