#include "als_scripted_category.h"

#include "mash_info_struct.h"
#include "utility.h"
#include "trace.h"
#include "func_wrapper.h"
#include "common.h"
#include "mash_config.h"

#include <cassert>

namespace als
{
    VALIDATE_SIZE(scripted_category, 0x7C);

    scripted_category::scripted_category()
    {
        THISCALL(0x004ACBF0, this);
    }

    void scripted_category::_unmash(mash_info_struct *a1, void *a3)
    {
        TRACE("als::scripted_category::unmash");

        if constexpr (1)
        {
            category::_unmash(a1, this);

            a1->unmash_class_in_place(this->field_10, this);

            a1->unmash_class_in_place(this->field_14, this);

            a1->unmash_class_in_place(this->field_2C, this);

            a1->unmash_class_in_place(this->field_3C, this);
            a1->unmash_class_in_place(this->field_50, this);
            a1->unmash_class_in_place(this->field_64, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
            {
                uint8_t class_mashed = -1;
                class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
                assert(class_mashed == 0xAF || class_mashed == 0);
            }
#endif

            if ( this->field_78 != nullptr )
            {
                a1->unmash_class(this->field_78, this
#if OPENUSM_XBOX_MASH_FORMAT
                    , mash::NORMAL_BUFFER
#endif
                        );
            }
        }
        else
        {
            THISCALL(0x004AC850, this, a1, a3);
        }
    }

    request_data scripted_category::do_implicit_trans(
        animation_logic_system *a4,
        state_machine *a5)
    {
        TRACE("als::scripted_category::do_implicit_trans");

        request_data data;
        THISCALL(0x004A7300, this, &data, a4, a5);
        return data;
    }
}


als::request_data * __fastcall als_scripted_category_do_implicit_trans(
    als::scripted_category *self, void *,
    als::request_data *out,
    als::animation_logic_system *a4,
    als::state_machine *a5)
{
    *out = self->do_implicit_trans(a4, a5);
    return out;
}

void als_scripted_category_patch()
{
    {
        FUNC_ADDRESS(address, &als::scripted_category::_unmash);
        set_vfunc(0x0087E254, address);
    }

    {
        auto address = int(&als_scripted_category_do_implicit_trans);
        set_vfunc(0x0087E268, address);
    }
}
