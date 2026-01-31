#pragma once

#include <cstdint>

namespace nalPanel {

struct nalPanelAnim {
    struct vtbl {
        void *field_0;
        void *finalize;
        void *Process;
        void *Release;
        void *nalPanelAnim;
    };

    std::intptr_t m_vtbl;
    uint32_t field_4;

    bool CheckVersion() {
        return this->field_4 == 0x300;
    }

    static int & vtbl_ptr;
};

} // namespace nalPanel
