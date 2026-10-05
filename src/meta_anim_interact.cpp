#include "meta_anim_interact.h"
#include "animation_controller.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "nal_anim.h"
#include "nal_system.h"
#include "osassert.h"
#include "utility.h"
#include "trace.h"
#include "memory.h"
#include "nal_skeleton.h"
#include "state_machine.h"
#include <cmath>
#include "ai_interaction_data.h"
#include "base_ai_core.h"
#include "interaction_inode.h"
#include "strength_test_inode.h"

namespace ai {
VALIDATE_SIZE(meta_anim_interact, 0x30);
VALIDATE_SIZE(meta_anim_strength_test, 0x38u);

VALIDATE_SIZE(strength_test_anim_inst, 0x24u);

namespace {
void __fastcall strength_destruct_mashed(meta_anim_strength_test *, void *)
{

}

void *__fastcall strength_scalar_delete(meta_anim_strength_test *self, void *, unsigned int flags)
{
    return self->scalar_delete(flags);
}

bool __fastcall strength_is_subclass(const meta_anim_strength_test *self, void *, mash::virtual_types_enum type)
{
    return self->_is_subclass_of(type);
}


bool __fastcall strength_needs_delay_create(const meta_anim_strength_test *, void *)
{
    return true;
}

void __fastcall strength_delay_create(meta_anim_strength_test *self, void *, actor *owner)
{
    self->delay_create(owner);
}

void *__fastcall strength_instance_delete(strength_test_anim_inst *self, void *, unsigned int flags)
{
    return self->scalar_delete(flags);
}

void __fastcall strength_instance_sample(strength_test_anim_inst *self, void *, Float t, Float t_prev,
                                         nalBasePose &pose, const nalBasePose &default_pose)
{
    self->sample_pose(t, t_prev, pose, default_pose);
}
}

void *meta_anim_strength_test::native_vtable()
{

    static void *table[] = {
        reinterpret_cast<void *>(&strength_destruct_mashed),
        func_address(&meta_anim_strength_test::_unmash),
        reinterpret_cast<void *>(&strength_scalar_delete),
        func_address(&meta_anim_strength_test::_get_virtual_type_enum),
        reinterpret_cast<void *>(&strength_is_subclass),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        func_address(&als::als_meta_anim_base::_get_anim_name),
        func_address(&meta_anim_strength_test::_is_anim_looping),
        func_address(&meta_anim_strength_test::_is_anim_trajectory_relative),
        func_address(&meta_anim_strength_test::_get_anim_duration),
        func_address(&meta_anim_strength_test::_get_skeleton),
        func_address(&meta_anim_strength_test::_create_anim_inst),
        reinterpret_cast<void *>(&strength_needs_delay_create),
        reinterpret_cast<void *>(&strength_delay_create),
        func_address(&meta_anim_strength_test::_get_mash_sizeof),
    };
    return table;
}

bool meta_anim_strength_test::_is_subclass_of(mash::virtual_types_enum type) const
{
    return type == 566 || type == 573;
}

void *meta_anim_strength_test::scalar_delete(unsigned int flags)
{

    this->~meta_anim_strength_test();
    if (flags & 1) {
        mash_virtual_base::operator delete(this, sizeof(*this));
    }
    return this;
}

void meta_anim_strength_test::delay_create(actor *owner)
{

    auto *core = owner->get_ai_core();
    if (core != nullptr) {
        auto *node = static_cast<interaction_inode *>(core->get_info_node(interaction_inode::default_id, true));
        if (field_30) {
            field_28 = static_cast<nalAnimClass<nalAnyPose> *>(
                node->field_2C->get_anim_ptr(static_cast<enum_anim_key::key_enum>(2), false));
        } else {
            field_28 = static_cast<nalAnimClass<nalAnyPose> *>(node->field_2C->get_anim_ptr(node->field_34, true));
        }
        field_2C = static_cast<nalAnimClass<nalAnyPose> *>(
            node->field_2C->get_anim_ptr(static_cast<enum_anim_key::key_enum>(3), !field_30));
        field_34 = core;
    } else {
        field_28 = nullptr;
        field_2C = nullptr;
    }
}

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
    this->initialize(mash::ALLOCATED);
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    this->m_vtbl = 0x0087559C;
#endif
}

meta_anim_strength_test::meta_anim_strength_test(from_mash_in_place_constructor *a2) : als_meta_anim_base(a2)
{
    this->initialize(mash::FROM_MASH);
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    this->m_vtbl = 0x0087559C;
#endif
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

    return 1.0f;
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
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    this->m_vtbl = 0x0087E99C;
#endif

    this->field_1C = a5;
    this->field_20 = 0.0f;
    this->field_14 = a2->VirtualCreateInstance(a4);
    this->field_18 = a3->VirtualCreateInstance(a4);
}

void *strength_test_anim_inst::native_vtable()
{

    static void *table[] = {
        reinterpret_cast<void *>(&strength_instance_delete),
        reinterpret_cast<void *>(&strength_instance_sample),
    };
    return table;
}

strength_test_anim_inst::~strength_test_anim_inst()
{

    using destroy_callback = void *(__fastcall *)(nalInstanceClass *, void *, unsigned int);
    if (field_14 != nullptr) {
        auto destroy = reinterpret_cast<destroy_callback>(get_vfunc(field_14->m_vtbl, 0));
        destroy(field_14, nullptr, 1);
    }
    if (field_18 != nullptr) {
        auto destroy = reinterpret_cast<destroy_callback>(get_vfunc(field_18->m_vtbl, 0));
        destroy(field_18, nullptr, 1);
    }

}

void *strength_test_anim_inst::scalar_delete(unsigned int flags)
{
    this->~strength_test_anim_inst();
    if (flags & 1) {
        nalAnimClass<nalAnyPose>::nalInstanceClass::operator delete(this);
    }
    return this;
}

double strength_test_anim_inst::get_curr_strength() const
{

    auto *node = static_cast<strength_test_inode *>(field_1C->get_info_node(strength_test_inode::default_id, true));
    return node->advanced ? node->advanced_strength : node->strength;
}

void strength_test_anim_inst::sample_pose(Float, Float, nalBasePose &pose, const nalBasePose &default_pose)
{

    double strength = get_curr_strength();
    float current_strength = static_cast<float>(strength);
    if (strength <= field_20) {
        field_18->VirtualGetPose(1.0f - current_strength, 1.0f - field_20, pose, default_pose);
    } else {
        field_14->VirtualGetPose(current_strength, field_20, pose, default_pose);
    }
    field_20 = current_strength;
}

}  // namespace ai

namespace als {
VALIDATE_SIZE(meta_key_anim, 0x10);
VALIDATE_SIZE(als_meta_linear_blend, 0x40u);

VALIDATE_SIZE(als_meta_linear_blend::nalInstance, 0x24u);

namespace {
using linear_instance = als_meta_linear_blend::nalInstance;
using child_instance = nalAnimClass<nalAnyPose>::nalInstanceClass;

void __fastcall linear_destruct_mashed(als_meta_linear_blend *self, void *)
{
    self->_destruct_mashed_class();
}

void *__fastcall linear_scalar_delete(als_meta_linear_blend *self, void *, unsigned int flags)
{
    return self->scalar_delete(flags);
}

bool __fastcall linear_is_subclass(const als_meta_linear_blend *self, void *, mash::virtual_types_enum type)
{
    return self->_is_subclass_of(type);
}

bool __fastcall linear_has_no_extra_mash_data(const als_meta_linear_blend *, void *)
{
    return false;
}

void __fastcall linear_release_extra_mash_data(als_meta_linear_blend *, void *, void *)
{

}

void *__fastcall linear_instance_delete(linear_instance *self, void *, unsigned int flags)
{
    return self->scalar_delete(flags);
}

void __fastcall linear_instance_sample(linear_instance *self, void *, Float t, Float t_prev,
                                       nalBasePose &pose, const nalBasePose &default_pose)
{
    self->sample_pose(t, t_prev, pose, default_pose);
}

void __fastcall linear_instance_blend(linear_instance *self, void *, Float t, Float t_prev,
                                      nalAnyPose &pose, const nalAnyPose &default_pose, Float weight,
                                      child_instance *lower, child_instance *upper)
{
    self->blend_poses(t, t_prev, pose, default_pose, weight, lower, upper);
}

struct linear_blend_pose : nalAnyPose {
    using nalAnyPose::nalAnyPose;

    ~linear_blend_pose()
    {
        const_cast<nalBaseSkeleton *>(GetSkeleton())->VirtualDestroyPose(field_0);
    }
};
}

void *als_meta_linear_blend::native_vtable()
{

    static void *table[] = {
        reinterpret_cast<void *>(&linear_destruct_mashed),
        func_address(&als_meta_linear_blend::_unmash),
        reinterpret_cast<void *>(&linear_scalar_delete),
        func_address(&als_meta_linear_blend::_get_virtual_type_enum),
        reinterpret_cast<void *>(&linear_is_subclass),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        func_address(&als_meta_anim_base::_get_anim_name),
        func_address(&als_meta_linear_blend::_is_anim_looping),
        func_address(&als_meta_linear_blend::_is_anim_trajectory_relative),
        func_address(&als_meta_linear_blend::_get_anim_duration),
        func_address(&als_meta_linear_blend::_get_skeleton),
        func_address(&als_meta_linear_blend::_create_anim_inst),
        reinterpret_cast<void *>(&linear_has_no_extra_mash_data),
        reinterpret_cast<void *>(&linear_release_extra_mash_data),
        func_address(&als_meta_linear_blend::_get_mash_sizeof),
    };
    return table;
}

bool als_meta_linear_blend::_is_subclass_of(mash::virtual_types_enum type) const
{

    return type == 566 || type == 573;
}

void als_meta_linear_blend::clear_key_anims()
{


    if (key_anims.field_10) {
        for (int i = 0; i < key_anims.size(); ++i) {
            auto *key = key_anims.m_data[i];
            if (key_anims.is_pointer_in_mash_image(key)) {
                key->~meta_key_anim();
            } else {
                delete key;
            }
            key_anims.m_data[i] = nullptr;
        }
    }
    if (!key_anims.is_pointer_in_mash_image(key_anims.m_data)) {
        mem_dealloc(key_anims.m_data, sizeof(meta_key_anim *) * key_anims.m_max_size);
    }
    key_anims.m_data = nullptr;
    key_anims.m_max_size = 0;
    key_anims.mContainer_base::clear();
}

als_meta_linear_blend::~als_meta_linear_blend()
{
    clear_key_anims();
}

void als_meta_linear_blend::_destruct_mashed_class()
{
    clear_key_anims();
    key_anims.mContainer_base::destruct_mashed_class();
}

void *als_meta_linear_blend::scalar_delete(unsigned int flags)
{
    this->~als_meta_linear_blend();
    if (flags & 1) {
        mash_virtual_base::operator delete(this, sizeof(*this));
    }
    return this;
}

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
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

als_meta_linear_blend::als_meta_linear_blend(from_mash_in_place_constructor *a2) : als_meta_anim_base(a2), key_anims(a2)
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());

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
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());

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

void *als_meta_linear_blend::nalInstance::native_vtable()
{

    static void *table[] = {
        reinterpret_cast<void *>(&linear_instance_delete),
        reinterpret_cast<void *>(&linear_instance_sample),
        reinterpret_cast<void *>(&linear_instance_blend),
    };
    return table;
}

als_meta_linear_blend::nalInstance::~nalInstance()
{

    for (int i = 0; i < field_20->key_anims.size(); ++i) {
        auto *child = static_cast<child_instance *>(field_14[i]);
        if (child != nullptr) {
            using destroy_callback = void *(__fastcall *)(child_instance *, void *, unsigned int);
            auto destroy = reinterpret_cast<destroy_callback>(get_vfunc(child->m_vtbl, 0));
            destroy(child, nullptr, 1);
        }
        field_14[i] = nullptr;
    }
    delete[] field_14;

}

void *als_meta_linear_blend::nalInstance::scalar_delete(unsigned int flags)
{
    this->~nalInstance();
    if (flags & 1) {
        nalAnimClass<nalAnyPose>::nalInstanceClass::operator delete(this);
    }
    return this;
}

void als_meta_linear_blend::nalInstance::sample_pose(Float t, Float t_prev, nalBasePose &pose,
                                                    const nalBasePose &default_pose)
{

    linear_blend_pose reference(default_pose, true);
    float parameter = field_1C->get_param(field_18, 18);
    if (field_1C->find_external_param(static_cast<external_parameter_types>(18))) {
        if (parameter < 0.0f) {
            parameter = 0.0f;
        } else if (parameter > 1.0f) {
            parameter = 1.0f;
        }
    } else {
        parameter = 0.5f;
    }

    const float position = (field_20->key_anims.size() - 1) * parameter;
    const float lower_index = std::floor(position);
    const float upper_index = std::ceil(position);
    float weight = position - lower_index;
    if (weight < 0.0f) {
        weight = 0.0f;
    } else if (weight > 1.0f) {
        weight = 1.0f;
    }

    auto *lower = static_cast<child_instance *>(field_14[static_cast<unsigned int>(lower_index)]);
    auto *upper = static_cast<child_instance *>(field_14[static_cast<unsigned int>(upper_index)]);
    linear_blend_pose result(field_C);
    using blend_callback = void(__fastcall *)(nalInstance *, void *, Float, Float, nalAnyPose &,
                                              const nalAnyPose &, Float, child_instance *, child_instance *);
    auto blend = reinterpret_cast<blend_callback>(get_vfunc(m_vtbl, 8));
    blend(this, nullptr, t, t_prev, result, reference, Float(weight), lower, upper);
    field_C->VirtualCopyPose(pose, *result.field_0);
}

void als_meta_linear_blend::nalInstance::blend_poses(Float t, Float t_prev, nalAnyPose &pose,
                                                    const nalAnyPose &default_pose, Float weight,
                                                    child_instance *lower, child_instance *upper)
{

    if (lower == upper) {
        lower->VirtualGetPose(t, t_prev, *pose.field_0, *default_pose.field_0);
        return;
    }
    linear_blend_pose lower_pose(field_C);
    linear_blend_pose upper_pose(field_C);
    lower->VirtualGetPose(t, t_prev, *lower_pose.field_0, *default_pose.field_0);
    upper->VirtualGetPose(t, t_prev, *upper_pose.field_0, *default_pose.field_0);
    sub_826140(pose, weight, lower_pose, upper_pose);
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
