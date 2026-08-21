#include "meta_anim_interact.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "nal_anim.h"
#include "nal_system.h"
#include "osassert.h"
#include "utility.h"
#include "trace.h"

namespace ai {
VALIDATE_SIZE(meta_anim_interact, 0x30);
VALIDATE_SIZE(meta_anim_strength_test, 0x38u);

VALIDATE_SIZE(strength_test_anim_inst, 0x24u);

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
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x00463670, this);
    }
}

meta_anim_strength_test::meta_anim_strength_test(from_mash_in_place_constructor *a2) : als_meta_anim_base(a2)
{
    this->field_28 = nullptr;
    this->field_2C = nullptr;

    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0087559C;
    }
}

void meta_anim_strength_test::initialize(mash::allocation_scope a2)
{
    this->field_28 = nullptr;
    this->field_2C = nullptr;

    if (a2 == mash::ALLOCATED) {
        this->field_30 = false;
        this->field_34 = nullptr;
    }
}

void meta_anim_strength_test::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("ai::meta_anim_strength_test::unmash");

    als::als_meta_anim_base::_unmash(a1, a3);
}

int meta_anim_strength_test::_get_virtual_type_enum() const
{
    return 150;
}

bool meta_anim_strength_test::_is_anim_looping() const
{
    auto *v1 = this->field_28;
    if (v1 != nullptr) {
        return (v1->field_34 & 1) != 0;
    }

    return false;
}

bool meta_anim_strength_test::_is_anim_trajectory_relative() const
{
    auto *v1 = this->field_28;
    return v1 == nullptr || (v1->field_34 & 2) == 0;
}

float meta_anim_strength_test::_get_anim_duration() const
{
    auto *v1 = this->field_28;
    if (v1 != nullptr) {
        return v1->field_38;
    }

    return 0.0f;
}

nalBaseSkeleton *meta_anim_strength_test::_get_skeleton()
{
    auto *v1 = this->field_28;
    if (v1 != nullptr) {
        return v1->Skeleton;
    }

    return nullptr;
}

strength_test_anim_inst *meta_anim_strength_test::_create_anim_inst(nalBaseSkeleton *a2, nalAnimClass<nalAnyPose> *,
                                                                    als::animation_logic_system *, als::state_machine *)
{
    return new ai::strength_test_anim_inst{this->field_28, this->field_2C, a2, this->field_34};
}

int meta_anim_strength_test::_get_mash_sizeof() const
{
    return sizeof(meta_anim_strength_test);
}

strength_test_anim_inst::strength_test_anim_inst(nalAnimClass<nalAnyPose> *a2, nalAnimClass<nalAnyPose> *a3,
                                                 nalBaseSkeleton *a4, ai::ai_core *a5)
    : nalAnimClass<nalAnyPose>::nalInstanceClass(a2, a4)
{
    if constexpr (1) {
        static void *g_vtbl[]{nullptr, nullptr};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0087E99C;
    }

    this->field_1C = a5;
    this->field_20 = 0.0f;
    this->field_14 = a2->VirtualCreateInstance(a4);
    this->field_18 = a3->VirtualCreateInstance(a4);
}

}  // namespace ai

namespace als {
VALIDATE_SIZE(meta_key_anim, 0x10);
VALIDATE_SIZE(als_meta_linear_blend, 0x40u);

VALIDATE_SIZE(als_meta_linear_blend::nalInstance, 0x24u);

meta_key_anim::meta_key_anim()
{
    this->initialize(mash::ALLOCATED);
}

meta_key_anim::meta_key_anim(from_mash_in_place_constructor *a2) : field_0(a2)
{
    this->initialize(mash::FROM_MASH);
}

void meta_key_anim::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
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

void meta_key_anim::lookup_nal_animation()
{
    auto *anim_by_hash = get_anim_by_hash(this->field_0, nullptr, nullptr);
    if (anim_by_hash == nullptr) {
        auto *v2 = this->field_0.to_string();
        error("Could not find animation file %s for meta anim.", v2);
    }

    this->field_4 = CAST(field_4, anim_by_hash);
}

void meta_key_anim::unmash(mash_info_struct *a1, void *)
{
    TRACE("als::meta_key_anim::unmash");

    a1->unmash_class_in_place(this->field_0, this);
}

als_meta_linear_blend::als_meta_linear_blend()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x004AB970, this);
    }
}

als_meta_linear_blend::als_meta_linear_blend(from_mash_in_place_constructor *a2) : als_meta_anim_base(a2), key_anims(a2)
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0087B954;
    }

    this->initialize(mash::FROM_MASH);
}

void als_meta_linear_blend::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::FROM_MASH) {
        for (int i = 0; i < this->key_anims.size(); ++i) {
            auto *v2 = this->key_anims.at(i);
            v2->lookup_nal_animation();
        }
    }
}

nalAnimClass<nalAnyPose> *als_meta_linear_blend::get_anim_proxy() const
{
    assert(!this->key_anims.empty());

    return this->key_anims.at(0)->field_4;
}

void als_meta_linear_blend::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("ai::als_meta_linear_blend::unmash");

    als::als_meta_anim_base::_unmash(a1, a3);
    a1->unmash_class_in_place(this->key_anims, this);
}

int als_meta_linear_blend::_get_virtual_type_enum() const
{
    return 489;
}

bool als_meta_linear_blend::_is_anim_looping() const
{
    return (this->get_anim_proxy()->field_34 & 1) != 0;
}

bool als_meta_linear_blend::_is_anim_trajectory_relative() const
{
    return (this->get_anim_proxy()->field_34 & 2) == 0;
}

float als_meta_linear_blend::_get_anim_duration() const
{
    TRACE("als::als_meta_linear_blend::get_anim_duration");

    return this->get_anim_proxy()->field_38;
}

nalBaseSkeleton *als_meta_linear_blend::_get_skeleton()
{
    return this->get_anim_proxy()->Skeleton;
}

als_meta_linear_blend::nalInstance *als_meta_linear_blend::_create_anim_inst(nalBaseSkeleton *a2,
                                                                             nalAnimClass<nalAnyPose> *,
                                                                             als::animation_logic_system *a4,
                                                                             als::state_machine *a5)
{
    return new als_meta_linear_blend::nalInstance{this, a2, a4, a5};
}

int als_meta_linear_blend::_get_mash_sizeof() const
{
    return sizeof(*this);
}

als_meta_linear_blend::nalInstance::nalInstance(als::als_meta_linear_blend *a2, nalBaseSkeleton *a3,
                                                als::animation_logic_system *a4, als::state_machine *a5)
    : nalAnimClass<nalAnyPose>::nalInstanceClass(a2->get_anim_proxy(), a3)
{
    if constexpr (0) {
    } else {
        this->m_vtbl = 0x00880AAC;
    }

    this->field_18 = a4;
    this->field_1C = a5;
    this->field_20 = a2;

    assert(!this->field_20->key_anims.empty());

    this->field_14 = new void *[a2->key_anims.size()];
    for (int i = 0; i < this->field_20->key_anims.size(); ++i) {
        auto *v11 = this->field_20->key_anims.at(i)->field_4;
        if (v11 != nullptr) {
            this->field_14[i] = v11->VirtualCreateInstance(a3);
        } else {
            this->field_14[i] = nullptr;
        }
    }
}

}  // namespace als

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
