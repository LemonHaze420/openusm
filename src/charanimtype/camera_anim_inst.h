#pragma once

namespace nalCam {

    struct nalCamAnim {
        struct vtbl {
            void *field_0;
            void *finalize;
            void *Process;
            void *Release;

            using CheckVersion_t = bool (nalCamAnim::*)();
            CheckVersion_t CheckVersion;
        };

        static int & vtbl_ptr;
    };
}
