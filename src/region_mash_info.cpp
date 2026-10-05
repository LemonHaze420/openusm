#include "region_mash_info.h"

#include "common.h"
#include "memory.h"

VALIDATE_SIZE(region_mash_info, 0x40u);


region_mash_info::region_mash_info() : field_0{}, field_20{0.25f, 0.25f, 0.25f, 1.0f}, field_30(0), field_3C(0) {}

void *region_mash_info::operator new(size_t size)
{
    return mem_alloc(size);
}
void region_mash_info::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}