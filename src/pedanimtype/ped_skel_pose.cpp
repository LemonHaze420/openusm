#include "ped_skel_pose.h"

#include "common.h"
#include "utility.h"
#include "matrix4x4.h"
#include "vector3d.h"

#include <cstring>

VALIDATE_SIZE(nalPed::nalPedPose, 0xC0);
VALIDATE_OFFSET(nalPed::nalPedPose, rotation, 0x10);
VALIDATE_OFFSET(nalPed::nalPedPose, arm_rotations, 0x50);
VALIDATE_OFFSET(nalPed::nalPedPose, leg_targets, 0x88);
VALIDATE_OFFSET(nalPed::nalPedPose, trajectory_rotation, 0xA0);
VALIDATE_OFFSET(nalPed::nalPedPose, floor, 0xBC);
VALIDATE_SIZE(nalPed::nalPedSkeleton, 0x2D0);
VALIDATE_OFFSET(nalPed::nalPedSkeleton, default_pose, 0x60);
VALIDATE_OFFSET(nalPed::nalPedSkeleton, spine_offsets, 0x120);
VALIDATE_OFFSET(nalPed::nalPedSkeleton, arms, 0x150);
VALIDATE_OFFSET(nalPed::nalPedSkeleton, legs, 0x1B0);
VALIDATE_OFFSET(nalPed::nalPedSkeleton, bone_indices, 0x210);
VALIDATE_OFFSET(nalPed::nalPedSkeleton, chains, 0x270);

namespace {

void __fastcall ped_empty(nalPed::nalPedSkeleton *, void *) {}
void __fastcall ped_pose_from_matrices(nalPed::nalPedSkeleton *, void *, void *, void *, void *, void *) {}

void *__fastcall destroy_ped_skeleton(nalPed::nalPedSkeleton *self, void *, unsigned int flags)
{
    self->~nalPedSkeleton();
    if (flags & 1)
        tlMemFree(self);
    return self;
}

nalMatrix4x4 pose_matrix(const float *rotation, const nalVector3 &position)
{
    return nalMatrix4x4(nalPositionOrientation(position, rotation));
}

nalMatrix4x4 spine_matrix(float a, float b, const nalVector3 &position)
{
    const float sa = nalPoseSin(a * 0.5f), ca = nalPoseCos(a * 0.5f);
    const float sb = nalPoseSin(b * 0.5f), cb = nalPoseCos(b * 0.5f);
    const float rotation[4]{sa * cb, ca * sb, sa * sb, ca * cb};
    return pose_matrix(rotation, position);
}

nalMatrix4x4 arm_matrix(float twist, float pitch, const float *base, const nalVector3 &position)
{
    const float st = nalPoseSin(twist * 0.5f), ct = nalPoseCos(twist * 0.5f);
    const float sp = nalPoseSin(pitch * 0.5f), cp = nalPoseCos(pitch * 0.5f);
    const float x = st * sp, y = st * cp, z = ct * sp, w = ct * cp;

    const float rotation[4]{base[3] * x + y * base[2] - z * base[1] + base[0] * w,
                            y * base[3] + w * base[1] - base[2] * x + base[0] * z,
                            x * base[1] - y * base[0] + base[3] * z + base[2] * w,
                            base[3] * w - (y * base[1] + x * base[0] + base[2] * z)};
    return pose_matrix(rotation, position);
}

vector3d *__cdecl leg_bend(vector3d *out, matrix4x4 *, matrix4x4 *effector, float x, float y, float z)
{
    out->x = y * (*effector)[1].z - z * (*effector)[1].y;
    out->y = z * (*effector)[1].x - (*effector)[1].z * x;
    out->z = x * (*effector)[1].y - y * (*effector)[1].x;
    return out;
}
}  // namespace

namespace nalPed {
int &nalPedSkeleton::vtbl_ptr = []() -> int & {
    static void *g_vtbl[]{reinterpret_cast<void *>(&ped_empty),
                          reinterpret_cast<void *>(&destroy_ped_skeleton),
                          func_address(&nalPedSkeleton::Process),
                          func_address(&nalPedSkeleton::Release),
                          func_address(&nalPedSkeleton::CheckVersion),
                          func_address(&nalPedSkeleton::GetBoneMatrixCount),
                          func_address(&nalPedSkeleton::GetBoneMatrices),
                          func_address(&nalPedSkeleton::GetTrajectoryUpdate),
                          reinterpret_cast<void *>(&ped_pose_from_matrices),
                          func_address(&nalPedSkeleton::GetDefaultPose),
                          func_address(&nalPedSkeleton::CreatePose),
                          func_address(&nalPedSkeleton::DestroyPose),
                          func_address(&nalPedSkeleton::CopyPose),
                          func_address(&nalPedSkeleton::BlendPose),
                          reinterpret_cast<void *>(&ped_empty)};
    static int g_vtbl_ptr = bit_cast<int>(static_cast<void *>(g_vtbl));
    return g_vtbl_ptr;
}();

void nalPedSkeleton::Process()
{
    default_pose.field_0 = this;
}

void nalPedSkeleton::Release()
{
    default_pose.field_0 = nullptr;
}

bool nalPedSkeleton::CheckVersion() const
{
    return Version == 0x500;
}

int nalPedSkeleton::GetBoneMatrixCount() const
{
    return 21;
}

nalBasePose *nalPedSkeleton::GetDefaultPose()
{
    return &default_pose;
}

nalBasePose *nalPedSkeleton::CreatePose() const
{
    auto *pose = static_cast<nalPedPose *>(tlMemAlloc(sizeof(nalPedPose), 8, 0));
    if (pose != nullptr)
        std::memcpy(pose, &default_pose, sizeof(nalPedPose));
    return pose;
}

void nalPedSkeleton::DestroyPose(nalBasePose *pose) const
{
    tlMemFree(pose);
}

void nalPedSkeleton::CopyPose(nalBasePose *out, const nalBasePose *source) const
{
    std::memcpy(out, source, sizeof(nalPedPose));
}

void nalPedSkeleton::BlendPose(nalBasePose *out, Float, const nalBasePose *, const nalBasePose *b) const
{
    std::memcpy(out, b, sizeof(nalPedPose));
}

void nalPedSkeleton::GetTrajectoryUpdate(const nalBasePose *base, nalPositionOrientation *out) const
{
    const auto *pose = static_cast<const nalPedPose *>(base);
    std::memcpy(out->field_0, pose->trajectory_rotation, sizeof(pose->trajectory_rotation));
    out->field_10 = pose->trajectory_position;
}

void nalPedSkeleton::GetBoneMatrices(const nalBasePose *base, nalMatrix4x4 *matrices)
{
    const auto &pose = *static_cast<const nalPedPose *>(base);
    const auto bone = [&](int index) -> nalMatrix4x4 & {
        return matrices[bone_indices[index]];
    };
    const auto parent = [&](int child, int ancestor) {
        bone(child) = sub_5FE000(bone(child), bone(ancestor));
    };


    bone(0) = pose_matrix(pose.rotation, pose.position);
    bone(0) = bone(0).sub_5EC0A0();
    for (int i = 0; i < 4; ++i) {
        bone(i + 1) = spine_matrix(pose.spine_angles[i][0], pose.spine_angles[i][1], spine_offsets[i]);
        parent(i + 1, i);
    }

    static constexpr float arm_bases[2][4]{
        {0.7369239926338196f, -0.673209011554718f, 0.021265000104904175f, 0.0572660006582737f},
        {0.7369239926338196f, 0.673209011554718f, 0.021265000104904175f, -0.0572660006582737f}};
    for (int i = 0; i < 2; ++i) {
        const int offset = 5 + 4 * i;
        bone(offset) = arm_matrix(pose.arm_angles[i][0], pose.arm_angles[i][1], arm_bases[i], arms[i].twist_offset);
        parent(offset, 2);
        bone(offset + 3) = pose_matrix(pose.arm_rotations[i], pose.arm_positions[i]);
    }


    matrix4x4 identity;
    for (int row = 0; row < 4; ++row)
        identity[row][row] = 1.0f;
    for (int i = 0; i < 2; ++i) {
        const int offset = 5 + 4 * i;
        inverse_kinematics::solve_two_bone(reinterpret_cast<matrix4x4 *>(&bone(offset + 1)),
                                           reinterpret_cast<matrix4x4 *>(&bone(offset + 2)),
                                           &identity,
                                           reinterpret_cast<vector3d *>(&arms[i].root),
                                           reinterpret_cast<matrix4x4 *>(&bone(offset + 3)),
                                           &chains[i].chain,
                                           i == 0 ? inverse_kinematics::compute_arm_elbow_bend_direction
                                                  : inverse_kinematics::compute_arm_elbow_bend_direction_mirrored);
        for (int j = 1; j <= 3; ++j)
            parent(offset + j, offset);
    }

    static constexpr float leg_rotation[4]{0.0f, 0.0f, 1.0f, 0.0f};
    for (int i = 0; i < 2; ++i) {
        const nalVector3 target{{pose.leg_targets[i][0], i == 0 ? 0.1f : -0.1f, pose.leg_targets[i][1]}};
        bone(15 + 4 * i) = pose_matrix(leg_rotation, target);
    }
    for (int i = 0; i < 2; ++i) {
        const int offset = 13 + 4 * i;
        inverse_kinematics::solve_two_bone(reinterpret_cast<matrix4x4 *>(&bone(offset)),
                                           reinterpret_cast<matrix4x4 *>(&bone(offset + 1)),
                                           &identity,
                                           reinterpret_cast<vector3d *>(&legs[i].root),
                                           reinterpret_cast<matrix4x4 *>(&bone(offset + 2)),
                                           &chains[i + 2].chain,
                                           &leg_bend);
        parent(offset, 0);
        parent(offset + 1, 0);
        sub_5F3080(bone(offset + 2), pose.leg_angles[i], legs[i].foot_offset);
        parent(offset + 2, offset + 1);
    }

    static constexpr float toe_rotation[4]{0.0f, 0.70710677f, 0.0f, 0.70710677f};
    for (int i = 0; i < 2; ++i) {
        const int offset = 16 + 4 * i;
        bone(offset) = pose_matrix(toe_rotation, legs[i].toe_offset);
        parent(offset, offset - 1);
    }
}
}  // namespace nalPed
