#include "arbitrarypocharcomp.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

void ArbitraryPOCharComp::CalcPoseDataDirect(
        void *a1,
        uint32_t a2,
        Float a3,
        Float a4,
        const nalComp::nalCompAnim *a5,
        const void *a6,
        const void *a7,
        const void *a8,
        void *a9)
{
    TRACE("ArbitraryPOCharComp::CalcPoseDataDirect");

    sp_log("a2 = %u, a3 = %f, a4 = %f, a5 = 0x%08X, a6 = 0x%08X, a7 = 0x%08X, a8 = 0x%08X, a9 = 0x%08X",
            a2, a3, a4, int(a5), int(a6), int(a7), int(a8), int(a9));

    if constexpr (0) {
#if 0
        nalChar::nalCharAnim::ComputeFrameValues(
                (const nalChar::nalCharAnim *)a5,
                (float *)&v23,
                &v21,
                (unsigned int *)&a3,
                &v22,
                a3);
        v11 = (ArbitraryPOCharComp::PerInstData *)a9;
        v12 = *((_DWORD *)a9 + 9);
        v13 = LODWORD(a3);
        if ( LODWORD(a3) != v12 )
        {
            if ( LODWORD(a3) + 1 == v12 )
            {
                v14 = (unsigned __int8 *)*((_DWORD *)a9 + 4);
                v15 = (unsigned __int8 *)*((_DWORD *)a9 + 5);
                *((float *)a9 + 9) = a3;
                v11->field_10 = v15;
                v11->field_14 = v14;
            }
            else
            {
                if ( v12 == -1 || SLODWORD(a3) <= v12 )
                {
                    v16 = 0;
                    v24 = 65280;
                    *((_DWORD *)a9 + 7) = a8;
                    v11->field_1C.field_4 = 65280;
                }
                else
                {
                    v16 = v12 + 2;
                }
                for ( ; v16 <= v13; ++v16 )
                    ArbitraryPOCharComp::AdvanceAnimDataOneFrame(this, v11, (const nalChar::nalCharAnim *)a5, v16);
                v20 = v11->field_10;
                v11->field_24 = v13;
                ArbitraryPOCharComp::RetrievePoseFromInst(this, v20, v11);
            }
            v17 = v21;
            if ( !v21 )
            {
                v24 = 65280;
                v11->field_1C.field_0 = (void *)a8;
                v11->field_1C.field_4 = 65280;
            }
            ArbitraryPOCharComp::AdvanceAnimDataOneFrame(this, v11, (const nalChar::nalCharAnim *)a5, v17);
            ArbitraryPOCharComp::RetrievePoseFromInst(this, v11->field_14, v11);
        }
        v18 = 0;
        if ( v11->field_2C )
        {
            v18 = (ArbitraryPOCharComp::StdPoseData *)a1;
            v19 = v11->field_30;
        }
        else
        {
            v19 = (ArbitraryPOCharComp::StdPoseData *)a1;
        }
        ArbitraryPOCharComp::BlendAnimPoseToSkelData(this, v19, a2, v22, v11->field_10, v11->field_14, (unsigned int *)a7);
        if ( v11->field_2C )
            ArbitraryPOCharComp::CopyRemapDataFromTempPose(this, v18, v19, v11, (const ArbitraryPOCharComp::PerSkelData *)a6);
#endif
    } else {
        THISCALL(0x005F98E0, this, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
}

void ArbitraryPOCharComp::CalcPoseDataRemapped(
        void *a1,
        uint32_t a2,
        Float a3,
        Float a4,
        const nalComp::nalCompAnim *a5,
        const void *a6,
        const void *a7,
        const void *a8,
        void *a9)
{
    TRACE("ArbitraryPOCharComp::CalcPoseDataRemapped");

    if constexpr (0) {
    } else {
        THISCALL(0x005EF710, this, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
}

void ArbitraryPOCharComp_patch()
{
    {
        FUNC_ADDRESS(address, &ArbitraryPOCharComp::CalcPoseDataDirect);
        set_vfunc(0x008920E8, address);
    }

    {
        FUNC_ADDRESS(address, &ArbitraryPOCharComp::CalcPoseDataRemapped);
        set_vfunc(0x008920EC, address);
    }
}

