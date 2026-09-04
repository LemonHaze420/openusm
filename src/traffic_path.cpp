#include "traffic_path.h"

#include "common.h"

traffic_ai_list (&traffic_ai_list::ai_lists)[1] = var<traffic_ai_list[1]>(0x00968690);

bool traffic_path_intersection::has_stopsign(bool)
{
    return false;
}

bool traffic_path_road::is_an_in_lane(const traffic_path_lane *a2) const
{
    for (auto i = 0u; i < this->total_in_lanes; ++i) {
        if (this->in_lanes[i] == a2) {
            return true;
        }
    }

    return false;
}

bool traffic_path_road::road_is_valid(const traffic_path_road *a1)
{
    return a1 != nullptr && a1->total_in_lanes <= 7u && a1->total_out_lanes <= 7u;
}

int traffic_ai_list::get_ai_index(vhandle_type<actor> me)
{
    assert(me != INVALID_VHANDLE);

    for (int i = 0; i < this->num_ais; ++i) {
        if (this->ais[i] == me) {
            assert(this->ais[i].get_volatile_ptr()->is_an_actor());
            return i;
        }
    }

    return -1;
}
