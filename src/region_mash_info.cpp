#include "region_mash_info.h"

#include "common.h"
#include "memory.h"

VALIDATE_SIZE(region_mash_info, 0x40u);

region_mash_info::region_mash_info() {}

void *region_mash_info::operator new(size_t size)
{
    return mem_alloc(size);
}
