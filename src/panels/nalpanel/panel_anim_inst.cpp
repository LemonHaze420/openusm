#include "panel_anim_inst.h"

#include "common.h"
#include "utility.h"
#include "panel_pose_skel.h"
#include "component.h"
#include "nal_system.h"
#include <cstring>

namespace {
void __fastcall panel_animation_unused(void *, void *) {}
struct panel_instance : nalBaseInstance {
    struct entry {
        uint32_t index;
        void *state;
    };
    entry *entries;
    unsigned count;

    panel_instance(nalPanel::nalPanelAnim *anim, nalPanel::nalPanelSkeleton *skeleton)
        : nalBaseInstance(reinterpret_cast<nalAnimClass<nalAnyPose> *>(anim), skeleton), entries(nullptr), count(0)
    {
        static void *table[]{func_address(&panel_instance::Destroy), func_address(&panel_instance::Pose)};
        m_vtbl = reinterpret_cast<std::intptr_t>(table);
        for (unsigned index = 0; index < static_cast<unsigned>(skeleton->GetNumComponents()); ++index)
            if (skeleton->_DoesComponentHavePoseTrackData(index))
                ++count;
        entries = static_cast<entry *>(tlMemAlloc(count * (sizeof(entry) + 12), 8, 0));
        unsigned current = 0;
        for (unsigned index = 0; index < static_cast<unsigned>(skeleton->GetNumComponents()); ++index) {
            if (!skeleton->_DoesComponentHavePoseTrackData(index))
                continue;
            auto &item = entries[current++];
            item = {index, nullptr};
            if (!anim->DoesComponentAddToPose(index))
                continue;
            auto *component = skeleton->GetComponent(index);
            const auto name = skeleton->GetName(index);
            const void *skel_data = skeleton->GetCompPerSkelDataInt(index);
            const void *default_data = skeleton->GetCompDefaultPoseData(index);
            const void *anim_data = anim->GetCompPerAnimDataInt(index);
            const void *track = anim->GetCompAnimTrackData(index);
            item.state = reinterpret_cast<char *>(entries + count) + (current - 1) * 12;
            component->BuildPerInstData(item.state, name, skel_data, skel_data, default_data, anim_data, track, false);
        }
    }
    void *Destroy(unsigned flags)
    {
        auto *skeleton = static_cast<nalPanel::nalPanelSkeleton *>(field_C);
        auto *anim = reinterpret_cast<nalPanel::nalPanelAnim *>(field_10);
        for (unsigned current = 0; current < count; ++current) {
            auto &item = entries[current];
            if (!item.state)
                continue;
            skeleton->GetComponent(item.index)
                ->DestroyPerInstData(item.state,
                                     skeleton->GetName(item.index),
                                     skeleton->GetCompPerSkelDataInt(item.index),
                                     anim->GetCompPerAnimDataInt(item.index));
        }
        tlMemFree(entries);
        this->~panel_instance();
        if (flags & 1)
            nalAnimClass<nalAnyPose>::nalInstanceClass::operator delete(this);
        return this;
    }
    void Pose(Float time, Float previous, nalBasePose *out, const nalBasePose *reference)
    {
        auto &pose = *reinterpret_cast<nalComp::nalCompPose *>(reinterpret_cast<char *>(out) - 4);
        const auto &source =
            *reinterpret_cast<const nalComp::nalCompPose *>(reinterpret_cast<const char *>(reference) - 4);
        if (source.m_pTheData) {
            if (!pose.m_pTheData)
                pose.AllocPoseData();
            pose.CopyPoseData(source.m_pTheData);
        } else {
            pose.FreePoseData();
            pose.InitializePoseDataFromSkel();
        }
        auto *skeleton = static_cast<nalPanel::nalPanelSkeleton *>(field_C);
        auto *anim = reinterpret_cast<nalPanel::nalPanelAnim *>(field_10);
        for (unsigned current = 0; current < count; ++current) {
            const auto &item = entries[current];
            if (item.state)
                skeleton->GetComponent(item.index)
                    ->CalcPoseDataDirect(pose.GetComponentPoseData(item.index),
                                         skeleton->GetName(item.index),
                                         time,
                                         previous,
                                         anim,
                                         skeleton->GetCompPerSkelDataInt(item.index),
                                         anim->GetCompPerAnimDataInt(item.index),
                                         anim->GetCompAnimTrackData(item.index),
                                         item.state);
        }
    }
};
}

namespace nalPanel {
int &nalPanelAnim::vtbl_ptr = []() -> int & {
    static void *g_vtbl[]{reinterpret_cast<void *>(panel_animation_unused),
                          func_address(&nalPanelAnim::Process),
                          func_address(&nalPanelAnim::Release),
                          func_address(&nalPanelAnim::CheckVersion),
                          func_address(&nalPanelAnim::CreateInstance),
                          func_address(&nalComp::nalCompAnim::_GetPerAnimDataFromComponentIx),
                          func_address(&nalComp::nalCompAnim::_GetPerAnimUserDataInt),
                          func_address(&nalComp::nalCompAnim::_UnMash),
                          func_address(&nalComp::nalCompAnim::_ReMash)};
    static int g_vtbl_ptr = bit_cast<int>(static_cast<void *>(g_vtbl));
    return g_vtbl_ptr;
}();

nalAnimClass<nalAnyPose>::nalInstanceClass *nalPanelAnim::CreateInstance(nalBaseSkeleton *)
{
    return new panel_instance{this, static_cast<nalPanelSkeleton *>(field_30)};
}

void nalPanelAnim::Process()
{
    _UnMash(this);
}

void nalPanelAnim::Release()
{
    _ReMash(this);
}

bool nalPanelAnim::CheckVersion() const
{
    return Version == 0x300;
}
}  // namespace nalPanel
