#include "resource_allocation_pool.h"

#include "common.h"
#include "parse_generic_mash.h"

VALIDATE_SIZE(resource_allocation_pool, 12u);

void resource_allocation_pool::un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *)
{
    ;
}
