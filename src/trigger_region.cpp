#include "trigger_region.h"

#include "common.h"

VALIDATE_SIZE(trigger_region, 0x4);


void trigger_region::unmash(mash_info_struct *, void *)
{
	;
}


int trigger_region::get_mash_sizeof() const
{
	return 4;
}
