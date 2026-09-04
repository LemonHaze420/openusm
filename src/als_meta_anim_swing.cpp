#include "als_meta_anim_swing.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "meta_anim_interact.h"
#include "nal_system.h"
#include "nal_skeleton.h"
#include "tl_system.h"
#include "trace.h"
#include "utility.h"

namespace als {

VALIDATE_SIZE(als_meta_anim_swing, 0x70);

VALIDATE_SIZE(als_meta_anim_swing::nalInstance, 0x12Cu);

als_meta_anim_swing::als_meta_anim_swing()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);

        this->initialize(mash::ALLOCATED);
    } else {
        THISCALL(0x004AB870, this);
    }
}

als_meta_anim_swing::als_meta_anim_swing(from_mash_in_place_constructor *a2) : field_28(a2)
{
    if constexpr (1) {
        if constexpr (1) {
            this->m_vtbl = CAST(m_vtbl, &g_vtbl);
        } else {
            this->m_vtbl = 0x0087B918;
        }

        this->initialize(mash::FROM_MASH);
    } else {
        THISCALL(0x004AB8F0, this, a2);
    }
}

void als_meta_anim_swing::initialize(mash::allocation_scope a2)
{
    if constexpr (1) {
        if (a2 == mash::FROM_MASH) {
            for (int i = 0; i < 12; ++i) {
                auto *v10 = this->field_28.at(i);
                v10->lookup_nal_animation();
            }
        } else {
            for (int i = 0; i < 12; ++i) {
                auto *mem = mem_alloc(sizeof(meta_key_anim));
                auto *v7 = new (mem) meta_key_anim{};

                this->field_28.push_back(v7);
                this->field_3C[i] = nullptr;
            }
        }
    } else {
        THISCALL(0x004A0350, this, a2);
    }
}

nalAnimClass<nalAnyPose> *als_meta_anim_swing::get_anim_proxy() const
{
    assert(!this->field_28.empty());

    return this->field_28.at(0)->field_4;
}

void als_meta_anim_swing::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("als::als_meta_anim_swing::unmash");

    als_meta_anim_base::_unmash(a1, a3);

    a1->unmash_class_in_place(this->field_28, this);
}

int als_meta_anim_swing::_get_virtual_type_enum() const
{
    return 488;
}

bool als_meta_anim_swing::_is_anim_looping() const
{
    return (this->get_anim_proxy()->field_34 & 1) != 0;
}

bool als_meta_anim_swing::_is_anim_trajectory_relative() const
{
    return (this->get_anim_proxy()->field_34 & 2) == 0;
}

float als_meta_anim_swing::_get_anim_duration() const
{
    TRACE("als::als_meta_anim_swing::get_anim_duration");
    return 5.0;
}

nalBaseSkeleton *als_meta_anim_swing::_get_skeleton()
{
    return this->get_anim_proxy()->Skeleton;
}

als_meta_anim_swing::nalInstance *als_meta_anim_swing::_create_anim_inst(nalBaseSkeleton *a2,
                                                                         nalAnimClass<nalAnyPose> *,
                                                                         als::animation_logic_system *a4,
                                                                         als::state_machine *a5)
{
    auto *v5 = a2;
    auto v7 = a2->VirtualGetDefaultPose();
    nalAnyPose v20{*v7, true};

    for (int i = 0; i < 12; ++i) {
        auto *v10 = this->field_28.at(i)->field_4;
        if (v10 != nullptr && this->field_3C[i] == nullptr) {
            auto *v11 = v10->CreateInstance(v5);
            this->field_3C[i] = new nalAnyPose{v20, true};

            auto *v15 = v5->VirtualGetDefaultPose();
            nalAnyPose v12{*v15, true};

            *(this->field_3C[i]) = v12;

            GetPose(v11, Float(0.0f), Float(0.0f), *this->field_3C[i], v20);

            if (v11 != nullptr) {
                delete v11;
            }
        }
    }

    auto *v17 = new nalInstance{this, v5, a4, a5};
    return v17;
}

int als_meta_anim_swing::_get_mash_sizeof() const
{
    return sizeof(*this);
}

als_meta_anim_swing::nalInstance::nalInstance(als::als_meta_anim_swing *a2, nalBaseSkeleton *a3,
                                              als::animation_logic_system *a4, als::state_machine *a5)
    : nalAnimClass<nalAnyPose>::nalInstanceClass(a2->get_anim_proxy(), a3)
{
    this->m_vtbl = 0x00880C64;

    this->field_14 = a4;
    this->field_1C = a2;
    this->field_18 = a5;
}

}  // namespace als

void als_meta_anim_swing_patch()
{
    {
        FUNC_ADDRESS(address, &als::als_meta_anim_swing::initialize);
        REDIRECT(0x004AB949, address);
    }

    {
        FUNC_ADDRESS(address, &als::als_meta_anim_swing::_get_anim_duration);
        SET_JUMP(0x00494210, address);
    }

    {
        FUNC_ADDRESS(address, &als::als_meta_anim_swing::_unmash);
        set_vfunc(0x0087B91C, address);
    }
}
