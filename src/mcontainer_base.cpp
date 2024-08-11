#include "mcontainer_base.h"

#include "common.h"

VALIDATE_SIZE(mContainer_base, 0x8);

mContainer_base::mContainer_base()
{
    this->initialize(mash::ALLOCATED);
}

mContainer_base::mContainer_base(from_mash_in_place_constructor *)
{
    this->initialize(mash::FROM_MASH);
}

void mContainer_base::initialize(mash::allocation_scope a2)
{
    if ( a2 == mash::ALLOCATED ) {
        this->clear();
    }
}

void mContainer_base::clear()
{
    this->m_size = 0;
    this->field_0 = 0;
}

bool mContainer_base::is_pointer_in_mash_image(void *a3) const
{
    return int(a3) >= int(this) && int(a3) <= int(this) + this->field_0;
}

void mContainer_base::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);
}
