#pragma once

namespace nalPed {

struct nalPedSkeleton {
    struct vtbl {
        void *field_0;
        void *finalize;
        void *Process;
        void *Release;

        using CheckVersion_t = bool (nalPedSkeleton::*)();
        CheckVersion_t CheckVersion;
    };

    static int &vtbl_ptr;
};

}  // namespace nalPed
