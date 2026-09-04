#pragma once

#include "component.h"

struct PanelComponent : BaseComponent {
    PanelComponent *m_prevComp;

    PanelComponent();
};

namespace PanelComponentMgr {
extern PanelComponent *&comp_list;

extern void Add(PanelComponent *);
}  // namespace PanelComponentMgr
