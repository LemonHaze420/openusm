#pragma once

#include "fixed_vector.h"

#include "variable.h"

struct region;

namespace nearby_hero_regions {

extern Var<fixed_vector<region *, 15>> regs;

void update();
}
