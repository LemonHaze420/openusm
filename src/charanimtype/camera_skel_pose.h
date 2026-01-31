#pragma once

namespace nalCam {

    struct nalCamSkeleton {
        struct vtbl {
            void *field_0;
            void *finalize;
            void *Process;
            void *Release;

            using CheckVersion_t = bool (nalCamSkeleton::*)();
            CheckVersion_t CheckVersion;
        };

        static int & vtbl_ptr;
    };

} // namespace nalCam

