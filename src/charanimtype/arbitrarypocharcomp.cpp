#include "arbitrarypocharcomp.h"

#include "common.h"
#include "func_wrapper.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "vector3d.h"
#include "vector4d.h"

#include <cmath>

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
        uint32_t a7,
        uint32_t a8,
        const void *a9,
        const void *a10,
        void *a11)
{
    TRACE("ArbitraryPOCharComp::CalcPoseDataRemapped");

    if constexpr (0) {
        this->CalcPoseDataDirect(
                a1,
                a2,
                a3,
                a4,
                a5,
                a6,
                a9,
                a10,
                a11);
    } else {
        THISCALL(0x005EF710, this, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
}

void ArbitraryPOCharComp::BlendPoseData(
        void *a1,
        uint32_t a2,
        Float a3,
        const void *a4,
        const void *a5,
        uint32_t a6,
        uint32_t a7)
{
    TRACE("ArbitraryPOCharComp::BlendPoseData");

    if constexpr (1)
    {
        uint32_t i = 0;
        for (; i < a6; ++i)
        {
            auto v13 = sub_5FD0C0(
                a3,
                *static_cast<const vector4d *>(a4),
                *static_cast<const vector4d *>(a5)
            );

            vector3d *v14 = static_cast<vector3d *>(a1);
            *v14 = v13;

            a1 = static_cast<char *>(a1) + 16;
            a4 = static_cast<const char *>(a4) +16;
            a5 = static_cast<const char *>(a5) +16;
        }

        for ( ; i < a7; ++i )
        {
            vector3d *v16 = static_cast<vector3d *>(a1);
            const vector3d *v4 = static_cast<const vector3d *>(a4);
            const vector3d *v5 = static_cast<const vector3d *>(a5);
            auto v10 = (*v5) - (*v4);
            auto v11 = v10 * a3;
            auto v12 = (*v4) + v11;

            *v16 = v12;

            a1 = static_cast<char *>(a1) + 12;
            a4 = static_cast<const char *>(a4) + 12;
            a5 = static_cast<const char *>(a5) + 12;
        }
    } else {
        THISCALL(0x005F6130, this, a1, a2, a3, a4, a5, a6, a7);
    }
}

void ArbitraryPOCharComp::BlendPoseData(
        void *a1,
        uint32_t a2,
        Float a3,
        const void *a4,
        const void *a5)
{
    this->BlendPoseData(
        static_cast<char *>(a1) + 16,
        a2,
        a3,
        static_cast<const char *>(a4) + 16,
        static_cast<const char *>(a5) + 16,
        static_cast<const uint32_t *>(a4)[0],
        static_cast<const uint32_t *>(a4)[1]
    );
}

void ArbitraryPOCharComp::SkelPoseProcess(uint32_t , void *a2, void *)
{
    *((int *)a2 + 6) += int(a2);
    *((int *)a2 + 7) += int(a2);
    auto v5 = *((int *)a2 + 5);
    char *v6 = ( v5 != 0 ) ? ((char *)a2 + v5) : nullptr;

    *((int *)a2 + 5) = int(v6);
    auto v7 = *((int *)a2 + 4);

    if ( v7 != 0 ) {
        *((int *)a2 + 4) = int((char *)a2 + v7);
    } else {
        *((int *)a2 + 4) = 0;
    }

}

int ArbitraryPOCharComp::GetDomain() const
{
    return 5;
}

uint32_t ArbitraryPOCharComp::GetPoseTypeID() const
{
    return to_hash("ArbitraryPO");
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

    {
        void (ArbitraryPOCharComp::*func)(
                void *,
                uint32_t,
                Float,
                const void *,
                const void *,
                uint32_t,
                uint32_t) = &ArbitraryPOCharComp::BlendPoseData;
        FUNC_ADDRESS(address, func);
        REDIRECT(0x005F9A49, address);
    }
}

