#pragma once

#include "component.h"

struct PanelComponent : BaseComponent {
    PanelComponent *m_prevComp;

    PanelComponent();

    void _SkelPoseRelease(uint32_t, void *, void *) const {}
    void _AnimProcess(uint32_t, void *, void *, const void *) const {}
    void _AnimRelease(uint32_t, void *, void *, const void *) const {}
};

namespace PanelComponentMgr {
extern PanelComponent *&comp_list;

extern void Add(PanelComponent *);
}  // namespace PanelComponentMgr
