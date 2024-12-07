#include "traffic_path_lane.h"

#include "func_wrapper.h"
#include "traffic_path.h"
#include "trace.h"
#include "utility.h"
#include "vector3d.h"

bool traffic_path_lane::is_valid(traffic_path_graph *) const
{
    TRACE("traffic_path_lane::is_valid");

    return traffic_path_lane::lane_is_valid(this)
          && traffic_path_road::road_is_valid(this->my_road);
}

int traffic_path_lane::get_type() const
{
    TRACE("traffic_path_lane::get_type");

    if ( (this->flags & 2) != 0 ) {
        return 0;
    } else {
        return ((this->flags & 1) != 0) + 1;
    }
}

bool traffic_path_lane::lane_is_valid(const traffic_path_lane *a1)
{
    return a1 != nullptr && a1->nodes != nullptr
        && a1->get_num_nodes() > 0 && a1->get_num_nodes() < 64;
}

void traffic_path_lane::seed_with_pedestrians(entity &ent, int a3)
{
    THISCALL(0x005CCFB0, this, &ent, a3);
}

vector3d traffic_path_lane::get_node(int a3) const
{
    auto v3 = a3;
    if (a3 >= 0)
    {
        auto v4 = this->total_nodes;
        if (a3 >= v4) {
            v3 = v4 - 1;
        }

    } else {
        v3 = 0;
    }

    auto v5 = this->nodes[v3];

    return v5;
}

vector3d traffic_path_lane::get_directional_node() const
{
    return this->nodes[this->total_nodes - 1];
}

void traffic_path_lane::sub_5CCFB0(entity *ent, int a3)
{
    TRACE("traffic_path_lane::sub_5CCFB0");

    THISCALL(0x005CCFB0, this, ent, a3);
}

vector3d traffic_path_lane::get_node_before_point(const vector3d &a3, int *a4) {
    vector3d result;
    THISCALL(0x005C85F0, this, &result, &a3, a4);

    return result;
}

void traffic_path_lane_patch()
{
    {
        FUNC_ADDRESS(address, &traffic_path_lane::is_valid);
        //SET_JUMP(0x005C8380, address);
    }

    {
        FUNC_ADDRESS(address, &traffic_path_lane::sub_5CCFB0);
        REDIRECT(0x006D0736, address);
    }
}
