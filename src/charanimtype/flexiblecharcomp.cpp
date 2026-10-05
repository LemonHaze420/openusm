#include "flexiblecharcomp.h"

#include "armikentcompdecomp.h"
#include "armikposedesc.h"
#include "armstdposedesc.h"
#include "common.h"
#include "fakerootentcompdecomp.h"
#include "fakerootposedesc.h"
#include "finger5stdposedesc.h"
#include "fing5curlentcompdecomp.h"
#include "fing5curlposedesc.h"
#include "fing5redentcompdecomp.h"
#include "fing5reducedposedesc.h"
#include "fing52knuckcurlentcompdecomp.h"
#include "fing52knuckcurlposedesc.h"
#include "func_wrapper.h"
#include "legsikentcompdecomp.h"
#include "legsikposedesc.h"
#include "legsstdposedesc.h"
#include "quatsentcompdecomp.h"
#include "torsoheadentcompdecomp.h"
#include "torsoheadstdposedesc.h"
#include "utility.h"
#include "variables.h"


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
        auto func = &FlexibleCharComp<ArmIKPoseDesc, ArmIKEntCompDecomp<ArmIKPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008924BC, address);
    }

    {
        auto func = &FlexibleCharComp<LegsIKPoseDesc, LegsIKEntCompDecomp<LegsIKPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x00892394, address);
    }

    {
        auto func = &FlexibleCharComp<LegsStdPoseDesc, QuatsEntCompDecomp<LegsStdPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008922FC, address);
    }

    {
        auto func = &FlexibleCharComp<Fing52KnuckCurlPoseDesc,
                                      Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008925CC, address);
    }

    {
        auto func = &FlexibleCharComp<Fing5CurlPoseDesc, Fing5CurlEntCompDecomp<Fing5CurlPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x00892664, address);
    }

    {
        auto func =
            &FlexibleCharComp<Fing5ReducedPoseDesc, Fing5RedEntCompDecomp<Fing5ReducedPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008926F4, address);
    }

    {
        auto func = &FlexibleCharComp<Finger5StdPoseDesc, QuatsEntCompDecomp<Finger5StdPoseDesc>>::_BuildBoneMatrices;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x00892784, address);
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
                                            void *a14) = &FlexibleCharCompClass::_CalcPoseDataRemapped;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008922BC, address);
    }
}
