#include "panel_pose_skel.h"

#include "common.h"

#include <nal_system.h>
#include <trace.h>
#include <variables.h>

namespace nalPanel {
VALIDATE_SIZE(nalPanelSkeleton, 0x84);

VALIDATE_SIZE(nalPanelPose, 0x10);

#if !STANDALONE_SYSTEM

int & nalPanelSkeleton::vtbl_ptr = var<int>(0x0096FC74);

#else

int & nalPanelSkeleton::vtbl_ptr = []() -> auto & {
    static nalPanelSkeleton skel {};
    return skel.m_vtbl;
}();

#endif

nalPanelPose::nalPanelPose(const nalPanelSkeleton *a2) : nalCompPose(a2) {
    field_C = 0;
}

nalPanelSkeleton::nalPanelSkeleton()
{
    if constexpr (1) {
        static void * g_vtbl[] {
            nullptr,
            nullptr,
            func_address(&_Process),
            nullptr,
            func_address(&_CheckVersion)
        };

        m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0142B7F8;
    }

    this->m_theDefaultPose = nullptr;
    this->Version = 0x300;
}

void nalPanelSkeleton::_Process() {
    TRACE("nalPanelSkeleton::Process");

    auto *v1 = PanelComponentMgr::comp_list;
    int num;
    for (num = 0; v1 != nullptr; ++num) {
        v1 = (int *) v1[1];
    }

    auto *v4 = (BaseComponent **) tlMemAlloc(4 * num, 8, 0);
    int v5 = 0;

    decltype(v4) j;
    for (j = v4; v5 < num; ++v5)
    {
        auto *v7 = PanelComponentMgr::comp_list;
        for (auto k = v5; k; v7 = (int *) v7[1]) {
            if (v7 == nullptr) {
                break;
            }

            --k;
        }

        j[v5] = (BaseComponent *) v7;
    }

    this->UnMash(this, j, num);
    tlMemFree(j);
    auto *mem = tlMemAlloc(sizeof(nalPanel::nalPanelPose), 8, 0);

    this->m_theDefaultPose = new (mem) nalPanel::nalPanelPose{this};
}
} // namespace nalPanel
