#include "biped_physics.h"

#include "utility.h"

void biped_physics::capture_frame(conglomerate *a1, int a2)
{
    void (*func)(conglomerate *a1, int a2) = CAST(func, 0x00592980);
    func(a1, a2);
}

void biped_physics::set_capture_frame_delta(Float a1)
{
    is_frame_delta = true;
    curr_delta_t = a1;
}
