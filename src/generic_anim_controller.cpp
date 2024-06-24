#include "generic_anim_controller.h"

#include "func_wrapper.h"

#include <nal_generic.h>


generic_anim_controller::generic_anim_controller(actor *a2,
                                                nalBaseSkeleton *a3,
                                                unsigned int a4,
                                                als::als_meta_anim_table_shared *a5) : nal_anim_controller(a2, a3, a4, a5), field_54(this), field_5C(this)
{
    if constexpr (0) {
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
