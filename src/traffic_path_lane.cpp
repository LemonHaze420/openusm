#include "traffic_path_lane.h"

#include "common.h"
#include "func_wrapper.h"
#include "traffic_path.h"
#include "trace.h"
#include "utility.h"
#include "vector3d.h"

VALIDATE_SIZE(traffic_path_lane, 0x14);

bool traffic_path_lane::is_valid(traffic_path_graph *) const
{
    TRACE("traffic_path_lane::is_valid");

    return traffic_path_lane::lane_is_valid(this) && traffic_path_road::road_is_valid(this->my_road);
}

traffic_path_intersection *traffic_path_lane::sub_5C8460()
{
    auto *my_road = this->my_road;
    if (my_road != nullptr) {
        return my_road->field_18;
    } else {
        return nullptr;
    }
}

int traffic_path_lane::get_ai_index(vhandle_type<actor> a2)
{
    auto v2 = this->field_10;
    if (v2 < 0) {
        return -1;
    }

    auto *v3 = &traffic_ai_list::ai_lists[v2];
    if (v3 == nullptr) {
        return -1;
    }

    return v3->get_ai_index(a2);
}

entity_base_vhandle traffic_path_lane::get_ai_by_index(int a3) const
{
    entity_base_vhandle result;
    traffic_ai_list *v4 = nullptr;

    auto v3 = this->field_10;
    if (v3 >= 0 && (v4 = &traffic_ai_list::ai_lists[v3]) != nullptr) {
        if (a3 > 19) {
            result.field_0 = 0;
        } else {
            result = v4->ais[a3].field_0;
        }
    } else {
        result.field_0 = 0;
    }

    return result;
}

int traffic_path_lane::get_num_ais()
{
    traffic_ai_list *v2 = nullptr;
    auto v1 = this->field_10;
    if (v1 >= 0 && (v2 = &traffic_ai_list::ai_lists[v1]) != nullptr) {
        return v2->num_ais;
    } else {
        return 0;
    }
}

int traffic_path_lane::get_type() const
{
    TRACE("traffic_path_lane::get_type");

    if ((this->flags & 2) != 0) {
        return 0;
    } else {
        return ((this->flags & 1) != 0) + 1;
    }
}

bool traffic_path_lane::lane_is_valid(const traffic_path_lane *a1)
{
    return a1 != nullptr && a1->nodes != nullptr && a1->get_num_nodes() > 0 && a1->get_num_nodes() < 64;
}

void traffic_path_lane::seed_with_pedestrians(entity &ent, int a3)
{
    THISCALL(0x005CCFB0, this, &ent, a3);
}

vector3d traffic_path_lane::get_node(int a3) const
{
    auto v3 = a3;
    if (a3 >= 0) {
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

vector3d traffic_path_lane::get_node_before_point(const vector3d &a3, int *a4)
{
    vector3d result;
    THISCALL(0x005C85F0, this, &result, &a3, a4);

    return result;
}

void traffic_path_lane::remove_ai_from_lane(vhandle_type<actor> a2)
{
    THISCALL(0x005C8320, this, a2);
}

void traffic_path_lane::update_lane_indexes()
{
    THISCALL(0x005BFEA0, this);
}

traffic_path_intersection *traffic_path_lane::get_next_intersection(int a2)
{
    assert(this->my_road != nullptr && this->my_road->is_an_in_lane(this));

    auto *my_road = this->my_road;
    if (my_road == nullptr) {
        return nullptr;
    }

    if (a2 > 0) {
        return this->my_road->get_previous_intersection();
    } else {
        return this->my_road->get_next_intersection();
    }
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
