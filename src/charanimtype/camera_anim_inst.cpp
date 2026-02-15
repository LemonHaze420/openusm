#include "camera_anim_inst.h"

#include "common.h"
#include "camera_skel_pose.h"
#include "variable.h"
#include "variables.h"

namespace nalCam {
#if !STANDALONE_SYSTEM
    int & nalCamAnim::vtbl_ptr = var<int>(0x0096AAF4);
#else
    int & nalCamAnim::vtbl_ptr = []() -> auto & {
        static nalCamAnim g_anim {};
        return g_anim.m_vtbl;
    }();
#endif
}

nalCam::nalCamAnim::nalCamAnim()
{
    if constexpr (1) {
        static void * g_vtbl[] {
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            func_address(&_VirtualCreateInstance)
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0089209C;
    }
}

nalCam::nalCamInstance * nalCam::nalCamAnim::CreateInstance(nalCam::nalCamSkeleton *a2)
{
    auto *result = new nalCamInstance(this, a2);
    return result;
}

nalCam::nalCamInstance * nalCam::nalCamAnim::_VirtualCreateInstance(nalBaseSkeleton *a1) {
    return this->CreateInstance(bit_cast<nalCamSkeleton *>(a1));
}

nalCam::nalCamBaseInstance::nalCamBaseInstance(nalAnimClass<nalAnyPose> *a1, nalBaseSkeleton *a2) : nalAnimClass<nalAnyPose>::nalInstanceClass(a1, a2) {
    this->m_vtbl = 0x00891AFC;
}


nalCam::nalCamInstance::nalCamInstance(nalCamAnim *a2, nalCam::nalCamSkeleton *a3) : nalCamBaseInstance(a2, a3)  {
    this->m_vtbl = 0x008920B0;
}
