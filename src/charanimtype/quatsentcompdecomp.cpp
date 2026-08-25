#include "quatsentcompdecomp.h"

#include "armstdposedesc.h"
#include "character_anim_inst.h"
#include "common.h"
#include "trace.h"

template <>
void QuatsEntCompDecomp<ArmStdPoseDesc>::RetrievePoseFromInst(ArmStdPoseDesc::StdPoseData &a1,
                                                              QuatsEntCompDecomp<ArmStdPoseDesc>::PerInstData *a2,
                                                              const ArmStdPoseDesc::PerAnimData *a3)
{
    if constexpr (0) {
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                ArmStdPoseDesc::StdPoseData *a1,
                                QuatsEntCompDecomp<ArmStdPoseDesc>::PerInstData *a2,
                                const ArmStdPoseDesc::PerAnimData *a3) = CAST(func, 0x005FFB70);
        func(this, nullptr, &a1, a2, a3);
    }
}


template <>
void QuatsEntCompDecomp<ArmStdPoseDesc>::AdvanceAnimDataOneFrame(QuatsEntCompDecomp<ArmStdPoseDesc>::PerInstData *a1,
                                                                 const nalChar::nalCharAnim *a2, const uint8_t *a3,
                                                                 uint32_t a4)
{
    TRACE("QuatsEntCompDecomp<ArmStdPoseDesc>::AdvanceAnimDataOneFrame");

    if constexpr (0) {
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                QuatsEntCompDecomp<ArmStdPoseDesc>::PerInstData *a1,
                                const nalChar::nalCharAnim *a2,
                                const uint8_t *a3,
                                uint32_t a4) = CAST(func, 0x005FFC10);
        func(this, nullptr, a1, a2, a3, a4);
    }
}

template <>
void QuatsEntCompDecomp<ArmStdPoseDesc>::GetPose(ArmStdPoseDesc::StdPoseData *a1, uint32_t a2, Float a3, Float a4,
                                                 const nalChar::nalCharAnim *a5, const ArmStdPoseDesc::PerSkelData *a6,
                                                 const ArmStdPoseDesc::PerAnimData *a7, const void *a8,
                                                 QuatsEntCompDecomp<ArmStdPoseDesc>::PerInstData *a9,
                                                 const ArmStdPoseDesc &a10)
{
    TRACE("QuatsEntCompDecomp<ArmStdPoseDesc>::GetPose");

    if constexpr (1) {
        uint32_t v19{};
        uint32_t v3{};
        float v21{};
        float v20{};
        a5->ComputeFrameValues(v21, v19, v3, v20, a3);
        auto *v12 = a9;
        auto v13 = a9->field_10C;
        auto v14 = v3;
        if (int(v3) != v13) {
            if (v13 == -1 || int(v3) != v13 + 1) {
                int v16{};
                if (v13 == -1 || int(v3) <= v13) {
                    v16 = 0;
                    a9->field_100 = CharEntropyDecoder::CharChannelDecoder{a8, 0};
                } else {
                    v16 = v13 + 2;
                }

                for (uint32_t i = v16; i <= v14; ++i) {
                    this->AdvanceAnimDataOneFrame(v12, a5, a7->field_4, i);
                }

                v12->field_10C = v14;
                this->RetrievePoseFromInst(v12->field_0, v12, a7);
            } else {
                a9->field_10C = v3;
                v12->field_0 = v12->field_80;
            }

            auto v17 = v19;
            if (v19 == 0) {
                v12->field_100 = CharEntropyDecoder::CharChannelDecoder{a8, 0};
            }

            this->AdvanceAnimDataOneFrame(v12, a5, a7->field_4, v17);
            this->RetrievePoseFromInst(v12->field_80, v12, a7);
        }

        a10.BlendPoseDataPartial(a1, a2, v20, &v12->field_0, &v12->field_80, a7->field_0);
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                ArmStdPoseDesc::StdPoseData *a1,
                                uint32_t a2,
                                Float a3,
                                Float,
                                const nalChar::nalCharAnim *a5,
                                const ArmStdPoseDesc::PerSkelData *,
                                const ArmStdPoseDesc::PerAnimData *a7,
                                const void *a8,
                                QuatsEntCompDecomp<ArmStdPoseDesc>::PerInstData *a9,
                                const ArmStdPoseDesc *a10) = CAST(func, 0x00600260);
        func(this, nullptr, a1, a2, a3, a4, a5, a6, a7, a8, a9, &a10);
    }
}
