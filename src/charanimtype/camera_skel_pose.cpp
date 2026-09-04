#include "camera_skel_pose.h"

#include "common.h"
#include "variable.h"
#include "variables.h"

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
        static void *g_vtbl[]{nullptr, nullptr, func_address(&_Process), nullptr, func_address(&_CheckVersion)};
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
