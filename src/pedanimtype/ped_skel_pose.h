#pragma once

#include "../nal/include/common/nal_skeleton.h"
#include "nal_system.h"

namespace nalPed {

struct nalPedPose : nalBasePose {
    nalVector3 position;
    float rotation[4];
    float spine_angles[4][2];
    float arm_angles[2][2];
    float arm_rotations[2][4];
    nalVector3 arm_positions[2];
    float leg_targets[2][2];
    float leg_angles[2];
    float trajectory_rotation[4];
    nalVector3 trajectory_position;
    float floor;
};

struct nalPedSkeleton : nalBaseSkeleton {
    struct ArmData {
        nalVector3 twist_offset;
        nalVector3 root;
        char reserved[0x18];
    };
    struct LegData {
        nalVector3 root;
        char reserved[0xC];
        nalVector3 foot_offset;
        nalVector3 toe_offset;
    };
    struct ChainData {
        inverse_kinematics::ik_bone_chain_t chain;
        float lower_limb_length;
    };

    int field_5C;
    nalPedPose default_pose;
    nalVector3 spine_offsets[4];
    ArmData arms[2];
    LegData legs[2];
    int bone_indices[21];
    char field_264[0xC];
    ChainData chains[4];

    static int &vtbl_ptr;

    void Process();
    void Release();
    bool CheckVersion() const;
    int GetBoneMatrixCount() const;
    void GetBoneMatrices(const nalBasePose *pose, nalMatrix4x4 *matrices);
    void GetTrajectoryUpdate(const nalBasePose *pose, nalPositionOrientation *out) const;
    nalBasePose *GetDefaultPose();
    nalBasePose *CreatePose() const;
    void DestroyPose(nalBasePose *pose) const;
    void CopyPose(nalBasePose *out, const nalBasePose *source) const;
    void BlendPose(nalBasePose *out, Float weight, const nalBasePose *a, const nalBasePose *b) const;
};

}  // namespace nalPed
