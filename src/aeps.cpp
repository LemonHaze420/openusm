#include "aeps.h"

#include "func_wrapper.h"
#include "game.h"
#include "os_developer_options.h"
#include "trace.h"
#include "vtbl.h"

#include "utility.h"

Var<_std::vector<void *> *> aeps::s_activeStructs{0x0095B83C};
#if STANDALONE_SYSTEM
static _std::vector<void *> s_standalone_active_structs;
#endif

void aeps::FrameAdvance(Float a1)
{
    TRACE("aeps::FrameAdvance");

#if STANDALONE_SYSTEM
    struct update_struct {
        std::intptr_t m_vtbl;
        float elapsed;
        float delay;
        float lifetime;
        int field_10;
        bool field_14;
        bool started;
        char padding[2];
        void *target;
    };
    auto *active = s_activeStructs();
    if (active == nullptr) {
        return;
    }
    for (auto iterator = active->end(); iterator != active->begin();) {
        auto current_iterator = --iterator;
        auto *current = static_cast<update_struct *>(*current_iterator);
        if (current == nullptr || current->target == nullptr) {
            continue;
        }
        const auto target_vtbl =
            *static_cast<std::intptr_t *>(current->target);
        if (target_vtbl == 0) {
            continue;
        }
        current->elapsed += a1.value;
        if (current->delay > 0.0f) {
            current->elapsed -= a1.value;
            current->delay -= a1.value;
            if (current->delay > 0.0f) {
                continue;
            }
            current->elapsed -= current->delay;
        }
        if (current->lifetime > 0.0f) {
            current->lifetime -= a1.value;
            if (current->lifetime <= 0.0f) {
                auto finish = get_vfunc(target_vtbl, 0x24);
                if (finish != nullptr) {
                    reinterpret_cast<void(__fastcall *)(void *, void *)>(
                        finish)(current->target, nullptr);
                }
                iterator = active->erase(current_iterator);
                continue;
            }
        }
        const auto offset = current->started ? 0x28 : 0x20;
        current->started = true;
        auto advance = get_vfunc(target_vtbl, offset);
        if (advance != nullptr) {
            reinterpret_cast<void(__fastcall *)(void *, void *)>(
                advance)(current->target, nullptr);
        }
    }
#else
    CDECL_CALL(0x004D3980, a1);
#endif
}

void aeps::FrameSetupRenderAndThenRender()
{
#if STANDALONE_SYSTEM
    return;
#else
    CDECL_CALL(0x004EA9F0);
#endif

}

void aeps::RefreshDevOptions()
{
#if STANDALONE_SYSTEM
    return;
#else
    CDECL_CALL(0x004CDFC0);
#endif

}

void aeps::Reset()
{
    TRACE("aeps::Reset");

#if STANDALONE_SYSTEM
    s_standalone_active_structs.clear();
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
    s_standalone_active_structs.clear();
    s_activeStructs() = &s_standalone_active_structs;
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
