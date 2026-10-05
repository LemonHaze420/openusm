#include "arbitrarypocharcomp.h"

#include "charcomponentmanager.h"
#include "common.h"
#include "func_wrapper.h"
#include "nal_math.h"
#include "nativeentcompdecomp.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vector3d.h"
#include "vector4d.h"
#include "vtbl.h"

#include <cmath>
#include <utility>

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
        static void *g_vtbl[]{
            func_address(&ArbitraryPOCharComp::_DestroyComponent),
            func_address(&CharComponentBase::_GetType),
            func_address(&ArbitraryPOCharComp::_ApplyPublicPerSkelDataOffset),
            func_address(&ArbitraryPOCharComp::_ApplyPublicPerAnimDataOffset),
            func_address(&ArbitraryPOCharComp::_GetTrajectoryData),
            func_address(&ArbitraryPOCharComp::_BuildBoneMatrices),
            func_address(&ArbitraryPOCharComp::_DoesContributeToPose),
            func_address(&ArbitraryPOCharComp::_GetSizeOfPerInstData),
            func_address(&ArbitraryPOCharComp::_GetAlignOfPerInstData),
            func_address(&ArbitraryPOCharComp::_BuildPerInstData),
            func_address(&ArbitraryPOCharComp::_DestroyPerInstData),
            func_address(&ArbitraryPOCharComp::_WillMapToComponentData),
            func_address(&ArbitraryPOCharComp::_CalcPoseDataDirect),
            func_address(&ArbitraryPOCharComp::_CalcPoseDataRemapped),
            func_address(&ArbitraryPOCharComp::_BlendPoseData),
            func_address(&ArbitraryPOCharComp::_SkelPoseProcess),
            func_address(&ArbitraryPOCharComp::_SkelPoseRelease),
            func_address(&ArbitraryPOCharComp::_AnimProcess),
            func_address(&ArbitraryPOCharComp::_AnimRelease),
            func_address(&ArbitraryPOCharComp::_CopyPoseExtraData),
            func_address(&ArbitraryPOCharComp::_PoseDataFree),
            func_address(&ArbitraryPOCharComp::_GetDomain),
            func_address(&ArbitraryPOCharComp::_GetPoseTypeID),
            func_address(&CharComponentBase::_GetRemapSizeOfPerInstData),
            func_address(&CharComponentBase::_GetRemapAlignOfPerInstData),
            func_address(&CharComponentBase::_BuildRemapPerInstData),
            func_address(&CharComponentBase::_DestroyRemapPerInstData),
            func_address(&CharComponentBase::_CalcPoseDataRemapped),
            func_address(&CharComponentBase::_AnimRelease),
            func_address(&ArbitraryPOCharComp::_CopyPoseDataToNothing),
            func_address(&CharComponentBase::_AllocTempPoseData),
            func_address(&CharComponentBase::_DeleteTempPoseData)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x008920B8;
    }

    this->m_strTypeString = "ArbitraryPO";
    CharComponentManager::RegisterComponent(this);
}

ArbitraryPOCharComp *ArbitraryPOCharComp::_DestroyComponent(unsigned char flags)
{
    this->~ArbitraryPOCharComp();
    if (flags & 1) operator delete(this);
    return this;
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

void ArbitraryPOCharComp::_CalcPoseDataDirect(void *out, uint32_t, Float time, Float,
    const nalComp::nalCompAnim *clip, const void *, const void *animData, const void *stream, void *instance)
{
    using Converter = CharEntropyQuantConverter;
    auto *state = static_cast<PerInstData *>(instance);
    const auto *anim = static_cast<const nalChar::nalCharAnim *>(clip);
    auto *tracks = reinterpret_cast<Converter::EncTrackData *>(state + 1);
    const auto *mask = static_cast<const uint32_t *>(animData);
    auto advance = [&](uint32_t frame) {
        Converter::DecodeDequantTracks(tracks, state->field_28, state->field_1C, frame, 0, state->field_8,
            anim->GetAnimQuantScale() * (1.0f / 1024.0f), anim->IsSceneAnim());
        if (!frame) return;
        uint32_t index = 0;
        for (int i = 0; i < state->field_0; ++i, index += 3) {
            if (frame == 1) Converter::UnEntropyQuaternionTracksInitial(tracks, state->field_28, index);
            else Converter::UnEntropyQuaternionTracks(tracks, state->field_28, index);
        }
        for (; index < static_cast<uint32_t>(state->field_8); ++index) {
            if (frame == 1) Converter::UnEntropyLinearTrackInitial(tracks, state->field_28, index);
            else Converter::UnEntropyLinearTrack(tracks, state->field_28, index);
        }
    };
    auto retrieve = [&](uint8_t *buffer) {
        auto *pose = reinterpret_cast<float *>(buffer);
        unsigned index = 0;
        for (int i = 0; i < state->field_0; ++i, index += 3, pose += 4)
            CharEntropyDecoder::RetrieveQuaternion(pose, tracks + index);
        for (; index < static_cast<uint32_t>(state->field_8); ++index) *pose++ = tracks[index].whole;
    };
    float frameTime, blend;
    uint32_t current, next;
    anim->ComputeFrameValues(frameTime, next, current, blend, time);
    if (static_cast<int32_t>(current) != state->field_24) {

        if (current + 1 == static_cast<uint32_t>(state->field_24)) {
            std::swap(state->field_10, state->field_14);
        } else {
            uint32_t first;
            if (state->field_24 == -1 || static_cast<int32_t>(current) <= state->field_24) {
                first = 0;
                state->field_1C = CharEntropyDecoder::CharChannelDecoder(stream, false);
            } else first = state->field_24 + 2;
            for (uint32_t frame = first; frame <= current; ++frame) advance(frame);
            retrieve(state->field_10);
        }
        state->field_24 = current;
        if (!next) state->field_1C = CharEntropyDecoder::CharChannelDecoder(stream, false);
        advance(next);
        retrieve(state->field_14);
    }
    auto *pose = state->field_2C ? state->field_30 : static_cast<StdPoseData *>(out);
    auto *dst = reinterpret_cast<float *>(pose->field_10);
    const auto *a = reinterpret_cast<const float *>(state->field_10);
    const auto *b = reinterpret_cast<const float *>(state->field_14);
    for (int i = 0; i < pose->field_4; ++i) {
        const bool quaternion = i < pose->field_0;
        if (mask[i >> 5] & (1u << (i & 31))) {
            if (quaternion) {
                const auto q = math::Slerp(blend, vector4d(a[0], a[1], a[2], a[3]),
                    vector4d(b[0], b[1], b[2], b[3]));
                for (unsigned j = 0; j < 4; ++j) dst[j] = q[j];
                a += 4; b += 4;
            } else {
                for (unsigned j = 0; j < 3; ++j) dst[j] = (b[j] - a[j]) * blend + a[j];
                a += 3; b += 3;
            }
        }
        dst += quaternion ? 4 : 3;
    }
    if (state->field_2C) {
        auto *target = static_cast<StdPoseData *>(out);
        auto *targetQuats = reinterpret_cast<float *>(target->field_10);
        auto *targetPositions = targetQuats + 4 * target->field_0;
        const auto *sourceQuats = reinterpret_cast<const float *>(pose->field_10);
        const auto *sourcePositions = sourceQuats + 4 * pose->field_0;
        for (int i = 0; i < target->field_4; ++i) {
            const int source = state->field_38[2*i];
            if (source == -1) continue;
            const bool animated = state->field_38[2*i+1] != 0;
            if (i < target->field_0) {
                const auto *values = animated ? sourceQuats : reinterpret_cast<const float *>(state->field_34->field_10);
                std::memcpy(targetQuats + 4*i, values + 4*source, 16);
            } else {
                const auto *values = animated ? sourcePositions : reinterpret_cast<const float *>(state->field_34->field_14);
                std::memcpy(targetPositions + 3*(i - target->field_0), values + 3*source, 12);
            }
        }
    }
}

void ArbitraryPOCharComp::_CalcPoseDataRemapped(void *out, uint32_t index, Float time, Float weight,
    const nalComp::nalCompAnim *anim, const void *skel, uint32_t, uint32_t,
    const void *animData, const void *tracks, void *instance)
{
    CalcPoseDataDirect(out, index, time, weight, anim, skel, animData, tracks, instance);
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
