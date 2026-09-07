#pragma once

#include "../nal/include/common/nal_anim.h"

namespace nalPed {

struct nalPedAnim : nalAnimClass<nalAnyPose> {
    static int &vtbl_ptr;

    void Process();
    void Release();
    bool CheckVersion() const;
};

}  // namespace nalPed
