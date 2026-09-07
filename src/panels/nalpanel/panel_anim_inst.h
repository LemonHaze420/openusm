#pragma once

#include "../../nalcomp/nal_anim_comp.h"

namespace nalPanel {

struct nalPanelAnim : nalComp::nalCompAnim {
    static int &vtbl_ptr;

    void Process();
    void Release();
    bool CheckVersion() const;
};

}  // namespace nalPanel
