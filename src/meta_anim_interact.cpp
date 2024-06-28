#include "meta_anim_interact.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "nal_anim.h"
#include "nal_system.h"
#include "utility.h"
#include "trace.h"

namespace ai
{
    VALIDATE_SIZE(meta_anim_interact, 0x30);
    VALIDATE_SIZE(meta_anim_strength_test, 0x38u);

    meta_anim_interact::meta_anim_interact()
    {
        THISCALL(0x00463620, this);
    }

    void meta_anim_interact::_unmash(mash_info_struct *a1, void *a3)
    {
        TRACE("ai::meta_anim_interact::unmash");

        als::als_meta_anim_base::_unmash(a1, a3);
    }

    meta_anim_strength_test::meta_anim_strength_test()
    {
        THISCALL(0x00463670, this);
    }

    void meta_anim_strength_test::_unmash(mash_info_struct *a1, void *a3)
    {
        TRACE("ai::meta_anim_strength_test::unmash");

        als::als_meta_anim_base::_unmash(a1, a3);
    }

} // ai

namespace als
{
    VALIDATE_SIZE(meta_key_anim, 0x10);
    VALIDATE_SIZE(als_meta_linear_blend, 0x40u);

    meta_key_anim::meta_key_anim(from_mash_in_place_constructor *a2) : field_0(a2)
    {
        this->initialize(mash::FROM_MASH);
    }

    void meta_key_anim::initialize(mash::allocation_scope a2)
    {
        if ( a2 == mash::ALLOCATED ) {
            this->clear();
        }
    }

    void meta_key_anim::clear()
    {
        this->field_0 = {0};
        this->field_4 = nullptr;
        this->field_8 = 0;
        this->field_C = 0;
    }

    void meta_key_anim::unmash(mash_info_struct *a1, void *)
    {
        TRACE("als::meta_key_anim::unmash");

        a1->unmash_class_in_place(this->field_0, this);
    }

    als_meta_linear_blend::als_meta_linear_blend()
    {
        THISCALL(0x004AB970, this);
    }

    void als_meta_linear_blend::_unmash(mash_info_struct *a1, void *a3)
    {
        TRACE("ai::als_meta_linear_blend::unmash");

        als::als_meta_anim_base::_unmash(a1, a3);
        a1->unmash_class_in_place(this->field_28, this);
    }

    float als_meta_linear_blend::get_anim_duration() const
    {
        TRACE("als::als_meta_linear_blend::get_anim_duration");

        return this->field_28.m_data[0]->field_4->field_38;
    }

} // als

void meta_anim_interact_patch()
{
    {
        FUNC_ADDRESS(address, &ai::meta_anim_interact::_unmash);
        set_vfunc(0x00875564, address);
    }

    {
        FUNC_ADDRESS(address, &ai::meta_anim_strength_test::_unmash);
        set_vfunc(0x008755A0, address);
    }

    {
        FUNC_ADDRESS(address, &als::als_meta_linear_blend::_unmash);
        set_vfunc(0x0087B958, address);
    }

    {
        FUNC_ADDRESS(address, &als::als_meta_linear_blend::get_anim_duration);
        set_vfunc(0x0087B978, address);
    }

}
