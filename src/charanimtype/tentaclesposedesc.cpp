#include "tentaclesposedesc.h"

#include "common.h"
#include "string_hash.h"

VALIDATE_SIZE(TentaclesPoseDesc::StdPoseData, 0x3Cu);

constexpr auto strUpLeftTent = to_hash("UpLeftTent_1");
constexpr auto strUpRightTent = to_hash("UpRightTent_1");
constexpr auto strLowLeftTent = to_hash("LowLeftTent_1");
constexpr auto strLowRightTent = to_hash("LowRightTent_1");
constexpr auto strTongueTent = to_hash("Tongue_1");

int sub_5F0220(uint32_t a1)
{
    switch (a1) {
    case strUpLeftTent:
        return 0;
    case strUpRightTent:
        return 3;
    case strLowLeftTent:
        return 6;
    case strLowRightTent:
        return 9;
    case strTongueTent:
        return 12;
    default:
        return -1;
    }
}

void TentaclesPoseDesc::CopyPoseDataToNothing(TentaclesPoseDesc::StdPoseData *a1, uint32_t,
                                              const TentaclesPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}

float TentaclesPoseDesc::StdPoseData::GetDiameterFromBone(uint32_t a2) const
{
    auto v3 = sub_5F0220(a2);
    if (v3 == -1) {
        return 0.0f;
    } else {
        return this->field_0[v3];
    }
}

float TentaclesPoseDesc::StdPoseData::GetActivityFromBone(uint32_t a2) const
{
    auto v3 = sub_5F0220(a2);
    if (v3 == -1) {
        return 0.0f;
    } else {
        return this->field_4[v3];
    }
}

float TentaclesPoseDesc::StdPoseData::GetPullFromBone(uint32_t a2) const
{
    auto v3 = sub_5F0220(a2);
    return (v3 == -1 ? 0.0f : this->field_8[v3]);
}

void TentaclesPoseDesc::BlendPoseDataPartial(TentaclesPoseDesc::StdPoseData *a1, uint32_t, Float a3,
                                             const TentaclesPoseDesc::StdPoseData *a4,
                                             const TentaclesPoseDesc::StdPoseData *a5, uint32_t a6) const
{
    int v7 = 1;
    for (int i = 0; i < 15; ++i) {
        if ((v7 & a6) != 0) {
            a1->field_0[i] = ((a5->field_0[i] - a4->field_0[i]) * a3) + a4->field_0[i];
        }

        v7 *= 2;
    }
}
