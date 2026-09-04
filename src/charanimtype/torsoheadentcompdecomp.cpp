#include "torsoheadentcompdecomp.h"

#include "character_anim_inst.h"
#include "trace.h"
#include "variables.h"

#include <cmath>

template <>
void TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::RetrievePoseFromInst(
    TorsoHeadStdPoseDesc::StdPoseData &a1, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::PerInstData *a2,
    const TorsoHeadStdPoseDesc::PerAnimData *a3)
{
    TRACE("TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::RetrievePoseFromInst");

    if constexpr (1) {
        int v5 = 0;
        float *v7 = a2->field_F0.field_0[2];

        for (int v6 = 0; v6 < 5; ++v6) {
            if (((1 << v6) & a3->field_0) != 0) {
                auto v8 = *(v7 - 8);
                auto v9 = *(v7 - 4);
                v5 += 3;
                auto v10 = *v7;
                v7 += 12;
                a1.field_0[v6][0] = v8;
                a1.field_0[v6][1] = v9;
                a1.field_0[v6][2] = v10;
                auto v22 = std::sqrt(std::abs(1.0f - (v10 * v10 + v9 * v9 + v8 * v8)));
                a1.field_0[v6][3] = v22;
            }
        }

        if ((a3->field_0 & 0x20) != 0) {
            auto v11 = a2->field_F0.field_0[v5][0];
            auto v12 = a2->field_F0.field_0[v5 + 1][0];
            auto v13 = a2->field_F0.field_0[v5 + 2][0];
            a1.field_50[0] = a2->field_F0.field_0[v5][0];
            a1.field_50[1] = v12;
            a1.field_50[2] = v13;
            auto v23 = std::sqrt(std::abs(1.0f - (v13 * v13 + v12 * v12 + v11 * v11)));
            a1.field_50[3] = v23;

            auto v14 = v5 + 3;
            a1.field_60[0] = a2->field_F0.field_0[v14][0];
            a1.field_60[1] = a2->field_F0.field_0[v14 + 1][0];
            a1.field_60[2] = a2->field_F0.field_0[v14 + 2][0];
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                TorsoHeadStdPoseDesc::StdPoseData *,
                                TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::PerInstData *,
                                const TorsoHeadStdPoseDesc::PerAnimData *) = CAST(func, 0x005FF700);
        func(this, nullptr, &a1, a2, a3);
    }
}

template <>
void TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::AdvanceAnimDataOneFrame(
    TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::PerInstData *a1, const TorsoHeadStdPoseDesc::PerAnimData *a2,
    const nalChar::nalCharAnim *a3, const uint8_t *a4, uint32_t a5)
{
    TRACE("TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::AdvanceAnimDataOneFrame");

    if constexpr (1) {
        auto v22 = flt_96A698 * a3->GetAnimQuantScale();
        auto *v6 = &a1->field_F0;
        CharEntropyQuantConverter::DecodeDequantTracks(
            &a1->field_F0, a4, a1->field_E0, a5, 0, a1->field_E8, v22, a3->IsSceneAnim());
        if (a5 != 0) {
            if (a5 == 1) {
                int v7 = 0;
                for (int i = 0; i < 5; ++i) {
                    if (((1 << i) & a2->field_0) != 0) {
                        CharEntropyQuantConverter::UnEntropyQuaternionTracksInitial(v6, a4, v7);
                        v7 += 3;
                    }
                }

                if ((a2->field_0 & 0x20) != 0) {
                    CharEntropyQuantConverter::UnEntropyQuaternionTracksInitial(v6, a4, v7);
                    auto v9 = v7 + 3;

                    CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v6, a4, v9++);
                    CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v6, a4, v9);
                    CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v6, a4, v9 + 1);
                }
            } else {
                int v11 = 0;
                for (int j = 0; j < 5; ++j) {
                    if (((1 << j) & a2->field_0) != 0) {
                        CharEntropyQuantConverter::UnEntropyQuaternionTracks(v6, a4, v11);
                        v11 += 3;
                    }
                }

                if ((a2->field_0 & 0x20) != 0) {
                    CharEntropyQuantConverter::UnEntropyQuaternionTracks(v6, a4, v11);
                    auto v13 = v11 + 3;

                    CharEntropyQuantConverter::UnEntropyLinearTrack(v6, a4, v13++);
                    CharEntropyQuantConverter::UnEntropyLinearTrack(v6, a4, v13);
                    CharEntropyQuantConverter::UnEntropyLinearTrack(v6, a4, v13 + 1);
                }
            }
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::PerInstData *a1,
                                const TorsoHeadStdPoseDesc::PerAnimData *a2,
                                const nalChar::nalCharAnim *a3,
                                const uint8_t *a4,
                                uint32_t a5) = CAST(func, 0x005FF870);
        func(this, nullptr, a1, a2, a3, a4, a5);
    }
}

template <>
void TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::GetPose(
    TorsoHeadStdPoseDesc::StdPoseData *a2, uint32_t a3, Float a4, Float a5, const nalChar::nalCharAnim *a6,
    const TorsoHeadStdPoseDesc::PerSkelData *a7, const TorsoHeadStdPoseDesc::PerAnimData *a8, const void *a9,
    TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::PerInstData *a10, const TorsoHeadStdPoseDesc &a11)
{
    TRACE("TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::GetPose");

    if constexpr (1) {
        auto *v11 = a8;
        uint32_t v4{};
        uint32_t v8{};
        float v18{};
        float v17{};
        a6->ComputeFrameValues(v18, v4, v8, v17, a4);
        auto *v12 = a10;
        auto v13 = a10->field_EC;
        int v14 = v8;
        if (int(v8) != v13) {
            if (v13 == -1 || int(v8) != v13 + 1) {
                int v18{};
                if (v13 == -1 || int(v8) <= v13) {
                    v18 = 0;
                    a10->field_E0 = CharEntropyDecoder::CharChannelDecoder{a9, 0};
                } else {
                    v18 = v13 + 2;
                }

                for (int i = v18; i <= v14; ++i) {
                    this->AdvanceAnimDataOneFrame(v12, v11, a6, v11->field_4, i);
                }

                v12->field_EC = v14;
                this->RetrievePoseFromInst(v12->field_0, v12, v11);
            } else {
                a10->field_EC = v8;
                v12->field_0 = v12->field_70;
            }

            if (v4 == 0) {
                v12->field_E0 = CharEntropyDecoder::CharChannelDecoder{a9, 0};
            }

            this->AdvanceAnimDataOneFrame(v12, v11, a6, v11->field_4, v4);
            this->RetrievePoseFromInst(v12->field_70, v12, v11);
        }

        a11.BlendPoseDataPartial(a2, a3, v17, &v12->field_0, &v12->field_70, v11->field_0);
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                TorsoHeadStdPoseDesc::StdPoseData *,
                                uint32_t,
                                Float,
                                Float,
                                const nalChar::nalCharAnim *,
                                const TorsoHeadStdPoseDesc::PerSkelData *,
                                const TorsoHeadStdPoseDesc::PerAnimData *,
                                const void *,
                                TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>::PerInstData *,
                                const TorsoHeadStdPoseDesc *) = CAST(func, 0x005FF5B0);
        func(this, nullptr, a2, a3, a4, a5, a6, a7, a8, a9, a10, &a11);
    }
}
