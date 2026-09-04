#pragma once

#include "nal_component.h"

#include "nal_system.h"

struct nalComponentRLE8Int1Data {};

struct nalComponentRLE8Int1 : nalComponent<nalComponentU8Base, nalComponentRLE8Int1Data, nalComponentRLE8Int1> {
    nalComponentRLE8Int1();
};

extern nalComponentRLE8Int1 &Component_nalComponentRLE8Int1;
