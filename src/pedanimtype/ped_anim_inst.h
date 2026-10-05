#pragma once

#include "../nal/include/common/nal_anim.h"
#include <cstdint>

namespace nalPed {

struct nalPedSkeleton;
struct nalPedPose;
struct nalPedInstance;

struct nalPedPackedPose {
    int16_t rotation[3];
    uint8_t upper_spine_angles[2];
    uint8_t lower_spine_angles[4];
    uint8_t arm_angles[4];
    int16_t position[3];
    uint8_t leg_angles;
    uint8_t reserved;
    uint32_t arm_rotations[2];
    int16_t arm_positions[2][3];
    int8_t leg_targets[2][2];
};

struct nalPedAnim : nalAnimClass<nalAnyPose> {
    float trajectory_rotation[4];
    float trajectory_position[3];
    float floor;
    uint32_t frame_count;
    int has_data;
    void *data;
    nalPedPackedPose *frames;

    static int &vtbl_ptr;

    void Process();
    void Release();
    bool CheckVersion() const;
    nalPedInstance *VirtualCreateInstance(nalBaseSkeleton *skeleton);
    uint32_t GetPoseFrame(Float time) const;
};

struct nalPedInstance : nalBaseInstance {
    float limb_scales[4];

    nalPedInstance(nalPedAnim *anim, nalPedSkeleton *skeleton);
    void GetPose(Float time, Float previous_time, nalBasePose *pose, const nalBasePose *reference);
};

}  // namespace nalPed
