#include "als_nal_meta_anim.h"

#include "als_meta_anim_base.h"
#include "common.h"
#include "trace.h"
#include "vtbl.h"

namespace als {

VALIDATE_SIZE(als_nal_meta_anim, 0x44);

als_nal_meta_anim::als_nal_meta_anim()
{
    this->m_vtbl = 0x00880968;
    this->field_40 = nullptr;
}

bool als_nal_meta_anim::is_delay_create() const
{
    TRACE("als_nal_meta_anim::is_delay_create");

    sp_log("0x%08X", this->field_40->m_vtbl);
    bool(__fastcall * func)(const void *) = CAST(func, get_vfunc(this->field_40->m_vtbl, 0x30));
    return func(this->field_40);
}

void als_nal_meta_anim::delay_create(actor *a2)
{
    TRACE("als_nal_meta_anim::delay_create");

    void(__fastcall * func)(void *, void *, actor *) = CAST(func, get_vfunc(this->field_40->m_vtbl, 0x34));
    func(this->field_40, nullptr, a2);
    this->create(this->field_40);
}

void als_nal_meta_anim::create(als_meta_anim_base *a2)
{
    TRACE("als_nal_meta_anim::create");

    if constexpr (1) {
        this->field_40 = a2;
        this->field_34 = 0;
        if (a2->is_anim_looping()) {
            this->field_34 |= 1u;
        }

        if (this->field_40->is_anim_trajectory_relative()) {
            this->field_34 |= 2u;
        }

        this->field_38 = this->field_40->get_anim_duration();
        this->Skeleton = this->field_40->get_skeleton();
        this->field_8 = this->field_40->get_anim_name();
    } else {
        THISCALL(0x00493F20, this, a2);
    }
}

void *als_nal_meta_anim::create_anim_inst(nalBaseSkeleton *a2, animation_logic_system *a3, state_machine *a4)
{
    TRACE("als_nal_meta_anim::create_anim_inst");

    sp_log("0x%08X", this->field_40->m_vtbl);

    if constexpr (1) {
        return this->field_40->create_anim_inst(a2, this, a3, a4);
    } else {
        struct {
            char field_0[0x2C];
            void *(__fastcall *func)(void *, void *, nalBaseSkeleton *, als_nal_meta_anim *, animation_logic_system *,
                                     state_machine *);
        } *vtbl = CAST(vtbl, this->field_40->m_vtbl);
        return vtbl->func(this->field_40, nullptr, a2, this, a3, a4);
    }
}


}  // namespace als

void als_nal_meta_anim_patch()
{
    FUNC_ADDRESS(address, &als::als_nal_meta_anim::create);
    SET_JUMP(0x00493F20, address);
}
