#pragma once

#include "../../nalcomp/nal_anim_comp.h"
#include <nal_anim.h>

namespace nalPanel {

struct nalPanelAnim : nalComp::nalCompAnim {
    uint32_t component_count;
    uint32_t frame_count;

    static int &vtbl_ptr;

    void Process();
    void Release();
    bool CheckVersion() const;
    nalAnimClass<nalAnyPose>::nalInstanceClass *CreateInstance(nalBaseSkeleton *skeleton);
};

}  // namespace nalPanel
