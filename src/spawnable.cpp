#include "spawnable.h"

#include "actor.h"
#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

VALIDATE_SIZE(spawnable, 0x8);

traffic_path_graph::laneInfoStruct *& spawnable::last_spawn_lane_info = var<traffic_path_graph::laneInfoStruct *>(0x0096C9B4);

spawnable::spawnable(vhandle_type<entity> )
{
    this->m_vtbl = 0x008A59FC;
    if ( spawnable_lanes() == nullptr )
    {
        spawnable_lanes() = new _std::vector<traffic_path_graph::laneInfoStruct> {};
        spawnable_lanes()->reserve(30u);
    }

    this->field_4 = true;
    this->field_5 = true;
}

void spawnable::advance_traffic_and_peds(Float a1) {
    TRACE("spawnable::advance_traffic_and_peds");

    CDECL_CALL(0x006D8610, a1);
}

void spawnable::do_spawn(
        vector3d a4,
        vector3d a2,
        traffic_path_lane *lane,
        int a9,
        bool a10,
        bool a11)
{
    void (__fastcall *func)(void *, void *edx,
            vector3d, vector3d, traffic_path_lane *,
            int, bool, bool) = CAST(func, get_vfunc(m_vtbl, 0x0));
    func(this, nullptr, a4, a2, lane, a9, a10, a11);
}

void spawnable::un_spawn()
{
    void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this);
}

bool spawnable::is_viable_pos(const vector3d &a2)
{
    bool (__fastcall *func)(void *, void *edx, const vector3d *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this, nullptr, &a2);
}

actor *spawnable::get_my_actor()
{

    actor * (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    return func(this);
}

vector3d spawnable::prepare_for_spawn(
        traffic_path_graph::laneInfoStruct *next_lane_struct,
        vector3d &a4,
        int a5)
{
    vector3d result = ZEROVEC;

    assert(next_lane_struct != nullptr);
    auto *v9 = this->get_my_actor();
    if ( v9->sub_48AE20())
    {
        vector3d v45 {};
        auto *the_lane = next_lane_struct->field_4;
        if ( the_lane->get_type() )
        {
            auto v12 = []() {
                return rand() * 0.000030518509;
            }();

            if ( v12 >= 0.5f )
            {
                auto node = the_lane->get_node(1);
                auto v9 = the_lane->get_node(0);
                v45 = v9 - node;
                result = the_lane->get_node(0);
            }
            else
            {
                auto node = the_lane->get_node(0);
                auto v6 = the_lane->get_node(1);
                v45 = v6 - node;
                result = the_lane->get_node(1);
            }
        }
        else
        {
            auto num_nodes = the_lane->get_num_nodes();
            auto node = the_lane->get_node(num_nodes - 2);
            auto directional_node = the_lane->get_directional_node();
            v45 = directional_node - node;
            result = the_lane->get_directional_node();
        }

        v45.normalize();
        spawnable::last_spawn_lane_info = next_lane_struct;
        if ( this->is_viable_pos(a4) ) {
            this->do_spawn(a4, v45, the_lane, a5, next_lane_struct->field_E, 1);
        }
    }

    return result;
}


void spawnable_patch()
{
    {
        REDIRECT(0x0055842F, spawnable::advance_traffic_and_peds);
    }
}
