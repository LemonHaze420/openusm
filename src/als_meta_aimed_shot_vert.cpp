#include "als_meta_aimed_shot_vert.h"

#include "common.h"
#include "nal_anim.h"
#include "nal_system.h"
#include "trace.h"
#include "utility.h"

namespace als {

VALIDATE_SIZE(meta_aimed_shot_vert, 0x30);

meta_aimed_shot_vert::meta_aimed_shot_vert() {
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00875560;
    }
}

meta_aimed_shot_vert::meta_aimed_shot_vert(from_mash_in_place_constructor *a2) : als_meta_anim_base(a2)  {
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00875560;
    }

    this->field_28 = nullptr;
}

void meta_aimed_shot_vert::_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("meta_aimed_shot_vert::unmash");

    als_meta_anim_base::_unmash(a2, a3);
}

int meta_aimed_shot_vert::_get_virtual_type_enum() const {
    return 149;
}

bool meta_aimed_shot_vert::_is_anim_looping() const
{
    auto *v1 = this->field_28;
    if ( v1 != nullptr ) {
        return (v1->field_34 & 1) != 0;
    }

    return false;
}

bool meta_aimed_shot_vert::_is_anim_trajectory_relative() const
{
    auto *v1 = this->field_28;
    return v1 == nullptr || (v1->field_34 & 2) == 0;
}

int meta_aimed_shot_vert::_get_mash_sizeof() const {
    return sizeof(meta_aimed_shot_vert);
}

float meta_aimed_shot_vert::_get_anim_duration() const
{
    TRACE("als::meta_aimed_shot_vert::get_anim_duration");

    auto *v1 = this->field_28;
    if ( v1 != nullptr ) {
        return v1->field_38;
    }

    return 1.0f;
}

nalBaseSkeleton * meta_aimed_shot_vert::_get_skeleton()
{
    auto *v1 = this->field_28;
    if ( v1 != nullptr ) {
        return v1->Skeleton;
    }

    return nullptr;
}

nalAnimClass<nalAnyPose>::nalInstanceClass * meta_aimed_shot_vert::_create_anim_inst(
        nalBaseSkeleton *a2,
        nalAnimClass<nalAnyPose> *,
        als::animation_logic_system *,
        als::state_machine *)
{
    return static_cast<nalAnimClass<nalAnyPose>::nalInstanceClass *>(this->field_28->VirtualCreateInstance(a2));
}

}

void als_meta_aimed_shot_vert_patch()
{
    {
        FUNC_ADDRESS(address, &als::meta_aimed_shot_vert::_get_anim_duration);
        SET_JUMP(0x004518D0, address);
    }
}


