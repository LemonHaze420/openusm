#include "ped_anim_inst.h"

#include "common.h"
#include "utility.h"
#include "ped_skel_pose.h"
#include "nal_math.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>

VALIDATE_SIZE(nalPed::nalPedPackedPose, 0x30);
VALIDATE_OFFSET(nalPed::nalPedPackedPose, arm_rotations, 0x18);
VALIDATE_OFFSET(nalPed::nalPedPackedPose, arm_positions, 0x20);
VALIDATE_SIZE(nalPed::nalPedAnim, 0x70);
VALIDATE_OFFSET(nalPed::nalPedAnim, trajectory_rotation, 0x40);
VALIDATE_OFFSET(nalPed::nalPedAnim, frame_count, 0x60);
VALIDATE_OFFSET(nalPed::nalPedAnim, frames, 0x6C);
VALIDATE_SIZE(nalPed::nalPedInstance, 0x24);

namespace {

void __fastcall ped_anim_empty(nalPed::nalPedAnim *, void *) {}

void *__fastcall destroy_ped_instance(nalPed::nalPedInstance *self, void *, unsigned int flags)
{
    self->~nalPedInstance();
    if (flags & 1)
        tlMemFree(self);
    return self;
}

vector4d compressed_quaternion(float x, float y, float z)
{
    const float w = static_cast<float>(std::sqrt(std::fabs(
        1.0 - (double(y) * y + double(z) * z + double(x) * x))));
    return vector4d{x, y, z, w};
}

vector4d packed_arm_quaternion(uint32_t packed)
{
    const auto signed_field = [](uint32_t value, unsigned bits) {
        const int32_t sign = 1 << (bits - 1);
        return (static_cast<int32_t>(value) ^ sign) - sign;
    };
    constexpr float scale11 = 0.0009775171056389809f;
    constexpr float scale10 = 0.0019569469150155783f;
    return compressed_quaternion(
        signed_field(packed >> 21, 11) * scale11,
        signed_field((packed >> 11) & 0x3FF, 10) * scale10,
        signed_field(packed & 0x7FF, 11) * scale11);
}

void unpack_pose(nalPed::nalPedPose &pose, const nalPed::nalPedPackedPose &packed)
{
    constexpr float quaternion_scale = 3.0518509447574615e-05f;
    constexpr float position_scale = 0.0010000000474974513f;
    constexpr float half_pi = 1.570796012878418f;
    constexpr float angle_bias = 0.7853981256484985f;
    const auto byte_angle = [](uint8_t value) {
        return static_cast<float>(double(value) * 0.003921568393707275f * half_pi - angle_bias);
    };
    const auto nibble_angle = [](uint8_t value) {
        return static_cast<float>(double(value) * 0.06666667014360428f * half_pi - angle_bias);
    };

    const vector4d root = compressed_quaternion(
        packed.rotation[0] * quaternion_scale, packed.rotation[1] * quaternion_scale,
        packed.rotation[2] * quaternion_scale);


    constexpr float basis = 0.70710677f;
    pose.rotation[0] = static_cast<float>((double(root.z) + root.w) * basis);
    pose.rotation[1] = static_cast<float>((double(root.z) - root.w) * basis);
    pose.rotation[2] = static_cast<float>(-(double(root.x) + root.y) * basis);
    pose.rotation[3] = static_cast<float>((double(root.y) - root.x) * basis);
    for (int i = 0; i < 3; ++i)
        pose.position[i] = packed.position[i] * position_scale;
    for (int i = 0; i < 2; ++i) {
        pose.spine_angles[i + 2][0] = nibble_angle(packed.upper_spine_angles[i] >> 4);
        pose.spine_angles[i + 2][1] = nibble_angle(packed.upper_spine_angles[i] & 0xF);
        for (int j = 0; j < 2; ++j) {
            pose.spine_angles[i][j] = byte_angle(packed.lower_spine_angles[2 * i + j]);
            pose.arm_angles[i][j] = byte_angle(packed.arm_angles[2 * i + j]);
        }
        const vector4d arm_rotation = packed_arm_quaternion(packed.arm_rotations[i]);
        std::memcpy(pose.arm_rotations[i], &arm_rotation, sizeof(arm_rotation));
        for (int j = 0; j < 3; ++j)
            pose.arm_positions[i][j] = packed.arm_positions[i][j] * position_scale;
        for (int j = 0; j < 2; ++j)
            pose.leg_targets[i][j] = packed.leg_targets[i][j] * 0.015748029574751854f;
    }
    pose.leg_angles[0] = nibble_angle(packed.leg_angles >> 4);
    pose.leg_angles[1] = nibble_angle(packed.leg_angles & 0xF);
}
}

namespace nalPed {
int &nalPedAnim::vtbl_ptr = []() -> int & {
    static void *g_vtbl[]{
        reinterpret_cast<void *>(&ped_anim_empty), func_address(&Process), func_address(&Release),
        func_address(&CheckVersion), func_address(&VirtualCreateInstance)};
    static int g_vtbl_ptr = bit_cast<int>(static_cast<void *>(g_vtbl));
    return g_vtbl_ptr;
}();

void nalPedAnim::Process()
{
    auto *base = reinterpret_cast<char *>(this);
    data = has_data != 0 ? base + sizeof(nalPedAnim) : nullptr;
    frames = reinterpret_cast<nalPedPackedPose *>(
        base + sizeof(nalPedAnim) + reinterpret_cast<std::intptr_t>(frames));
}

void nalPedAnim::Release()
{
    auto *base = reinterpret_cast<char *>(this);
    frames = reinterpret_cast<nalPedPackedPose *>(
        reinterpret_cast<std::intptr_t>(frames) - reinterpret_cast<std::intptr_t>(base + sizeof(nalPedAnim)));
    data = nullptr;
}

bool nalPedAnim::CheckVersion() const
{
    return Version == 0x500;
}

nalPedInstance *nalPedAnim::VirtualCreateInstance(nalBaseSkeleton *skeleton)
{
    void *memory = tlMemAlloc(sizeof(nalPedInstance), 8, 0);
    return memory != nullptr ? ::new (memory) nalPedInstance(this, static_cast<nalPedSkeleton *>(skeleton)) :
                               nullptr;
}

uint32_t nalPedAnim::GetPoseFrame(Float time) const
{


    const double time_seconds = double(time.value) * field_38;
    const float frame_time = static_cast<float>(time_seconds);
    const uint32_t intervals = (field_34 & 1) ? frame_count : frame_count - 1;
    const float frames_per_second = static_cast<float>(double(intervals) / field_38);
    const double sample = double(frames_per_second) * frame_time;
    uint32_t frame = static_cast<uint32_t>(std::ceil(time_seconds * frames_per_second));
    if (std::equal_to<double>{}(std::ceil(sample), static_cast<uint32_t>(sample)))
        ++frame;
    if (field_34 & 1)
        return frame % frame_count;
    return std::min(frame, frame_count - 1);
}

nalPedInstance::nalPedInstance(nalPedAnim *anim, nalPedSkeleton *skeleton)
    : nalBaseInstance(anim, skeleton)
{
    static void *table[]{reinterpret_cast<void *>(&destroy_ped_instance), func_address(&GetPose)};
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
    const auto *source = static_cast<const nalPedSkeleton *>(anim->Skeleton);
    for (int i = 0; i < 4; ++i) {
        limb_scales[i] = (skeleton->chains[i].chain.chain_scale + skeleton->chains[i].lower_limb_length) /
                        (source->chains[i].chain.chain_scale + source->chains[i].lower_limb_length);
    }
}

void nalPedInstance::GetPose(Float time, Float previous_time, nalBasePose *base, const nalBasePose *)
{
    const auto *anim = static_cast<const nalPedAnim *>(field_10);
    auto &pose = *static_cast<nalPedPose *>(base);
    unpack_pose(pose, anim->frames[anim->GetPoseFrame(time)]);
    pose.floor = anim->floor;

    float delta = 0.0f;
    if ((anim->field_34 & 1) || (time < 1.0f && previous_time < 1.0f)) {
        delta = time - previous_time;
    } else if (time >= 1.0f) {
        if (previous_time >= 1.0f)
            return;
        delta = 1.0f - previous_time;
    }
    for (int i = 0; i < 3; ++i)
        pose.trajectory_position[i] = delta * anim->trajectory_position[i];
    const vector4d rotation = math::Slerp(delta, vector4d{0.0f, 0.0f, 0.0f, 1.0f},
        vector4d{anim->trajectory_rotation[0], anim->trajectory_rotation[1],
                 anim->trajectory_rotation[2], anim->trajectory_rotation[3]});
    std::memcpy(pose.trajectory_rotation, &rotation, sizeof(rotation));
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j)
            pose.arm_positions[i][j] *= limb_scales[i];
        for (int j = 0; j < 2; ++j)
            pose.leg_targets[i][j] *= limb_scales[i + 2];
    }
}
}  // namespace nalPed
