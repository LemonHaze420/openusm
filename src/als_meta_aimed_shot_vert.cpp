#include "als_meta_aimed_shot_vert.h"

#include "actor.h"
#include "ai_interaction_data.h"
#include "base_ai_core.h"
#include "interaction_inode.h"

#include "common.h"
#include "nal_anim.h"
#include "nal_system.h"
#include "trace.h"
#include "utility.h"

namespace als {

VALIDATE_SIZE(meta_aimed_shot_vert, 0x30);

meta_aimed_shot_vert::meta_aimed_shot_vert()
{
    if constexpr (STANDALONE_SYSTEM) {
        this->m_vtbl = CAST(m_vtbl, native_vtable());
    } else {
        this->m_vtbl = 0x00875560;
    }


    this->field_8 = tlFixedString{};
    this->field_28 = nullptr;
}

meta_aimed_shot_vert::meta_aimed_shot_vert(from_mash_in_place_constructor *a2) : als_meta_anim_base(a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        this->m_vtbl = CAST(m_vtbl, native_vtable());
    } else {
        this->m_vtbl = 0x00875560;
    }

    this->field_28 = nullptr;
}

void *meta_aimed_shot_vert::native_vtable()
{
    return g_vtbl;
}

void meta_aimed_shot_vert::_destruct_mashed_class() {}

meta_aimed_shot_vert *meta_aimed_shot_vert::_scalar_deleting_destructor(uint32_t flags)
{
    this->~meta_aimed_shot_vert();
    if ((flags & 1) != 0) {
        mash_virtual_base::operator delete(this, sizeof(*this));
    }
    return this;
}

bool meta_aimed_shot_vert::_is_subclass_of(mash::virtual_types_enum type) const
{
    return type == static_cast<mash::virtual_types_enum>(0x236) || type == static_cast<mash::virtual_types_enum>(0x23D);
}

bool meta_aimed_shot_vert::_requires_delay_create() const
{
    return true;
}

void meta_aimed_shot_vert::_delay_create(actor *owner)
{
    auto *core = owner->get_ai_core();
    if (core != nullptr) {
        auto *node = static_cast<ai::interaction_inode *>(core->get_info_node(ai::interaction_inode::default_id, true));
        this->field_28 = static_cast<nalAnimClass<nalAnyPose> *>(node->field_2C->get_anim_ptr(node->field_34, true));
    } else {
        this->field_28 = nullptr;
    }
}

void meta_aimed_shot_vert::_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("meta_aimed_shot_vert::unmash");

    als_meta_anim_base::_unmash(a2, a3);
}

int meta_aimed_shot_vert::_get_virtual_type_enum() const
{
    return 149;
}

bool meta_aimed_shot_vert::_is_anim_looping() const
{
    auto *v1 = this->field_28;
    if (v1 != nullptr) {
        return (v1->field_34 & 1) != 0;
    }

    return false;
}

bool meta_aimed_shot_vert::_is_anim_trajectory_relative() const
{
    auto *v1 = this->field_28;
    return v1 == nullptr || (v1->field_34 & 2) == 0;
}

int meta_aimed_shot_vert::_get_mash_sizeof() const
{
    return sizeof(meta_aimed_shot_vert);
}

float meta_aimed_shot_vert::_get_anim_duration() const
{
    TRACE("als::meta_aimed_shot_vert::get_anim_duration");

    auto *v1 = this->field_28;
    if (v1 != nullptr) {
        return v1->field_38;
    }

    return 1.0f;
}

nalBaseSkeleton *meta_aimed_shot_vert::_get_skeleton()
{
    auto *v1 = this->field_28;
    if (v1 != nullptr) {
        return v1->Skeleton;
    }

    return nullptr;
}

nalAnimClass<nalAnyPose>::nalInstanceClass *meta_aimed_shot_vert::_create_anim_inst(nalBaseSkeleton *a2,
                                                                                    nalAnimClass<nalAnyPose> *,
                                                                                    als::animation_logic_system *,
                                                                                    als::state_machine *)
{
    return static_cast<nalAnimClass<nalAnyPose>::nalInstanceClass *>(this->field_28->VirtualCreateInstance(a2));
}

}  // namespace als

void als_meta_aimed_shot_vert_patch()
{
    {
        FUNC_ADDRESS(address, &als::meta_aimed_shot_vert::_get_anim_duration);
        SET_JUMP(0x004518D0, address);
    }
}
