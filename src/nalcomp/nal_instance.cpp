#include "nal_instance.h"

#include "common.h"
#include "func_wrapper.h"
#include "nal_anim_comp.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(nalComp::nalCompInstance, 0x20u);

nalComp::nalCompSkeleton * nalComp::nalCompInstance::GetSkeleton()
{
    return bit_cast<nalCompSkeleton *>(this->field_C);
}

nalComp::nalCompAnim * nalComp::nalCompInstance::GetAnim()
{
    return bit_cast<nalCompAnim *>(this->field_10);
}

void nalComp::nalCompInstance::VirtualGetPose(
        Float a1,
        Float a2,
        nalBasePose *a3,
        const nalBasePose *a4)
{
    TRACE("nalComp::nalCompInstance::VirtualGetPose");

    const nalComp::nalCompPose *v5 = nullptr;
    if ( a4 != nullptr ) {
        v5 = (const nalComp::nalCompPose *)&a4[-1];
    }

    if ( a3 != nullptr ) {
        this->GetPose(a1, a2, (nalComp::nalCompPose *)&a3[-1], v5);
    } else {
        this->GetPose(a1, a2, nullptr, v5);
    }
}

void nalComp::nalCompInstance::GetPose(
        Float a2,
        Float a3,
        nalComp::nalCompPose *a4,
        const nalComp::nalCompPose *a5)
{
    TRACE("nalComp::nalCompInstance::GetPose");

    if constexpr (1) {
        *a4 = *a5;
        for ( int i = 0; i < this->field_18; ++i )
        {
            struct {
                int field_0;
                int field_4;
                int field_8;
                void *field_C;
                char field_10;
                char field_11;
                char field_12;
                char field_13;
            } *v30 = CAST(v30, ((char *)this->field_14 + 0x14 * i));
            if ( v30->field_8 != -1 && v30->field_11 )
            {
                auto v26 = v30->field_0;
                auto *v5 = this->GetSkeleton();
                auto *v29 = v5->GetComponent(v26);
                if ( v30->field_10 )
                {
                    auto v24 = v30->field_8;
                    auto *v7 = this->GetAnim();
                    auto animTrackData = v7->GetCompAnimTrackData(v24);
                    auto v22 = v30->field_8;
                    auto animDataInt = v7->GetCompPerAnimDataInt(v22);
                    auto v21 = v30->field_0;
                    auto *v8 = this->GetSkeleton();
                    auto skelDataInt = v8->GetCompPerSkelDataInt(v21);
                    auto v19 = v30->field_0;
                    auto *v9 = this->GetSkeleton();
                    auto v20 = v9->GetName(v19);
                    auto v10 = a4->GetComponentPoseData(v30->field_0);

                    v29->CalcPoseDataDirect(
                            v10,
                            v20,
                            a2,
                            a3,
                            v7,
                            skelDataInt,
                            bit_cast<void *>(animDataInt),
                            bit_cast<void *>(animTrackData),
                            v30->field_C
                    );
                }
                else
                {
                    auto v27 = v30->field_8;
                    auto anim = this->GetAnim();
                    auto *v12 = anim->GetSkeleton();
                    [[maybe_unused]] auto v28 = v12->GetComponentId(v27);
                    auto v25 = v30->field_8;
                    auto animTrackData = anim->GetCompAnimTrackData(v25);
                    auto v23 = v30->field_8;
                    auto animDataInt = anim->GetCompPerAnimDataInt(v23);
                    auto v20 = v30->field_0;
                    auto *Skeleton = anim->GetSkeleton();
                    auto skelDataInt = Skeleton->GetCompPerSkelDataInt(v20);
                    auto v18 = v30->field_0;
                    auto v16 = this->GetSkeleton();
                    auto v19 = v16->GetName(v18);
                    auto v17 = a4->GetComponentPoseData(v30->field_0);
                    v29->CalcPoseDataRemapped(
                            v17,
                            v19,
                            a2,
                            a3,
                            anim,
                            skelDataInt,
                            bit_cast<void *>(animDataInt),
                            bit_cast<void *>(animTrackData),
                            v30->field_C
                    );
                }
            }
        }
    } else {
        THISCALL(0x00733EA0, this, a2, a3, a4, a5);
    }
}

void nalCompInstance_patch()
{
    {
        FUNC_ADDRESS(address, &nalComp::nalCompInstance::VirtualGetPose);
        set_vfunc(0x008AA374, address);
    }
}
