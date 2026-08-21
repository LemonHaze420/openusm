#include "pc_input_mgr.h"

#include "common.h"
#include "func_wrapper.h"
#include "input.h"
#include "pc_joypad_device.h"
#include "trace.h"
#include "variables.h"

#include <windows.h>

VALIDATE_SIZE(pc_input_mgr, 0x14u);

#if !STANDALONE_SYSTEM

pc_input_mgr *&pc_input_mgr::instance = var<pc_input_mgr *>(0x00967BB0);

#else

pc_input_mgr *&pc_input_mgr::instance = []() -> auto & {
    static pc_input_mgr *g_instance{};
    return g_instance;
}();

#endif

pc_input_mgr::pc_input_mgr()
{
    TRACE("pc_input_mgr::pc_input_mgr");

    if constexpr (1) {
        this->m_vtbl = 0x0088EA7C;

        for (int i{0}; i < 4; ++i) {
            auto *v5 = new pc_joypad_device{i};

            this->pad[i] = v5;
        }
    } else {
        THISCALL(0x0059B930, this);
    }
}

void pc_input_mgr::create_inst()
{
    if constexpr (1) {
        instance = new pc_input_mgr{};
    } else {
        CDECL_CALL(0x005E2C30);
    }
}
