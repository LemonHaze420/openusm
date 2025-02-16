#pragma once

#include <nalcomp/nal_pose_comp.h>
#include <nalcomp/nal_anim_comp.h>

namespace nalPanel {

struct nalPanelSkeleton;

struct nalPanelPose : nalComp::nalCompPose {
    nalPanelPose(const nalPanelSkeleton *a2);
};

struct nalPanelSkeleton : nalComp::nalCompSkeleton {
    struct vtbl {
        void *field_0;
        void *finalize;
        void (nalPanelSkeleton::*Process)();
        void *Release;
        bool (nalPanelSkeleton::*CheckVersion)();
    };

    int field_7C;
    nalPanelPose *field_80;

    void Process();

    bool CheckVersion() {
        return this->Version == 0x300;
    }

    static int vtbl_ptr;
};
} // namespace nalPanel
