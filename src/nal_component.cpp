#include "nal_component.h"

#include "nal_system.h"
#include "spidey_signal.h"

template<>
nalComponent<nalComponentU8Base, spideySignalData, spideySignal>::nalComponent() {
}

template<>
void nalComponent<nalComponentU8Base, spideySignalData, spideySignal>::_Process(
    const nalGeneric::nalComponentInfo *a1, void *&a2, void *&) {
    for (int i = 0; i < a1->field_28; ++i) {
        a2 = static_cast<char *>(a2) + 1;
    }
}
