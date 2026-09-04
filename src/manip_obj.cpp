#include "manip_obj.h"

#include "func_wrapper.h"

manip_obj::manip_obj() {}

void manip_obj::frame_advance_all_manip_objs(Float a1)
{
    CDECL_CALL(0x004DB4D0, a1);
}
