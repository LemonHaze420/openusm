#include "aeps.h"

#include "func_wrapper.h"
#include "game.h"
#include "os_developer_options.h"
#include "trace.h"
#include "utility.h"

Var<void *> aeps::s_activeStructs{0x0095B83C};

void aeps::FrameAdvance(Float a1)
{
    TRACE("aeps::FrameAdvance");

    if constexpr (0) {
#if 0
        int v1;   // eax
        char *v2; // esi

        if (!os_developer_options::instance()->get_flag(46) && //NO_PARTICLE_PUMP
            !os_developer_options::instance()->get_flag(45)) { //NO_PARTICLES
            Var<int> devCounter{0x00921B18};
            if (!(devCounter() % 0x3Cu)) {
                ++devCounter();
                aeps::RefreshDevOptions();
            }

            Var<int *> dword_937B1C{0x00937B1C};

            if (!g_game_ptr()->field_16C ||
                (v1 = *(DWORD *) (dword_937B1C() + 12), v1 >= 0) && v1 == 10) {
                Var<void *> dword_95B840{0x0095B840};
                v2 = (char *) dword_95B840();
                if (dword_95B840() != aeps::s_activeStructs()) {
                    do {
                        (*(void(__stdcall **)(_DWORD, _DWORD))(**((_DWORD **) v2 - 1) +
                                                               4))(LODWORD(a1), 0);
                        v2 -= 4;
                    } while (v2 != aeps::s_activeStructs());
                }
            }
        }

#endif

    } else {
        CDECL_CALL(0x004D3980, a1);
    }
}

void aeps::FrameSetupRenderAndThenRender()
{
    CDECL_CALL(0x004EA9F0);
}

void aeps::RefreshDevOptions()
{
    CDECL_CALL(0x004CDFC0);
}

void aeps::Reset()
{
    TRACE("aeps::Reset");

    void(__cdecl * func)() = CAST(func, 0x004D91A0);
    func();
}

void aeps::Init()
{
    TRACE("aeps::Init");

    CDECL_CALL(0x004DDDC0);
}

void aeps_patch()
{
    REDIRECT(0x005584F4, aeps::FrameAdvance);

    REDIRECT(0x005AD2DF, aeps::Init);
}
