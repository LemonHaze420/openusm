#include "floatsentcompdecomp.h"

#include "character_anim_inst.h"
#include "tentaclesposedesc.h"
#include "variables.h"

template <>
void FloatsEntCompDecomp<TentaclesPoseDesc>::RetrievePoseFromInst(
    TentaclesPoseDesc::StdPoseData *a1, FloatsEntCompDecomp<TentaclesPoseDesc>::PerInstData *a2,
    const TentaclesPoseDesc::PerAnimData *a3)
{
    int v5 = 0;
    for (int i = 0; i < 15; ++i) {
        if ((a3->field_0 & (1 << i)) != 0) {
            a1->field_0[i] = a2->field_88.field_0[v5++][0];
        }
    }
}

template <>
void FloatsEntCompDecomp<TentaclesPoseDesc>::AdvanceAnimDataOneFrame(
    FloatsEntCompDecomp<TentaclesPoseDesc>::PerInstData *a1, const nalChar::nalCharAnim *a2, const uint8_t *a3,
    uint32_t a4)
{
    auto a7 = flt_96A698 * a2->GetAnimQuantScale();
    CharEntropyQuantConverter::DecodeDequantTracks(
        &a1->field_88, a3, a1->field_78, a4, 0, a1->field_80, a7, a2->IsSceneAnim());
    if (a4 != 0) {
        if (a4 == 1) {
            auto *v7 = &a1->field_88;

            for (int v6 = 0; v6 < a1->field_80; ++v6) {
                CharEntropyQuantConverter::UnEntropyLinearTrackInitial(v7, a3, v6);
            }
        } else {
            for (int v6 = 0; v6 < a1->field_80; ++v6) {
                CharEntropyQuantConverter::UnEntropyLinearTrack(&a1->field_88, a3, v6);
            }
        }
    }
}

template <>
void FloatsEntCompDecomp<TentaclesPoseDesc>::GetPose(TentaclesPoseDesc::StdPoseData *a1, uint32_t a2, Float a3, Float,
                                                     const nalChar::nalCharAnim *a5,
                                                     const TentaclesPoseDesc::PerSkelData *,
                                                     const TentaclesPoseDesc::PerAnimData *a7, const void *a8,
                                                     FloatsEntCompDecomp<TentaclesPoseDesc>::PerInstData *a9,
                                                     const TentaclesPoseDesc &a10)
{
    auto *v11 = a7;
    uint32_t v3{};
    uint32_t v7{};
    float v25{};
    float v26{};
    a5->ComputeFrameValues(v26, v3, v7, v25, a3);
    auto *v12 = a9;
    int v13 = a9->field_84;
    int v14 = v7;
    if (int(v7) != v13) {
        if (v13 == -1 || int(v7) != v13 + 1) {
            int v18{};
            if (v13 == -1 || int(v7) <= v13) {
                v18 = 0;
                a9->field_78 = CharEntropyDecoder::CharChannelDecoder{a8, false};
            } else {
                v18 = v13 + 2;
            }

            for (int i = v18; i <= v14; ++i) {
                this->AdvanceAnimDataOneFrame(v12, a5, v11->field_4, i);
            }

            v12->field_84 = v7;
            this->RetrievePoseFromInst(&v12->field_0, v12, a7);
        } else {
            a9->field_84 = v7;
            v12->field_0 = v12->field_3C;
        }

        if (v3 == 0) {
            v12->field_78 = CharEntropyDecoder::CharChannelDecoder{a8, false};
        }

        this->AdvanceAnimDataOneFrame(v12, a5, v11->field_4, v3);
        this->RetrievePoseFromInst(&v12->field_3C, v12, a7);
    }

    a10.BlendPoseDataPartial(a1, a2, v25, &v12->field_0, &v12->field_3C, v11->field_0);
}
