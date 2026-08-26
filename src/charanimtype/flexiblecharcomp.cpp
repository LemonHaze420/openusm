#include "flexiblecharcomp.h"

#include "armstdposedesc.h"
#include "common.h"
#include "fakerootentcompdecomp.h"
#include "fakerootposedesc.h"
#include "fing52knuckcurlentcompdecomp.h"
#include "fing52knuckcurlposedesc.h"
#include "func_wrapper.h"
#include "legsikentcompdecomp.h"
#include "legsikposedesc.h"
#include "quatsentcompdecomp.h"
#include "torsoheadentcompdecomp.h"
#include "torsoheadstdposedesc.h"
#include "utility.h"
#include "variables.h"

template <>
void FlexibleCharComp<FakerootPoseDesc, FakerootEntCompDecomp<FakerootPoseDesc>>::_CalcPoseDataRemapped(
    void *, uint32_t, Float, Float, const nalComp::nalCompAnim *, const void *, const void *, const void *, void *)
{
    TRACE("FlexibleCharComp<FakerootPoseDesc,FakerootEntCompDecomp<FakerootPoseDesc>>::CalcPoseDataRemapped");
}

template <>
void FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>>::_CalcPoseDataRemapped(
    void *, uint32_t, Float, Float, const nalComp::nalCompAnim *, const void *, const void *, const void *, void *)
{
    TRACE("FlexibleCharComp<TorsoHeadStdPoseDesc,TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>>::CalcPoseDataRemapped");
}

template <>
void FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>>::_CalcPoseDataRemapped(
    void *a2, uint32_t a3, Float a4, Float a5, const nalComp::nalCompAnim *a6, const void *a7, uint32_t a8, uint32_t a9,
    const void *a10, const void *a11, const void *a12, const void *a13, void *a14)
{
    TRACE("FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>>::CalcPoseDataRemapped");

    if constexpr (1) {
        auto *ComponentByType = (CharComponentBase *)CharComponentManager::GetComponentByType(a9);
        auto v16 = ComponentByType->GetPoseTypeID();
        auto v17 = this->GetPoseTypeID();
        if (v16 == v17) {
            ComponentByType->CalcPoseDataDirect(a2, a8, a4, a5, a6, a10, a12, a13, a14);
        } else {
            auto *v19 = ComponentByType->AllocTempPoseData(a8, a10, a11);
            ComponentByType->CalcPoseDataDirect(v19, a8, a4, a5, a6, a10, a12, a13, a14);
            ComponentByType->DeleteTempPoseData(a8, v19);
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                void *,
                                uint32_t,
                                Float,
                                Float,
                                const nalComp::nalCompAnim *,
                                const void *,
                                uint32_t,
                                uint32_t,
                                const void *,
                                const void *,
                                const void *,
                                const void *,
                                void *) = CAST(func, 0x005FBF90);

        func(this, nullptr, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    }
}

void FlexibleCharComp_patch()
{
    {
        auto func = &FlexibleCharComp<FakerootPoseDesc, FakerootEntCompDecomp<FakerootPoseDesc>>::_CalcPoseDataDirect;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008921F0, address);
    }

    {
        auto func = &FlexibleCharComp<LegsIKPoseDesc, LegsIKEntCompDecomp<LegsIKPoseDesc>>::_CalcPoseDataDirect;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008923B0, address);
    }

    {
        auto func = &FlexibleCharComp<ArmStdPoseDesc, QuatsEntCompDecomp<ArmStdPoseDesc>>::_CalcPoseDataDirect;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x00892440, address);
    }

    {
        auto func =
            &FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>>::_CalcPoseDataDirect;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x00892280, address);
    }

    {
        auto func = &FlexibleCharComp<Fing52KnuckCurlPoseDesc,
                                      Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>>::_CalcPoseDataDirect;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008925E8, address);
    }

    {
        auto func =
            &FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x00892264, address);
    }

    {
        auto func = &FlexibleCharComp<ArmStdPoseDesc, QuatsEntCompDecomp<ArmStdPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x00892424, address);
    }

    {
        using FlexibleCharCompClass =
            FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>>;

        void (FlexibleCharCompClass::*func)(void *a2,
                                            uint32_t a3,
                                            Float a4,
                                            Float a5,
                                            const nalComp::nalCompAnim *a6,
                                            const void *a7,
                                            uint32_t a8,
                                            uint32_t a9,
                                            const void *a10,
                                            const void *a11,
                                            const void *a12,
                                            const void *a13,
                                            void *a14) = FlexibleCharCompClass::_CalcPoseDataRemapped;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008922BC, address);
    }
}
