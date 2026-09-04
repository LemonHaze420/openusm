#include "biped_system.h"

#include "func_wrapper.h"
#include "phys_vector3d.h"

#include <cassert>

biped_system::biped_system() {}

void biped_system::create_bps(conglomerate *a2, int a3, physical_interface::biped_physics_body_types arg4a)
{
    this->field_2130 = a3;
    this->setup_physics(a2, arg4a);
    this->field_2134 = 0.0;
    this->field_2138 = 0.15000001;
    this->field_0.reset_state_variables();
}

void biped_system::setup_physics(conglomerate *a2, physical_interface::biped_physics_body_types arg4)
{
    THISCALL(0x005A0CB0, this, a2, arg4);
}

void biped_system::prolog_frame_advance(Float a2)
{
    THISCALL(0x0059A790, this, a2);
}
