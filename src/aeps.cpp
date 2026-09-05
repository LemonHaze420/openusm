#include "aeps.h"

#include "func_wrapper.h"
#include "game.h"
#include "os_developer_options.h"
#include "trace.h"
#include "utility.h"

Var<void *> aeps::s_activeStructs{0x0095B83C};
#if STANDALONE_SYSTEM
static bool s_enabled{};
#endif

void aeps::FrameAdvance(Float a1)
{
    TRACE("aeps::FrameAdvance");

#if STANDALONE_SYSTEM
    if (!s_enabled) {
        return;
    }
#endif
    CDECL_CALL(0x004D3980, a1);
}

void aeps::FrameSetupRenderAndThenRender()
{
#if STANDALONE_SYSTEM
    if (!s_enabled) {
        return;
    }
#endif
    CDECL_CALL(0x004EA9F0);
}

void aeps::RefreshDevOptions()
{
#if STANDALONE_SYSTEM
    if (!s_enabled) {
        return;
    }
#endif
    CDECL_CALL(0x004CDFC0);
}

void aeps::Reset()
{
    TRACE("aeps::Reset");

#if STANDALONE_SYSTEM
    s_activeStructs() = nullptr;
    return;
#else
    void(__cdecl *func)() = CAST(func, 0x004D91A0);
    func();
#endif
}

void aeps::Init()
{
    TRACE("aeps::Init");

#if STANDALONE_SYSTEM
    s_enabled = false;
    s_activeStructs() = nullptr;
    os_developer_options::instance->set_flag(mString {"NO_PARTICLES"}, true);
#else
    CDECL_CALL(0x004DDDC0);
#endif
}

void aeps_patch()
{
    REDIRECT(0x005584F4, aeps::FrameAdvance);

    REDIRECT(0x005AD2DF, aeps::Init);
}
