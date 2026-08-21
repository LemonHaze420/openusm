#include "als_meta_anim_table_shared.h"

#include "als_meta_anim_base.h"
#include "als_nal_meta_anim.h"
#include "common.h"
#include "func_wrapper.h"
#include "trace.h"

namespace als {

VALIDATE_SIZE(als_meta_anim_table_shared, 0x18);

als_meta_anim_table_shared::als_meta_anim_table_shared(from_mash_in_place_constructor *a2) : field_0(a2)
{
    this->initialize(mash::FROM_MASH);
}

void als_meta_anim_table_shared::initialize(mash::allocation_scope a2)
{
    TRACE("als::als_meta_anim_table_shared::initialize");

    if constexpr (1) {
        if (a2 == mash::FROM_MASH) {
            auto count = this->field_0.size();
            auto *mem = operator new(sizeof(als_nal_meta_anim) * count);
            this->field_14 = new (mem) als_nal_meta_anim[count];

            for (auto i = 0; i < count; ++i) {
                auto *anim_ptr = this->field_0.at(i);
                this->field_14[i].create(anim_ptr);
            }
        }
    } else {
        THISCALL(0x00499200, this, a2);
    }
}

void als_meta_anim_table_shared::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, this);
}

als_nal_meta_anim *als_meta_anim_table_shared::get_nal_meta_anim(string_hash a2, actor *a3) const
{
    TRACE("als_meta_anim_table_shared::get_nal_meta_anim");

    if constexpr (1) {
#if 0
            for ( auto i = 0; i < this->field_0.size(); ++i )
            {
                auto &v5 = this->field_14[i].field_8;
                if ( a2.source_hash_code == v5.m_hash )
                {
                    if ( this->field_14[i].is_delay_create() ) {
                        this->field_14[i].delay_create(a3);
                    }

                    auto *v7 = &this->field_14[i];
                    return v7;
                }

                ++i;
            }

            return nullptr;
#else
        auto begin = this->field_14;
        auto end = begin + this->field_0.size();
        auto it = std::find_if(begin, end, [a2, a3](auto &anim_ptr) {
            auto &v5 = anim_ptr.field_8;
            return (a2.source_hash_code == v5.m_hash);
        });

        if (it != end) {
            sp_log("vtbl = 0x%08X", it->m_vtbl);
            if (it->is_delay_create()) {
                it->delay_create(a3);
            }

            return it;
        }

        return nullptr;
#endif

    } else {
        return (als_nal_meta_anim *)THISCALL(0x004992B0, this, a2, a3);
    }
}
}  // namespace als

void als_meta_anim_table_shared_patch()
{
    FUNC_ADDRESS(address, &als::als_meta_anim_table_shared::initialize);
    REDIRECT(0x004A8FA3, address);

    {
        FUNC_ADDRESS(address, &als::als_meta_anim_table_shared::get_nal_meta_anim);
        REDIRECT(0x0049B92E, address);
    }
}
