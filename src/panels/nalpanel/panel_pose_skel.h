#pragma once

#include <nalcomp/nal_pose_comp.h>
#include <nalcomp/nal_anim_comp.h>

namespace nalPanel {

struct nalPanelSkeleton;

struct nalPanelPose : nalComp::nalCompPose {
    nalPanelPose(const nalPanelSkeleton *a2);
};

struct nalPanelSkeleton : nalComp::nalCompSkeleton {

    int field_7C;
    nalPanelPose *m_theDefaultPose;

    nalPanelSkeleton();

    //virtual
    void _Process();

    bool _CheckVersion() const {
        return this->Version == 0x300;
    }

    static int & vtbl_ptr;
};
} // namespace nalPanel
