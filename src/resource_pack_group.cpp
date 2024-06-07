#include "resource_pack_group.h"

#include "func_wrapper.h"
#include "parse_generic_mash.h"

#include "common.h"

VALIDATE_SIZE(resource_pack_group, 0x20u);

int resource_pack_group::get_pack_slot(const mString &a2) {
    return THISCALL(0x0054C950, this, &a2);
}

int resource_pack_group::get_first_free_slot() {
    return THISCALL(0x00531EA0, this);
}

int resource_pack_group::get_num_free_slots() const
{
    auto v1 = this->field_1C;
    auto result = 0;
    if ( v1 > 0 )
    {
        auto *v3 = (int *) this->field_10[3];
        auto *v4 = this->field_14;
        do
        {
            if ( !*(uint32_t *) (v3[*v4] + 12) ) {
                ++result;
            }

            ++v4;
            --v1;
        }
        while ( v1 );
    }

    return result;
}

void resource_pack_group::un_mash(generic_mash_header *a2,
                                  [[maybe_unused]] void *a3,
                                  generic_mash_data_ptrs *a4) {
    if constexpr (1)
    {
        this->field_0 = *a4->get<resource_key>();

        this->field_8.custom_un_mash(a2, &this->field_8, a4, nullptr);

        a4->rebase(4);

        auto v7 = this->field_1C;
        this->field_14 = a4->get<int>(v7);
    }
    else
    {
        THISCALL(0x0051F070, this, a2, a3, a4);
    }
}
