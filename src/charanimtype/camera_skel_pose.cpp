#include "camera_skel_pose.h"

#include "common.h"
#include "variable.h"
#include "variables.h"
#include "nal_instance.h"
#include <cstring>

#if STANDALONE_SYSTEM
namespace {
void __fastcall camera_skeleton_unused(void *, void *) {}
void *__fastcall camera_skeleton_destroy(nalCam::nalCamSkeleton *self, void *, unsigned flags)
{
    if (flags & 1)
        tlMemFree(self);
    return self;
}
int __fastcall camera_bone_count(void *, void *)
{
    return 0;
}
void __fastcall camera_bones(void *, void *, const void *, void *) {}
void __fastcall camera_trajectory(void *, void *, const nalCam::nalCamPose *pose, nalPositionOrientation *out)
{
    std::memcpy(out, pose->field_4 + 12, 28);
}
void __fastcall camera_pose_mask(void *, void *, int, int, int, int) {}
nalCam::nalCamPose *__fastcall camera_default(nalCam::nalCamSkeleton *self, void *)
{
    return &self->field_60;
}
nalCam::nalCamPose *__fastcall camera_pose_create(nalCam::nalCamSkeleton *self, void *)
{
    auto *pose = static_cast<nalCam::nalCamPose *>(tlMemAlloc(sizeof(nalCam::nalCamPose), 8, 0));
    if (pose)
        *pose = self->field_60;
    return pose;
}
void __fastcall camera_pose_destroy(void *, void *, void *pose)
{
    tlMemFree(pose);
}
void __fastcall camera_pose_copy(void *, void *, nalCam::nalCamPose *out, const nalCam::nalCamPose *src)
{
    *out = *src;
}
void __fastcall camera_pose_blend(void *, void *, nalCam::nalCamPose *out, Float, const void *,
                                  const nalCam::nalCamPose *src)
{
    *out = *src;
}
void __fastcall camera_skeleton_unmash(nalCam::nalCamSkeleton *self, void *)
{
    self->field_48 += reinterpret_cast<int>(&self->field_50);
    self->field_4C = reinterpret_cast<int>(&self->field_50);
}
void __fastcall camera_skeleton_remash(nalCam::nalCamSkeleton *self, void *)
{
    self->field_48 -= reinterpret_cast<int>(&self->field_50);
    self->field_4C = 0;
}
}
#endif

namespace nalCam {

VALIDATE_SIZE(nalCamPose, 0x30u);

#if !STANDALONE_SYSTEM
int &nalCamSkeleton::vtbl_ptr = var<int>(0x0096A780);
#else
int &nalCamSkeleton::vtbl_ptr = []() -> auto & {
    static nalCamSkeleton cam{};
    return cam.m_vtbl;
}();
#endif
}  // namespace nalCam

nalCam::nalCamSkeleton::nalCamSkeleton()
{
    if constexpr (1) {
#if STANDALONE_SYSTEM
        static void *g_vtbl[]{reinterpret_cast<void *>(camera_skeleton_unused),
                              reinterpret_cast<void *>(camera_skeleton_destroy),
                              func_address(&nalCamSkeleton::_Process),
                              func_address(&nalCamSkeleton::_Release),
                              func_address(&nalCamSkeleton::_CheckVersion),
                              reinterpret_cast<void *>(camera_bone_count),
                              reinterpret_cast<void *>(camera_bones),
                              reinterpret_cast<void *>(camera_trajectory),
                              reinterpret_cast<void *>(camera_pose_mask),
                              reinterpret_cast<void *>(camera_default),
                              reinterpret_cast<void *>(camera_pose_create),
                              reinterpret_cast<void *>(camera_pose_destroy),
                              reinterpret_cast<void *>(camera_pose_copy),
                              reinterpret_cast<void *>(camera_pose_blend),
                              reinterpret_cast<void *>(camera_skeleton_unused),
                              reinterpret_cast<void *>(camera_skeleton_unmash),
                              reinterpret_cast<void *>(camera_skeleton_remash),
                              func_address(&nalCamSkeleton::_CheckVersion)};
#else
        static void *g_vtbl[]{nullptr,
                              nullptr,
                              func_address(&nalCamSkeleton::_Process),
                              func_address(&nalCamSkeleton::_Release),
                              func_address(&nalCamSkeleton::_CheckVersion)};
#endif
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00892064;
    }
}

void nalCam::nalCamSkeleton::_Process()
{
    this->field_60.field_0 = this;
}


void nalCam::nalCamSkeleton::_Release()
{
    this->field_60.field_0 = nullptr;
}

bool nalCam::nalCamSkeleton::_CheckVersion()
{
    return this->Version == 0x10000;
}
