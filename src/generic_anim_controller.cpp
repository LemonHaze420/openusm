#include "generic_anim_controller.h"

#include "func_wrapper.h"

#include <nal_generic.h>
#include "trace.h"


generic_anim_controller::generic_anim_controller(actor *a2,
                        nalBaseSkeleton *a3,
                        unsigned int a4,
                        als::als_meta_anim_table_shared *a5) : nal_anim_controller(a2, a3, a4, a5), field_54(this), field_5C(this)
{
    TRACE("generic_anim_controller::generic_anim_controller");

    if constexpr (1) {
        this->m_vtbl = 0x00880EE8;

        this->field_64 = {};
        this->field_74 = {};
        this->field_84 = {};
        this->field_94 = {};
        this->field_A4 = {};
        this->field_B4 = {};
        this->field_C4 = {};
        this->field_D4 = {};
        this->field_E4 = {};
        this->field_F4 = {};

        tlFixedString v7 {"AE_Base_Bone"};
        tlFixedString v8 {"camera_root"};

        auto *Skel = bit_cast<nalGeneric::nalGenericSkeleton *>(this->field_40.field_0->field_0);
        Skel->GetComponentHandle<nalPositionOrientation>(
            this->field_64,
            v8,
            v7);

        v8 = {"AE_Base_Bone"};
        v7 = {"shake_root"};
        Skel->GetComponentHandle<nalPositionOrientation>(
            this->field_74,
            v7,
            v8);

        v8 = {"AE_Floor_Offset"};
        v7 = {"fakeroot"};
        Skel->GetComponentHandle<float>(
            this->field_84,
            v7,
            v8);

        v8 = {"NAL_TRAJECTORY"};
        v7 = {"fakeroot"};
        Skel->GetComponentHandle<nalPositionOrientation>(
            this->field_94,
            v7,
            v8);

        v8 = {"USMEvent"};
        v7 = {"fakeroot"};
        Skel->GetComponentHandle<unsigned char>(
            this->field_A4,
            v7,
            v8);

        v8 = {"MaxParamFloat.FOV"};
        v7 = {"camera"};
        Skel->GetComponentHandle<float>(
            this->field_B4,
            v7,
            v8);

        v8 = {"MaxParamFloat.Far Env Range"};
        v7 = {"camera"};
        Skel->GetComponentHandle<float>(
            this->field_C4,
            v7,
            v8);

        v8 = {"MaxParamFloat.Tentacle Base Diameter"};
        v7 = {"fakeroot"};
        Skel->GetComponentHandle<float>(
            this->field_D4,
            v7,
            v8);

        v8 = {"MaxParamFloat.Subtentacle Activity"};
        v7 = {"fakeroot"};
        Skel->GetComponentHandle<float>(
            this->field_E4,
            v7,
            v8);

        v8 = {"MaxParamFloat.Pull Factor"};
        v7 = {"fakeroot"};
        Skel->GetComponentHandle<float>(
            this->field_F4,
            v7,
            v8);
    } else {
        THISCALL(0x0049C190, this);
    }
}

nalGeneric::nalGenericPose * generic_anim_controller::GetPose()
{
    return bit_cast<nalGeneric::nalGenericPose *>(this->field_40.field_0);
}

tlFixedString tlfs_AE_SCALE {"AE_SCALE"};

bool generic_anim_controller::will_have_hint_token_scale(string_hash a2)
{
    auto *pose = this->GetPose();
    auto *v2 = pose->GetSkeleton();
    nalGeneric::nalGenericConstComponentHandle<nalVector3> v4 {};
    v2->GetComponentHandle(v4, a2.source_hash_code, tlfs_AE_SCALE);
    return v4.Skeleton != nullptr;
}

tlFixedString tlfs_NAL_SCALE {"NAL_SCALE"};

vector3d generic_anim_controller::get_hint_token_scale(
        string_hash a2)
{
    TRACE("generic_anim_controller::get_hint_token_scale");

    if constexpr (0) {
        auto *v3 = this->GetPose();
        nalGeneric::nalGenericSkeleton *v4 = v3->GetSkeleton();

        nalGeneric::nalGenericConstComponentHandle<nalVector3> v10 {};
        v4->GetComponentHandle(
                v10,
                a2.source_hash_code,
                tlfs_NAL_SCALE); 
        if ( v10.Skeleton != nullptr )
        {
            nalVector3 v5 = (*v3)[v10];
            return vector3d {v5[0], v5[1], v5[2]};
        }

        return vector3d {1.f, 1.f, 1.f};
    } else {
        vector3d result;
        THISCALL(0x0049C7F0, this, &result, a2);
        return result;
    }
}

void generic_anim_controller_patch() {
    FUNC_ADDRESS(address, &generic_anim_controller::get_hint_token_scale);
    set_vfunc(0x00880F84, address);
}
