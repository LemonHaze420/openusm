#include "arbitrarypocharcomp.h"

#include "charcomponentmanager.h"
#include "common.h"
#include "func_wrapper.h"
#include "nal_math.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vector3d.h"
#include "vector4d.h"
#include "vtbl.h"

#include <cmath>

VALIDATE_SIZE(ArbitraryPOCharComp::PerSkelData, 0x20);

VALIDATE_SIZE(ArbitraryPOCharComp::BoneData, 0x30);

VALIDATE_SIZE(ArbitraryPOCharComp::StdPoseData, 0x20);

VALIDATE_SIZE(ArbitraryPOCharComp::PerInstData, 0x3C);

ArbitraryPOCharComp::ArbitraryPOCharComp()
{
    TRACE("ArbitraryPOCharComp()");

#if STANDALONE_SYSTEM
    if constexpr (1) {
#else
    if constexpr (0) {
#endif
        static void *g_vtbl[]{nullptr,
                              func_address(&CharComponentBase::_GetType),
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              func_address(&ArbitraryPOCharComp::_SkelPoseProcess),
                              func_address(&ArbitraryPOCharComp::_AnimProcess),
                              func_address(&ArbitraryPOCharComp::_AnimProcess),
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              func_address(&ArbitraryPOCharComp::_CopyPoseDataToNothing)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x008920B8;
    }

    this->m_strTypeString = "ArbitraryPO";
    CharComponentManager::RegisterComponent(this);
}

const void *ArbitraryPOCharComp::_ApplyPublicPerSkelDataOffset(uint32_t, const void *a2)
{
    TRACE("ArbitraryPOCharComp::ApplyPublicPerSkelDataOffset");

    return a2;
}

int ArbitraryPOCharComp::_ApplyPublicPerAnimDataOffset(uint32_t, const void *)
{
    TRACE("ArbitraryPOCharComp::ApplyPublicPerAnimDataOffset");

    return 0;
}

nalPositionOrientation *ArbitraryPOCharComp::_GetTrajectoryData(nalPositionOrientation *out, uint32_t, const void *,
                                                                const void *)
{
    TRACE("ArbitraryPOCharComp::GetTrajectoryData");

    *out = nalPositionOrientation::Identity;
    return out;
}

void ArbitraryPOCharComp::_BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const void *a3, const void *a4)
{
    TRACE("ArbitraryPOCharComp::BuildBoneMatrices");

    if constexpr (1) {
        auto *perSkelData = static_cast<const PerSkelData *>(a3);
        auto *stdPoseData = static_cast<const StdPoseData *>(a4);
        auto *v23 = perSkelData->field_10;
        const nalVector3 *v24 = perSkelData->field_14;
        const auto *v6 = stdPoseData->field_10;
        const nalVector3 *v7 = &stdPoseData->field_10[stdPoseData->field_0];

        for (uint32_t i = 0; i < perSkelData->field_0; ++i) {
            auto *v9 = &perSkelData->field_18[perSkelData->field_1C[i]];
            if (v9->field_24 != -1) {
                auto *v10 = (v9->field_28 != 0 ? &v6[v9->field_20] : &v23[v9->field_20]);

                auto &v15 = (v9->field_2A != 0 ? v24[v9->field_22] : v7[v9->field_22]);

                nalPositionOrientation v26{v15, v10->field_0};

                nalMatrix4x4 v30{v26};

                a1[v9->field_24] = v30;

                if (v9->field_26 != -1) {
                    auto &v20 = a1[v9->field_24];
                    v20 = sub_5FE000(v20, a1[v9->field_26]);
                }
            }
        }

    } else {
        void(__fastcall * func)(void *self, void *edx, nalMatrix4x4 *a1, uint32_t a2, const void *a3, const void *a4) =
            CAST(func, 0x005F5E60);
        func(this, nullptr, a1, a2, a3, a4);
    }
}

bool sub_C75AA0(const int *a1, uint32_t a2)
{
    return (a1[a2 >> 5] & (1 << (a2 % 32))) != 0;
}

int ArbitraryPOCharComp::_GetSizeOfPerInstData(uint32_t, const void *, const void *a3, const void *, const void *a5,
                                               const void *, bool)
{
    TRACE("ArbitraryPOCharComp::GetSizeOfPerInstData");

    auto func = [](const int *a5, const int *a3) -> int {
        int result = 0;
        const int v9 = a3[1] + a3[2];
        for (int i = 0; i < v9; ++i) {
            if (sub_C75AA0(a5, i)) {
                result += 3;
            }
        }

        return result;
    };

    return 16 * func(static_cast<const int *>(a5), static_cast<const int *>(a3)) + 60;
}

int ArbitraryPOCharComp::_GetAlignOfPerInstData(uint32_t, const void *, const void *, const void *, const void *,
                                                const void *, bool)
{
    return 4;
}

int sub_C7F000(int a1, int a2)
{
    return ~(a2 - 1) & (a1 + a2 - 1);
}

void ArbitraryPOCharComp::_BuildPerInstData(void *a1, uint32_t a2, const void *a3, const void *a4, const void *a5,
                                            const void *a6, const void *a7, bool bIsRemapped)
{
    TRACE("ArbitraryPOCharComp::BuildPerInstData");

    if constexpr (1) {
        auto *v1 = static_cast<PerInstData *>(a1);
        auto *v3 = static_cast<const PerSkelData *>(a3);
        auto *v4 = static_cast<const PerSkelData *>(a4);
        v1->field_0 = 0;
        int v12 = 0;
        for (; v12 < v4->field_4; ++v12) {
            if (sub_C75AA0(static_cast<const int *>(a6), v12)) {
                ++v1->field_0;
            }
        }

        for (v1->field_4 = v1->field_0; v12 < v4->field_4 + v4->field_8; ++v12) {
            if (sub_C75AA0(static_cast<const int *>(a6), v12)) {
                ++v1->field_4;
            }
        }

        auto v14 = sub_C7F000(v4->field_8 + v4->field_4, 32) >> 5;
        new (a1) PerInstData{a7, bit_cast<uint8_t *>(a6) + 4 * v14};

        auto v16 = 16 * v1->field_0;
        v16 += 12 * (v1->field_4 - v1->field_0);
        v16 = sub_C7F000(v16, 16u);

        v1->field_24 = -1;
        v1->field_C = v16;
        auto *v17 = static_cast<uint8_t *>(tlMemAlloc(2 * v1->field_C, 16u, 0));
        v1->field_18 = v17;
        tlMemFree(v17);

        auto *v18 = static_cast<uint8_t *>(tlMemAlloc(2 * v1->field_C, 16u, 0));
        auto *v20 = &v18[v1->field_C];
        v1->field_18 = v18;
        v1->field_10 = v18;
        auto v21 = 3 * v1->field_4;
        v1->field_14 = v20;
        v1->field_8 = v21;

        if (v4 == v3) {
            v1->field_2C = 0;
            v1->field_30 = nullptr;
            v1->field_34 = nullptr;
            v1->field_38 = nullptr;
        } else {
            v1->field_2C = 1;
            v1->field_34 = v4;

            auto dwSize = sub_C7F000(16 * v4->field_4 + 16 + 12 * v4->field_8, 16);
            v1->field_30 = static_cast<StdPoseData *>(tlMemAlloc(dwSize, 16u, 0));
            v1->field_30->field_0 = v4->field_4;
            v1->field_30->field_4 = v4->field_4 + v4->field_8;
            v1->field_38 = static_cast<int *>(tlMemAlloc(8 * (v3->field_4 + v3->field_8), 4u, 0));

            int v14 = 0;
            int v13 = v3->field_4;
            for (uint32_t j = 0; j < v3->field_0; ++j) {
                auto *v25 = &v3->field_18[j];
                if (v25->field_28 != 0 || v25->field_2A != 0) {
                    uint32_t k;
                    for (k = 0; k < v4->field_0 && v4->field_18[k].field_0 != v25->field_0; ++k) {
                        ;
                    }

                    if (k == v4->field_0) {
                        if (v25->field_28 != 0) {
                            v1->field_38[2 * v14++] = -1;
                        }

                        if (v25->field_2A != 0) {
                            v1->field_38[2 * v13++] = -1;
                        }
                    } else {
                        auto *v30 = &v4->field_18[k];
                        if (v25->field_28 != 0) {
                            if (v30->field_28 != 0) {
                                if (sub_C75AA0(static_cast<const int *>(a6), v30->field_20)) {
                                    v1->field_38[2 * v14] = v30->field_20;
                                    v1->field_38[2 * v14 + 1] = 1;
                                } else {
                                    v1->field_38[2 * v14] = -1;
                                }

                                ++v14;
                            } else {
                                v1->field_38[2 * v14] = v30->field_20;
                                v1->field_38[2 * v14 + 1] = 0;
                                ++v14;
                            }
                        }

                        if (v25->field_2A != 0) {
                            if (v30->field_2A != 0) {
                                if (sub_C75AA0(static_cast<const int *>(a6), v4->field_4 + v30->field_22)) {
                                    v1->field_38[2 * v13] = v30->field_22;
                                    v1->field_38[2 * v13 + 1] = 1;
                                } else {
                                    v1->field_38[2 * v13] = -1;
                                }

                                ++v13;
                            } else {
                                v1->field_38[2 * v13] = v30->field_22;
                                v1->field_38[2 * v13 + 1] = 0;
                                ++v13;
                            }
                        }
                    }
                }
            }
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                void *,
                                uint32_t,
                                const void *,
                                const void *,
                                const void *,
                                const void *,
                                const void *,
                                bool) = CAST(func, 0x005F2270);
        func(this, nullptr, a1, a2, a3, a4, a5, a6, a7, bIsRemapped);
    }
}

void ArbitraryPOCharComp::_DestroyPerInstData(void *a1, uint32_t, const void *, const void *)
{
    TRACE("ArbitraryPOCharComp::DestroyPerInstData");

    tlMemFree(*((void **)a1 + 6));
    auto v4 = *((DWORD *)a1 + 11);
    *((DWORD *)a1 + 4) = 0;
    *((DWORD *)a1 + 5) = 0;
    *((DWORD *)a1 + 6) = 0;
    if (v4 != 0) {
        tlMemFree(*((void **)a1 + 14));
        tlMemFree(*((void **)a1 + 12));
        *((DWORD *)a1 + 14) = 0;
        *((DWORD *)a1 + 12) = 0;
        *((DWORD *)a1 + 13) = 0;
        *((DWORD *)a1 + 11) = 0;
    }
}

bool ArbitraryPOCharComp::_WillMapToComponentData(uint32_t, uint32_t, uint32_t a4)
{
    TRACE("ArbitraryPOCharComp::WillMapToComponentData");

    return a4 == this->GetType();
}

void ArbitraryPOCharComp::_CalcPoseDataDirect(void *a1, uint32_t a2, Float a3, Float a4, const nalComp::nalCompAnim *a5,
                                              const void *a6, const void *a7, const void *a8, void *a9)
{
    TRACE("ArbitraryPOCharComp::CalcPoseDataDirect");

    sp_log("a2 = %u, a3 = %f, a4 = %f, a5 = 0x%08X, a6 = 0x%08X, a7 = 0x%08X, a8 = 0x%08X, a9 = 0x%08X",
           a2,
           a3,
           a4,
           int(a5),
           int(a6),
           int(a7),
           int(a8),
           int(a9));

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

void ArbitraryPOCharComp::_CalcPoseDataRemapped(void *a1, uint32_t a2, Float a3, Float a4,
                                                const nalComp::nalCompAnim *a5, const void *a6, uint32_t a7,
                                                uint32_t a8, const void *a9, const void *a10, void *a11)
{
    TRACE("ArbitraryPOCharComp::CalcPoseDataRemapped");

    if constexpr (0) {
        this->CalcPoseDataDirect(a1, a2, a3, a4, a5, a6, a9, a10, a11);
    } else {
        THISCALL(0x005EF710, this, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
}

void ArbitraryPOCharComp::BlendPoseData(void *a1, uint32_t a2, Float blend, const void *a4, const void *a5, uint32_t a6,
                                        uint32_t a7)
{
    TRACE("ArbitraryPOCharComp::BlendPoseData");

    if constexpr (1) {
        uint32_t i = 0;
        for (; i < a6; ++i) {
            auto v13 = math::Slerp(blend, *static_cast<const vector4d *>(a4), *static_cast<const vector4d *>(a5));

            vector3d *v14 = static_cast<vector3d *>(a1);
            *v14 = v13;

            a1 = static_cast<char *>(a1) + 16;
            a4 = static_cast<const char *>(a4) + 16;
            a5 = static_cast<const char *>(a5) + 16;
        }

        for (; i < a7; ++i) {
            vector3d *v16 = static_cast<vector3d *>(a1);
            const vector3d *v4 = static_cast<const vector3d *>(a4);
            const vector3d *v5 = static_cast<const vector3d *>(a5);
            auto v10 = (*v5) - (*v4);
            auto v11 = v10 * blend;
            auto v12 = (*v4) + v11;

            *v16 = v12;

            a1 = static_cast<char *>(a1) + 12;
            a4 = static_cast<const char *>(a4) + 12;
            a5 = static_cast<const char *>(a5) + 12;
        }
    } else {
        THISCALL(0x005F6130, this, a1, a2, blend, a4, a5, a6, a7);
    }
}

void ArbitraryPOCharComp::_BlendPoseData(void *a1, uint32_t a2, Float blend, const void *a4, const void *a5)
{
    this->BlendPoseData(static_cast<char *>(a1) + 16,
                        a2,
                        blend,
                        static_cast<const char *>(a4) + 16,
                        static_cast<const char *>(a5) + 16,
                        static_cast<const uint32_t *>(a4)[0],
                        static_cast<const uint32_t *>(a4)[1]);
}

void ArbitraryPOCharComp::_SkelPoseProcess(uint32_t, void *a2, void *)
{
    TRACE("ArbitraryPOCharComp::SkelPoseProcess");

    auto *v2 = static_cast<PerSkelData *>(a2);
    v2->field_18 = CAST(v2->field_18, int(v2->field_18) + int(a2));
    v2->field_1C = CAST(v2->field_1C, int(v2->field_1C) + int(a2));
    auto *v5 = v2->field_14;
    v2->field_14 = (v5 != nullptr) ? CAST(v2->field_14, int(a2) + int(v5)) : nullptr;

    auto *v7 = v2->field_10;
    if (v7 != nullptr) {
        v2->field_10 = CAST(v2->field_10, int(a2) + int(v7));
    } else {
        v2->field_10 = nullptr;
    }
}

void ArbitraryPOCharComp::_SkelPoseRelease(uint32_t, void *out, void *)
{
    TRACE("ArbitraryPOCharComp::SkelPoseRelease");

    *((DWORD *)out + 6) -= int(out);
    *((DWORD *)out + 7) -= int(out);
    auto v3 = *((DWORD *)out + 5);
    int v4;
    if (v3) {
        v4 = v3 - (DWORD)out;
    } else {
        v4 = 0;
    }

    *((DWORD *)out + 5) = v4;
    auto v5 = *((DWORD *)out + 4);
    if (v5) {
        *((DWORD *)out + 4) = v5 - (DWORD)out;
    } else {
        *((DWORD *)out + 4) = 0;
    }
}

void ArbitraryPOCharComp::_AnimProcess(uint32_t, void *, void *, const void *)
{
    ;
}

void ArbitraryPOCharComp::_AnimRelease(uint32_t, void *, void *, const void *)
{
    ;
}

void ArbitraryPOCharComp::_CopyPoseExtraData(void *a1, uint32_t, const void *a3)
{
    TRACE("ArbitraryPOCharComp::CopyPoseExtraData");

    std::memcpy(a1, a3, 16 * (*(const DWORD *)a3 + 1) + 12 * (*((const DWORD *)a3 + 1) - *(const DWORD *)a3));
}

void ArbitraryPOCharComp::_PoseDataFree(uint32_t, void *)
{
    ;
}

int ArbitraryPOCharComp::_GetDomain() const
{
    return 5;
}

uint32_t ArbitraryPOCharComp::_GetPoseTypeID() const
{
    return to_hash("ArbitraryPO");
}

void ArbitraryPOCharComp::_CopyPoseDataToNothing(void *a1, uint32_t, const void *a3)
{
    TRACE("ArbitraryPOCharComp::CopyPoseDataToNothing");

    std::memcpy(a1, a3, 16 * (*(const DWORD *)a3 + 1) + 12 * (*((const DWORD *)a3 + 1) - *(const DWORD *)a3));
}

ArbitraryPOCharComp::PerInstData::PerInstData(const void *a2, uint8_t *a3) : field_1C(a2, false)
{
    this->field_24 = -1;
    this->field_28 = a3;
}

void sub_853300()
{
    static ArbitraryPOCharComp g_ArbitraryPOCharComp{};
}

void ArbitraryPOCharComp_patch()
{
    static constexpr auto address_vtbl = 0x008920B8;

    auto set_vfunc_local = [](std::intptr_t offset, auto func) {
        set_vfunc(address_vtbl + offset, func_address(func));
    };

    {
        set_vfunc_local(0x4, &ArbitraryPOCharComp::_GetType);
        set_vfunc_local(0x8, &ArbitraryPOCharComp::_ApplyPublicPerSkelDataOffset);
        set_vfunc_local(0xC, &ArbitraryPOCharComp::_ApplyPublicPerAnimDataOffset);
        set_vfunc_local(0x10, &ArbitraryPOCharComp::_GetTrajectoryData);
        set_vfunc_local(0x14, &ArbitraryPOCharComp::_BuildBoneMatrices);
        set_vfunc_local(0x18, &ArbitraryPOCharComp::_DoesContributeToPose);

        set_vfunc_local(0x1C, &ArbitraryPOCharComp::_GetSizeOfPerInstData);
        set_vfunc_local(0x20, &ArbitraryPOCharComp::_GetAlignOfPerInstData);

        set_vfunc_local(0x24, &ArbitraryPOCharComp::_BuildPerInstData);
        set_vfunc_local(0x28, &ArbitraryPOCharComp::_DestroyPerInstData);
        set_vfunc_local(0x2C, &ArbitraryPOCharComp::_WillMapToComponentData);
        set_vfunc_local(0x30, &ArbitraryPOCharComp::_CalcPoseDataDirect);
    }

    {
        void (ArbitraryPOCharComp::*func)(void *a1,
                                          uint32_t a2,
                                          Float a3,
                                          Float a4,
                                          const nalComp::nalCompAnim *a5,
                                          const void *a6,
                                          uint32_t a7,
                                          uint32_t a8,
                                          const void *a9,
                                          const void *a10,
                                          void *a11) = &ArbitraryPOCharComp::_CalcPoseDataRemapped;
        set_vfunc_local(0x34, func);
    }

    {
        void (ArbitraryPOCharComp::*func)(void *, uint32_t, Float, const void *, const void *) =
            &ArbitraryPOCharComp::_BlendPoseData;
        set_vfunc_local(0x38, func);
    }

    {
        set_vfunc_local(0x3C, &ArbitraryPOCharComp::_SkelPoseProcess);
        set_vfunc_local(0x40, &ArbitraryPOCharComp::_SkelPoseRelease);
        set_vfunc_local(0x44, &ArbitraryPOCharComp::_AnimProcess);
        set_vfunc_local(0x48, &ArbitraryPOCharComp::_AnimRelease);
        set_vfunc_local(0x4C, &ArbitraryPOCharComp::_CopyPoseExtraData);
        set_vfunc_local(0x50, &ArbitraryPOCharComp::_PoseDataFree);
        set_vfunc_local(0x54, &ArbitraryPOCharComp::_GetDomain);
        set_vfunc_local(0x58, &ArbitraryPOCharComp::_GetPoseTypeID);
        set_vfunc_local(0x5C, &CharComponentBase::_GetRemapSizeOfPerInstData);
        set_vfunc_local(0x60, &CharComponentBase::_GetRemapAlignOfPerInstData);
        set_vfunc_local(0x64, &CharComponentBase::_BuildRemapPerInstData);
        set_vfunc_local(0x68, &CharComponentBase::_DestroyRemapPerInstData);
        set_vfunc_local(0x6C, &CharComponentBase::_CalcPoseDataRemapped);
        set_vfunc_local(0x70, &CharComponentBase::_AnimRelease);
        set_vfunc_local(0x74, &ArbitraryPOCharComp::_CopyPoseDataToNothing);
        set_vfunc_local(0x78, &CharComponentBase::_AllocTempPoseData);
        set_vfunc_local(0x7C, &CharComponentBase::_DeleteTempPoseData);
    }

    {
        SET_JUMP(0x00853300, sub_853300);
    }
}
