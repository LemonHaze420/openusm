#include "als_meta_anim_base.h"

#include "common.h"
#include "func_wrapper.h"
#include "string_hash.h"
#include "trace.h"
#include "vtbl.h"
#include "utility.h"

namespace als
{
    VALIDATE_SIZE(als_meta_anim_base, 0x28);

    const tlFixedString &als_meta_anim_base::get_anim_name() const
    {
        sp_log("%s %s", this->field_8.to_string(), string_hash {int(this->field_8.m_hash)}.to_string());
        return this->field_8;
    }

    bool als_meta_anim_base::is_anim_looping() const
    {
        bool (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
        return func(this);
    }

    bool als_meta_anim_base::is_anim_trajectory_relative() const
    {
        bool (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x20));
        return func(this);
    }

    float als_meta_anim_base::get_anim_duration() const
    {
        float (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x24));
        return func(this);
    }

    nalBaseSkeleton *als_meta_anim_base::get_skeleton()
    {
        nalBaseSkeleton * (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x28));
        return func(this);
    }

    void * als_meta_anim_base::create_anim_inst(
            nalBaseSkeleton *a1,
            nalAnimClass<nalAnyPose> *a2,
            als::animation_logic_system *a3,
            als::state_machine *a4)
    {
        void * (__fastcall *func)(void *, void *edx, nalBaseSkeleton *, nalAnimClass<nalAnyPose> *, als::animation_logic_system *, als::state_machine *) = CAST(func, get_vfunc(m_vtbl, 0x2C));
        return func(this, nullptr, a1, a2, a3, a4);
    }

    int als_meta_anim_base::get_mash_sizeof() const
    {
        int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x38));
        return func(this);
    }

    void als_meta_anim_base::_unmash(mash_info_struct *, void *)
    {
        TRACE("als::als_meta_anim_base::unmash");
        ;
    }
}

void als_meta_anim_base_patch()
{
    {
        FUNC_ADDRESS(address, &als::als_meta_anim_base::_unmash);
        set_vfunc(0x0087545C, address);
    }
}

