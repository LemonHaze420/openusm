#include "panel_component.h"

#include "common.h"
#include "log.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "variable.h"
#include "variables.h"

VALIDATE_SIZE(PanelComponent, 0x8);

#define create_panel_class(Type, Vtbl)                  \
    struct Panel##Type##Component : PanelComponent {    \
        Panel##Type##Component() {                      \
            if constexpr (1) {                      \
                static void * g_vtbl[] {            \
                    nullptr,                        \
                    func_address(&_GetType),         \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    nullptr,                        \
                    func_address(&_SkelPoseProcess) \
                };                                  \
                m_vtbl = CAST(m_vtbl, &g_vtbl);     \
            } else {                                \
                this->m_vtbl = Vtbl;                   \
            }                                           \
                                                        \
        }                                               \
                                                        \
        uint32_t _GetType() const {                      \
            return to_hash("Panel" #Type);  \
        }                                               \
    };                                                  \
                                                        \
    static Panel##Type##Component g##Type##Component {}



#ifndef STANDALONE_SYSTEM
#error "Not defined macro STANDALONE_SYSTEM"
#endif

#if !STANDALONE_SYSTEM

PanelComponent *& PanelComponentMgr::comp_list = var<PanelComponent *>(0x0096F7DC);

#else

PanelComponent *& PanelComponentMgr::comp_list = []() -> auto & {
    static PanelComponent *g_comp_list {};
    return g_comp_list;
}();

namespace nalPanel {
    create_panel_class(Base, 0x008AA3E0);
    create_panel_class(Scissor, 0x008AA438);
    create_panel_class(Texture, 0x008AA4E8);
    create_panel_class(Gutter, 0x008AA490);
    create_panel_class(Camera, 0x008AA540);
    create_panel_class(Character, 0x008AA648);
    create_panel_class(Color, 0x008AA598);
    create_panel_class(Effect, 0x008AA5F0);
}

#endif

#undef create_panel_class

namespace PanelComponentMgr {

    void Add(PanelComponent *a1) {
        TRACE("PanelComponentMgr::Add");

        a1->m_prevComp = comp_list;
        comp_list = a1;
    }
}

PanelComponent::PanelComponent()
{
    TRACE("PanelComponent()");

    this->m_vtbl = 0x008AA388;

    this->m_prevComp = nullptr;
    PanelComponentMgr::Add(this);
}


