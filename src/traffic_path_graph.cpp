#include "traffic_path_graph.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(traffic_path_graph::laneInfoStruct, 0x10);

traffic_path_graph::traffic_path_graph() {}

traffic_path_lane *traffic_path_graph::get_closest_or_farthest_lane(bool arg0, const vector3d &a1, const vector3d &arg8,
                                                                    vector3d *a5, traffic_path_lane::eLaneType a6,
                                                                    bool a7, float *a8)
{
    return (traffic_path_lane *)THISCALL(0x005CE2D0, this, arg0, &a1, &arg8, a5, a6, a7, a8);
}

bool traffic_path_graph::un_mash(char *a2, int *a3, region *a4, traffic_path_brew &a5)
{
    TRACE("traffic_path_graph::un_mash");

    if constexpr (0) {
    } else {
        bool(__fastcall * func)(void *, int, char *a2, int *a3, region *a4, traffic_path_brew *a5) =
            CAST(func, 0x005C7E20);
        return func(this, 0, a2, a3, a4, &a5);
    }
}

void traffic_path_graph::release_mem()
{
    THISCALL(0x005D5CB0, this);
}
