#pragma once

namespace nalPed {

    struct nalPedAnim {
        struct vtbl {
            void *field_0;
            void *finalize;
            void *Process;
            void *Release;

            using CheckVersion_t = bool (nalPedAnim::*)();
            CheckVersion_t CheckVersion;
        };

        static int & vtbl_ptr;
    };

}
