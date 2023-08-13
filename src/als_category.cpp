#include "als_category.h"

#include "common.h"

#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "param_block.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

namespace als
{
    VALIDATE_SIZE(category, 0x10);

    int category::get_mash_sizeof() const
    {
        auto &func = get_vfunc(m_vtbl, 0x34);
        return func(this);
    }

    void category::_unmash(mash_info_struct *a1, void *a3)
    {
        TRACE("als::category::unmash");

        if constexpr (1)
        {
            a1->unmash_class_in_place(this->field_4, this);

#ifdef TARGET_XBOX
            {
                uint8_t class_mashed = -1;
                class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
                assert(class_mashed == 0xAF || class_mashed == 0);
            }
#endif

            if ( this->field_C != nullptr )
            {
                a1->unmash_class(this->field_C, this
#ifdef TARGET_XBOX
                    , mash::NORMAL_BUFFER
#endif
                        );
            }
        }
        else
        {
            THISCALL(0x0049F110, this, a1, a3);
        }
    }
}

void als_category_patch()
{
    {
        FUNC_ADDRESS(address, &als::category::_unmash);
        set_vfunc(0x00875708, address);
    }
}
