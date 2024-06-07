#include "anim_info.h"

#include "common.h"
#include "parse_generic_mash.h"

VALIDATE_SIZE(anim_info, 0x24);

void anim_info::un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *a4)
{
    if ( (this->field_1C & 0x2000000) != 0 )
    {
        a4->rebase_shared(8u);

        a4->rebase_shared(4u);

        this->field_14 = a4->get_from_shared<char>(16u);
    }
    else
    {
        this->field_14 = nullptr;
    }

    if ( (this->field_1C & 0x4000000) != 0 )
    {
        a4->rebase(4u);

        this->field_18 = a4->get<char>(0x34);
    }
    else
    {
        this->field_18 = nullptr;
    }
}
