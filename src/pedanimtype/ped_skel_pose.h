#pragma once

#include "../nal/include/common/nal_skeleton.h"

namespace nalPed {

struct nalPedSkeleton : nalBaseSkeleton {
    static int &vtbl_ptr;

    void Process();
    void Release();
    bool CheckVersion() const;
};

}  // namespace nalPed
