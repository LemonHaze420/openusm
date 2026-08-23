#include "legsikentcompdecomp.h"

#include "character_anim_inst.h"
#include "legsikposedesc.h"
#include "trace.h"
#include "utility.h"

template <>
void LegsIKEntCompDecomp<LegsIKPoseDesc>::AdvanceAnimDataOneFrame(LegsIKEntCompDecomp<LegsIKPoseDesc>::PerInstData *a1,
                                                                  const LegsIKPoseDesc::PerAnimData *a2,
                                                                  const nalChar::nalCharAnim *a3, const uint8_t *a4,
                                                                  uint32_t a5)
{
    TRACE("LegsIKEntCompDecomp<LegsIKPoseDesc>::AdvanceAnimDataOneFrame");

    if constexpr (1) {
        constexpr auto flt_96A698 = 1.0 / 1024.0f;
        auto v22 = flt_96A698 * a3->GetAnimQuantScale();
        auto *v6 = &a1->field_D0;
        CharEntropyQuantConverter::DecodeDequantTracks(
            &a1->field_D0, a4, a1->field_C0, a5, 0, a1->field_C8, v22, a3->IsSceneAnim());
        if (a5 != 0) {
            if (a5 == 1) {
                uint32_t v7 = 0;
                int i{};
                for (i = 0; i < 2; ++i) {
                    if (((1 << i) & a2->field_0) != 0) {
                        CharEntropyQuantConverter::UnEntropyQuaternionTracksInitial(v6, a4, v7);
                        v7 += 3;
                    }
                }

                for (; i < 4; ++i) {
                    if (((1 << i) & a2->field_0) != 0) {
                        CharEntropyQuantConverter::UnEntropyQuaternionTracksInitial(v6, a4, v7);
                        v7 += 3;

                        CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v6, a4, v7++);
                        CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v6, a4, v7++);
                        CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v6, a4, v7);
                        CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v6, a4, v7 + 1);
                        v7 += 2;
                    }
                }
            } else {
                uint32_t v14 = 0;
                int j{};
                for (j = 0; j < 2; ++j) {
                    if (((1 << j) & a2->field_0) != 0) {
                        CharEntropyQuantConverter::UnEntropyQuaternionTracks(v6, a4, v14);
                        v14 += 3;
                    }
                }

                for (; j < 4; ++j) {
                    if (((1 << j) & a2->field_0) != 0) {
                        CharEntropyQuantConverter::UnEntropyQuaternionTracks(v6, a4, v14);
                        v14 += 3;

                        CharEntropyQuantConverter::UnEntropyLinearTrack(v6, a4, v14++);
                        CharEntropyQuantConverter::UnEntropyLinearTrack(v6, a4, v14++);
                        CharEntropyQuantConverter::UnEntropyLinearTrack(v6, a4, v14);
                        CharEntropyQuantConverter::UnEntropyLinearTrack(v6, a4, v14 + 1);
                        v14 += 2;
                    }
                }
            }
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                LegsIKEntCompDecomp<LegsIKPoseDesc>::PerInstData *a1,
                                const LegsIKPoseDesc::PerAnimData *a2,
                                const nalChar::nalCharAnim *a3,
                                const uint8_t *a4,
                                uint32_t a5) = CAST(func, 0x00600060);
        func(this, nullptr, a1, a2, a3, a4, a5);
    }
}

template <>
void LegsIKEntCompDecomp<LegsIKPoseDesc>::RetrievePoseFromInst(LegsIKPoseDesc::StdPoseData &a1,
                                                               LegsIKEntCompDecomp<LegsIKPoseDesc>::PerInstData *a2,
                                                               const LegsIKPoseDesc::PerAnimData *a3)
{
    if constexpr (0) {
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                LegsIKPoseDesc::StdPoseData *a1,
                                LegsIKEntCompDecomp<LegsIKPoseDesc>::PerInstData *a2,
                                const LegsIKPoseDesc::PerAnimData *a3) = CAST(func, 0x005FFE50);
        func(this, nullptr, &a1, a2, a3);
    }
}

template <>
void LegsIKEntCompDecomp<LegsIKPoseDesc>::GetPose(LegsIKPoseDesc::StdPoseData *a2, uint32_t a3, Float a4, Float a5,
                                                  const nalChar::nalCharAnim *a6, const LegsIKPoseDesc::PerSkelData *a7,
                                                  const LegsIKPoseDesc::PerAnimData *a8, const void *a9,
                                                  LegsIKEntCompDecomp<LegsIKPoseDesc>::PerInstData *a10,
                                                  const LegsIKPoseDesc *a11)
{
    TRACE("LegsIKEntCompDecomp<LegsIKPoseDesc>::GetPose");

    if constexpr (1) {
        auto *v11 = a8;
        uint32_t v4 = 0;
        uint32_t v8 = 0;
        float v20 = 0;
        float v19 = 0;
        a6->ComputeFrameValues(v20, v4, v8, v19, a4);
        auto v12 = a10;
        auto v14 = v8;
        if (int(v8) != a10->field_CC) {
            if (a10->field_CC == -1 || int(v8) != a10->field_CC + 1) {
                int v22 = 0;
                if (a10->field_CC == -1 || int(v8) <= a10->field_CC) {
                    v22 = 0;
                    a10->field_C0 = CharEntropyDecoder::CharChannelDecoder{a9, 0};
                } else {
                    v22 = a10->field_CC + 2;
                }

                for (uint32_t i = v22; i <= v14; ++i) {
                    this->AdvanceAnimDataOneFrame(v12, v11, a6, v11->field_4, i);
                }

                v12->field_CC = v14;
                this->RetrievePoseFromInst(v12->field_0, v12, v11);
            } else {
                a10->field_CC = v8;
                v12->field_0 = v12->field_60;
            }

            auto v17 = v4;
            if (v4 == 0) {
                v12->field_C0 = CharEntropyDecoder::CharChannelDecoder{a9, 0};
            }

            this->AdvanceAnimDataOneFrame(v12, v11, a6, v11->field_4, v17);

            this->RetrievePoseFromInst(v12->field_60, v12, v11);
        }

        a11->BlendPoseDataPartial(a2, a3, v19, &v12->field_0, &v12->field_60, v11->field_0);
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                LegsIKPoseDesc::StdPoseData *a2,
                                uint32_t a3,
                                Float a4,
                                Float a5,
                                const nalChar::nalCharAnim *a6,
                                const LegsIKPoseDesc::PerSkelData *a7,
                                const LegsIKPoseDesc::PerAnimData *a8,
                                const void *a9,
                                LegsIKEntCompDecomp<LegsIKPoseDesc>::PerInstData *a10,
                                const LegsIKPoseDesc *a11) = CAST(func, 0x005FE980);

        func(this, nullptr, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
}
