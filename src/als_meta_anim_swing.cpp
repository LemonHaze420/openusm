#include "als_meta_anim_swing.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "meta_anim_interact.h"
#include "nal_system.h"
#include "nal_skeleton.h"
#include "tl_system.h"
#include "state_machine.h"
#include "trace.h"
#include "utility.h"

#include <cmath>

namespace {

struct scoped_swing_pose {
    nalBasePose *pose;

    ~scoped_swing_pose()
    {
        pose->GetSkeleton()->VirtualDestroyPose(pose);
    }
};

void destroy_swing_anim_instance(nalAnimClass<nalAnyPose>::nalInstanceClass *instance)
{
    void *(__fastcall * destroy)(void *, void *, uint32_t) = CAST(destroy, get_vfunc(instance->m_vtbl, 0));
    destroy(instance, nullptr, 1);
}

}  // namespace

namespace als {

VALIDATE_SIZE(als_meta_anim_swing, 0x70);

VALIDATE_SIZE(als_meta_anim_swing::nalInstance, 0x12Cu);

als_meta_anim_swing::als_meta_anim_swing()
{
    this->m_vtbl = CAST(m_vtbl, native_vtable());
    this->initialize(mash::ALLOCATED);
}

als_meta_anim_swing::als_meta_anim_swing(from_mash_in_place_constructor *a2) : als_meta_anim_base(a2), field_28(a2)
{
    this->m_vtbl = CAST(m_vtbl, native_vtable());
    this->initialize(mash::FROM_MASH);
}

als_meta_anim_swing::~als_meta_anim_swing()
{
    this->finalize();
}

void *als_meta_anim_swing::native_vtable()
{
    return g_vtbl;
}

void als_meta_anim_swing::finalize()
{
    for (auto &pose : this->field_3C) {
        if (pose != nullptr) {
            const_cast<nalBaseSkeleton *>(pose->GetSkeleton())->VirtualDestroyPose(pose->field_0);
            delete pose;
        }
        pose = nullptr;
    }


    if (this->field_28.field_10) {
        for (int i = 0; i < this->field_28.m_size; ++i) {
            auto *&key = this->field_28.m_data[i];
            if (!this->field_28.is_pointer_in_mash_image(key)) {
                delete key;
            }
            key = nullptr;
        }
    }
    if (!this->field_28.is_pointer_in_mash_image(this->field_28.m_data)) {
        mem_dealloc(this->field_28.m_data, 4 * this->field_28.m_max_size);
    }
    this->field_28.m_data = nullptr;
    this->field_28.m_max_size = 0;
    this->field_28.mContainer_base::clear();
}

void als_meta_anim_swing::_destruct_mashed_class()
{
    this->finalize();
    this->field_28.mContainer_base::destruct_mashed_class();
}

als_meta_anim_swing *als_meta_anim_swing::_scalar_deleting_destructor(uint32_t flags)
{
    this->~als_meta_anim_swing();
    if ((flags & 1) != 0) {
        mash_virtual_base::operator delete(this, sizeof(*this));
    }
    return this;
}

bool als_meta_anim_swing::_is_subclass_of(mash::virtual_types_enum type) const
{
    return type == static_cast<mash::virtual_types_enum>(0x236) || type == static_cast<mash::virtual_types_enum>(0x23D);
}

bool als_meta_anim_swing::_is_pivot_valid() const
{
    return false;
}

void als_meta_anim_swing::_release(void *) {}

void als_meta_anim_swing::initialize(mash::allocation_scope scope)
{
    if (scope == mash::FROM_MASH) {
        for (int i = 0; i < 12; ++i) {
            this->field_28.at(i)->lookup_nal_animation();
        }
    } else {
        for (int i = 0; i < 12; ++i) {
            this->field_28.push_back(new meta_key_anim{});
            this->field_3C[i] = nullptr;
        }
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
    auto *default_pose = a2->VirtualGetDefaultPose();
    nalAnyPose v20{*default_pose, true};
    scoped_swing_pose default_copy{v20.field_0};

    for (int i = 0; i < 12; ++i) {
        auto *anim = this->field_28.at(i)->field_4;
        if (anim != nullptr && this->field_3C[i] == nullptr) {
            auto *instance = anim->CreateInstance(a2);
            this->field_3C[i] = new nalAnyPose{v20, true};

            nalAnyPose pose{*a2->VirtualGetDefaultPose(), true};
            scoped_swing_pose pose_copy{pose.field_0};
            *this->field_3C[i] = pose;
            GetPose(instance, Float(0.0f), Float(0.0f), *this->field_3C[i], v20);
            destroy_swing_anim_instance(instance);
        }
    }
    return new nalInstance{this, a2, a4, a5};
}

int als_meta_anim_swing::_get_mash_sizeof() const
{
    return sizeof(*this);
}

als_meta_anim_swing::nalInstance::nalInstance(als::als_meta_anim_swing *a2, nalBaseSkeleton *a3,
                                              als::animation_logic_system *a4, als::state_machine *a5)
    : nalAnimClass<nalAnyPose>::nalInstanceClass(a2->get_anim_proxy(), a3)
{
    this->m_vtbl = CAST(m_vtbl, &g_vtbl);

    this->field_14 = a4;
    this->field_1C = a2;
    this->field_18 = a5;
}

als_meta_anim_swing::nalInstance *als_meta_anim_swing::nalInstance::_scalar_deleting_destructor(uint32_t flags)
{
    --this->field_10->InstanceCount;
    if ((flags & 1) != 0) {
        nalInstanceClass::operator delete(this);
    }
    return this;
}

void als_meta_anim_swing::nalInstance::_get_pose(Float t, Float t_prev, nalBasePose &pose,
                                                 const nalBasePose &default_pose)
{
    const float elapsed = std::fabs(static_cast<float>(t) - static_cast<float>(t_prev));
    const float parameter = this->field_18->get_param(this->field_14, 10);
    this->field_28.push_sample(Float(elapsed), Float(parameter));
    float angle = this->field_28.average(Float(1.0f));

    float upper_angle;
    float inverse_span;
    if (angle >= 315.0f) {
        this->field_20 = 11;
        this->field_24 = 0;
        upper_angle = 360.0f;
        inverse_span = 0.02222222276031971f;
    } else if (angle >= 270.0f) {
        this->field_20 = 10;
        this->field_24 = 11;
        upper_angle = 315.0f;
        inverse_span = 0.02222222276031971f;
    } else if (angle >= 225.0f) {
        this->field_20 = 9;
        this->field_24 = 10;
        upper_angle = 270.0f;
        inverse_span = 0.02222222276031971f;
    } else if (angle >= 180.0f) {
        this->field_20 = 8;
        this->field_24 = 9;
        upper_angle = 225.0f;
        inverse_span = 0.02222222276031971f;
    } else if (angle >= 22.5f) {
        this->field_20 = static_cast<int>(angle / 22.5f);
        this->field_24 = this->field_20 + 1;
        upper_angle = static_cast<float>(this->field_24) * 22.5f;
        inverse_span = 0.04444444552063942f;
    } else {
        if (angle < 0.0f) {
            angle = 0.0f;
        }

        this->field_20 = 0;
        this->field_24 = 2;
        upper_angle = 45.0f;
        inverse_span = 0.02222222276031971f;
    }
    const float blend = 1.0f - (upper_angle - angle) * inverse_span;

    nalAnyPose default_copy{default_pose, true};
    scoped_swing_pose original{default_copy.field_0};
    this->field_18->get_param(this->field_14, 0);
    scoped_swing_pose result{this->field_C->VirtualCreatePose()};
    sub_826190(*result.pose,
               Float(blend),
               *this->field_1C->field_3C[this->field_20]->field_0,
               *this->field_1C->field_3C[this->field_24]->field_0);
    this->field_C->VirtualCopyPose(pose, *result.pose);
}

void als_meta_anim_swing::nalInstance::_blend_two_anims(Float t0, Float t1, nalAnyPose &pose,
                                                        const nalAnyPose &default_pose, Float blend,
                                                        nalInstanceClass *anim0, nalInstanceClass *anim1)
{
    scoped_swing_pose pose0{this->field_C->VirtualCreatePose()};
    scoped_swing_pose pose1{this->field_C->VirtualCreatePose()};
    anim0->VirtualGetPose(t0, t0, *pose0.pose, *default_pose.field_0);
    anim1->VirtualGetPose(t1, t1, *pose1.pose, *default_pose.field_0);
    sub_826190(*pose.field_0, blend, *pose0.pose, *pose1.pose);
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
