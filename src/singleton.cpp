#include "singleton.h"

#include "common.h"
#include "memory.h"

VALIDATE_SIZE(singleton, 0x4);

void *singleton::operator new(size_t size)
{
    return mem_alloc(size);
}

void singleton::operator delete(void *ptr, size_t sz)
{
    mem_dealloc(ptr, sz);
}
