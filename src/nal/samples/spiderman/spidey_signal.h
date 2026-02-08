#pragma once

#include "nal_component.h"

#include "nal_system.h"

struct spideySignalData {
};


struct spideySignal : nalComponent<nalComponentU8Base, spideySignalData, spideySignal> {

    spideySignal();
};

extern spideySignal & Component_spideySignal;
