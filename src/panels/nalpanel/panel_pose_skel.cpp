#include "panel_pose_skel.h"

#include "common.h"
#include "panel_component.h"

#include <nal_system.h>
#include <trace.h>
#include <variables.h>

namespace nalPanel {
VALIDATE_SIZE(nalPanelSkeleton, 0x84);

VALIDATE_SIZE(nalPanelPose, 0x10);

#if !STANDALONE_SYSTEM

int &nalPanelSkeleton::vtbl_ptr = var<int>(0x0096FC74);

#else

int &nalPanelSkeleton::vtbl_ptr = []() -> auto & {
    static nalPanelSkeleton skel{};
    return skel.m_vtbl;
}();

#endif

nalPanelPose::nalPanelPose(const nalPanelSkeleton *a2) : nalCompPose(a2)
{
    field_C = 0;
}

static auto constexpr NAL_PANEL_VERSION = 0x300;

nalPanelSkeleton::nalPanelSkeleton()
{
    if constexpr (1) {
        static void *g_vtbl[]{nullptr,
                              nullptr,
                              func_address(&_Process),
                              nullptr,
                              func_address(&_CheckVersion),
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              func_address(&_UnMash)};

        m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0142B7F8;
    }

    this->m_theDefaultPose = nullptr;
    this->Version = 0x300;
}

void nalPanelSkeleton::_Process()
{
    TRACE("nalPanelSkeleton::Process");

    assert(this->Version == NAL_PANEL_VERSION && "Panel skeleton version mismatch, must be reconverted");

    auto *v1 = PanelComponentMgr::comp_list;
    int num;
    for (num = 0; v1 != nullptr; ++num) {
        v1 = v1->m_prevComp;
    }

    auto *v4 = (BaseComponent **)tlMemAlloc(4 * num, 8, 0);
    int v5 = 0;

    decltype(v4) j;
    for (j = v4; v5 < num; ++v5) {
        auto *v7 = PanelComponentMgr::comp_list;
        for (auto k = v5; k; v7 = v7->m_prevComp) {
            if (v7 == nullptr) {
                break;
            }

            --k;
        }

        j[v5] = v7;
    }

    this->UnMash(this, j, num);
    tlMemFree(j);

    auto *mem = tlMemAlloc(sizeof(nalPanel::nalPanelPose), 8, 0);
    this->m_theDefaultPose = new (mem) nalPanel::nalPanelPose{this};
}
}  // namespace nalPanel
