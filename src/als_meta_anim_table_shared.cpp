#include "als_meta_anim_table_shared.h"

#include "als_meta_anim_base.h"
#include "als_nal_meta_anim.h"
#include "common.h"
#include "exe_allocator.h"
#include "func_wrapper.h"
#include "game.h"
#include "trace.h"
#include <cstring>

namespace als {

    VALIDATE_SIZE(als_meta_anim_table_shared, 0x18);

    als_meta_anim_table_shared::als_meta_anim_table_shared(
        from_mash_in_place_constructor *a2)
    {
#ifdef OPENUSM_XBPACK_V10
        this->field_0 = {a2};
        for (auto i = 0; i < this->field_0.size(); ++i) {
            auto *&anim = this->field_0.m_data[i];
            if (anim != nullptr) {
                anim = static_cast<als_meta_anim_base *>(
                    mash_virtual_base::construct_class_helper(anim));
            }
        }
#else
        this->field_0 = {a2};
#endif
        this->initialize(mash::FROM_MASH);
    }

    void als_meta_anim_table_shared::initialize(mash::allocation_scope a2)
    {
        TRACE("als::als_meta_anim_table_shared::initialize");

        if constexpr (1) {
            if ( a2 == mash::FROM_MASH ) {
                auto count = this->field_0.size();
                auto *mem = exe_allocator<als_nal_meta_anim> {}.allocate(count);
                this->field_14 = static_cast<als_nal_meta_anim *>(mem);

                for ( auto i = 0; i < count; ++i )
                {
                    new (&this->field_14[i]) als_nal_meta_anim;
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

    als_nal_meta_anim *als_meta_anim_table_shared::get_nal_meta_anim(
            string_hash a2,
            actor *a3) const {
        TRACE("als_meta_anim_table_shared::get_nal_meta_anim");

        const auto find_anim = [this](uint32_t hash)
            -> als_nal_meta_anim *
        {
            auto *begin = this->field_14;
            auto *end = begin + this->field_0.size();
            auto *it = std::find_if(
                begin,
                end,
                [hash](const auto &anim)
                {
                    return anim.field_8.m_hash == hash;
                });
            return it == end ? nullptr : it;
        };

        auto *anim = find_anim(a2.source_hash_code);
        string_hash remapped;
        if ( anim == nullptr
            && remap_venom_animation_name(
                a2,
                &remapped) )
        {
            anim = find_anim(remapped.source_hash_code);
        }

        if ( anim != nullptr && anim->is_delay_create() )
            anim->delay_create(a3);
        return anim;
    }
}

void venom_als_remap_patch()
{
    FUNC_ADDRESS(
        address,
        &als::als_meta_anim_table_shared::get_nal_meta_anim);
    REDIRECT(0x0049B92E, address);
}

void als_meta_anim_table_shared_patch()
{
    FUNC_ADDRESS(address, &als::als_meta_anim_table_shared::initialize);
    REDIRECT(0x004A8FA3, address);

    {
        FUNC_ADDRESS(address, &als::als_meta_anim_table_shared::get_nal_meta_anim);
        REDIRECT(0x0049B92E, address);
    }
}
