#include "color.h"

#include "common.h"
#include "ngl.h"

VALIDATE_SIZE(color, 0x10);

void *color::operator new(size_t size)
{
    auto *mem = nglListAlloc(size, 16);
    return mem;
}
