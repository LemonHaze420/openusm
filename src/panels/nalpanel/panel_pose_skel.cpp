#include "panel_pose_skel.h"

#include "common.h"
#include "panel_component.h"
#include "func_wrapper.h"

#include <nal_system.h>
#include <trace.h>
#include <variables.h>

namespace nalPanel {
VALIDATE_SIZE(nalPanelSkeleton, 0x84);

VALIDATE_SIZE(nalPanelPose, 0x10);

namespace {
void __fastcall skeleton_empty(nalPanelSkeleton *, void *) {}
void *__fastcall skeleton_destroy(nalPanelSkeleton *self, void *, unsigned flags)
{
    if (self->m_theDefaultPose) {
        self->m_theDefaultPose->FreePoseData();
        tlMemFree(self->m_theDefaultPose);
        self->m_theDefaultPose = nullptr;
    }
    self->~nalPanelSkeleton();
    if (flags & 1)
        tlMemFree(self);
    return self;
}
unsigned __fastcall skeleton_bone_count(nalPanelSkeleton *, void *)
{
    return 0;
}
void __fastcall skeleton_bones(nalPanelSkeleton *, void *, const nalBasePose *, nalMatrix4x4 *) {}
void __fastcall skeleton_trajectory(nalPanelSkeleton *, void *, const nalBasePose *, nalPositionOrientation *out)
{
    *out = nalPositionOrientation::Identity;
}
nalBasePose *__fastcall skeleton_default(nalPanelSkeleton *self, void *)
{
    return self->m_theDefaultPose ? reinterpret_cast<nalBasePose *>(&self->m_theDefaultPose->field_4) : nullptr;
}
nalBasePose *__fastcall skeleton_create(const nalPanelSkeleton *self, void *)
{
    auto *pose = new (tlMemAlloc(sizeof(nalPanelPose), 8, 0)) nalPanelPose{self};
    return reinterpret_cast<nalBasePose *>(&pose->field_4);
}
void __fastcall skeleton_destroy_pose(nalPanelSkeleton *, void *, nalBasePose *value)
{
    if (!value || reinterpret_cast<std::uintptr_t>(value) == 4)
        return;
    auto *pose = reinterpret_cast<nalPanelPose *>(reinterpret_cast<char *>(value) - 4);
    pose->FreePoseData();
    pose->~nalPanelPose();
    tlMemFree(pose);
}
void __fastcall skeleton_copy(nalPanelSkeleton *, void *, nalBasePose *out, const nalBasePose *source)
{
    auto *destination = reinterpret_cast<nalPanelPose *>(reinterpret_cast<char *>(out) - 4);
    const auto *reference = reinterpret_cast<const nalPanelPose *>(reinterpret_cast<const char *>(source) - 4);
    *destination = *reference;
}
void __fastcall skeleton_blend(nalPanelSkeleton *self, void *, nalBasePose *out, Float, const nalBasePose *,
                               const nalBasePose *source)
{
    skeleton_copy(self, nullptr, out, source);
}
void __fastcall skeleton_blend_direct(nalPanelSkeleton *self, void *, nalBasePose *out, Float time,
                                      const nalBasePose *left, const nalBasePose *right)
{
    skeleton_blend(self, nullptr, out, time, left, right);
}
}

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
    static void *g_vtbl[]{reinterpret_cast<void *>(skeleton_empty),
                          reinterpret_cast<void *>(skeleton_destroy),
                          func_address(&nalPanelSkeleton::_Process),
                          func_address(&nalPanelSkeleton::_Release),
                          func_address(&nalPanelSkeleton::_CheckVersion),
                          reinterpret_cast<void *>(skeleton_bone_count),
                          reinterpret_cast<void *>(skeleton_bones),
                          reinterpret_cast<void *>(skeleton_trajectory),
                          reinterpret_cast<void *>(skeleton_blend_direct),
                          reinterpret_cast<void *>(skeleton_default),
                          reinterpret_cast<void *>(skeleton_create),
                          reinterpret_cast<void *>(skeleton_destroy_pose),
                          reinterpret_cast<void *>(skeleton_copy),
                          reinterpret_cast<void *>(skeleton_blend),
                          func_address(&nalCompSkeleton::_GetPerSkelDataFromComponent),
                          func_address(&nalCompSkeleton::_DoesComponentHavePoseTrackData),
                          func_address(&nalCompSkeleton::_UnMash),
                          func_address(&nalCompSkeleton::ReMash)};
    m_vtbl = reinterpret_cast<std::intptr_t>(g_vtbl);

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

void nalPanelSkeleton::_Release()
{
    if constexpr (STANDALONE_SYSTEM) {
        ReMash(this);
        if (m_theDefaultPose != nullptr) {
            m_theDefaultPose->FreePoseData();
            tlMemFree(m_theDefaultPose);
        }
        m_theDefaultPose = nullptr;
    } else {
        THISCALL(0x007348E0, this);
    }
}
}  // namespace nalPanel
