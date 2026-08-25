#include "fing52knuckcurlentcompdecomp.h"

#include "character_anim_inst.h"
#include "fing52knuckcurlposedesc.h"
#include "trace.h"

template <>
void Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::RetrievePoseFromInst(
    Fing52KnuckCurlPoseDesc::StdPoseData &a1, Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::PerInstData *a2,
    const Fing52KnuckCurlPoseDesc::PerAnimData *a3)
{
    TRACE("Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::RetrievePoseFromInst");

    if constexpr (0) {
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                Fing52KnuckCurlPoseDesc::StdPoseData *a1,
                                Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::PerInstData *a2,
                                const Fing52KnuckCurlPoseDesc::PerAnimData *a3) = CAST(func, 0x00600880);
        func(this, nullptr, &a1, a2, a3);
    }
}

template <>
void Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::AdvanceAnimDataOneFrame(
    Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::PerInstData *a1,
    const Fing52KnuckCurlPoseDesc::PerAnimData *a2, const nalChar::nalCharAnim *a3, const uint8_t *a4, uint32_t a5)
{
    TRACE("Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::AdvanceAnimDataOneFrame");

    if constexpr (0) {
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::PerInstData *a1,
                                const Fing52KnuckCurlPoseDesc::PerAnimData *a2,
                                const nalChar::nalCharAnim *a3,
                                const uint8_t *a4,
                                uint32_t a5) = CAST(func, 0x00600940);
        func(this, nullptr, a1, a2, a3, a4, a5);
    }
}

template <>
void Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::GetPose(
    Fing52KnuckCurlPoseDesc::StdPoseData *a1, uint32_t a2, Float a3, Float a4, const nalChar::nalCharAnim *a5,
    const Fing52KnuckCurlPoseDesc::PerSkelData *a6, const Fing52KnuckCurlPoseDesc::PerAnimData *a7, const void *a8,
    Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::PerInstData *a9, const Fing52KnuckCurlPoseDesc &a10)
{
    TRACE("Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::GetPose");

    if constexpr (1) {
        auto *v11 = a7;

        uint32_t v3{};
        uint32_t v7{};
        float v18{};
        float v17{};
        a5->ComputeFrameValues(v18, v3, v7, v17, a3);
        auto v12 = a9;
        auto v13 = a9->field_18C;
        int v14 = v7;
        if (int(v7) != v13) {
            if (v13 == -1 || int(v7) == v13 + 1) {
                int v22 = 0;
                if (v13 == -1 || int(v7) <= v13) {
                    v22 = 0;
                    a9->field_180 = CharEntropyDecoder::CharChannelDecoder{a8, 0};
                } else {
                    v22 = v13 + 2;
                }

                for (int i = v22; i <= v14; ++i) {
                    this->AdvanceAnimDataOneFrame(v12, v11, a5, v11->field_4, i);
                }

                v12->field_18C = v14;
                this->RetrievePoseFromInst(v12->field_0, v12, v11);
            } else {
                a9->field_18C = v7;
                v12->field_0 = v12->field_C0;
            }

            auto v16 = v3;
            if (v3 == 0) {
                v12->field_180 = CharEntropyDecoder::CharChannelDecoder{a8, 0};
            }

            this->AdvanceAnimDataOneFrame(v12, v11, a5, v11->field_4, v16);
            this->RetrievePoseFromInst(v12->field_C0, v12, v11);
        }

        a10.BlendPoseDataPartial(a1, a2, v17, &v12->field_0, &v12->field_C0, v11->field_0);
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                Fing52KnuckCurlPoseDesc::StdPoseData *,
                                uint32_t,
                                Float,
                                Float,
                                const nalChar::nalCharAnim *,
                                const Fing52KnuckCurlPoseDesc::PerSkelData *,
                                const Fing52KnuckCurlPoseDesc::PerAnimData *,
                                const void *,
                                Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>::PerInstData *,
                                const Fing52KnuckCurlPoseDesc *) = CAST(func, 0x00600730);
        func(this, nullptr, a1, a2, a3, a4, a5, a6, a7, a8, a9, &a10);
    }
}
