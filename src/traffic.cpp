#include "traffic.h"

#include "actor.h"
#include "common.h"
#include "func_wrapper.h"
#include "region.h"
#include "terrain.h"
#include "trace.h"
#include "traffic_path_graph.h"
#include "traffic_path_lane.h"
#include "vehicle.h"
#include "wds.h"

VALIDATE_SIZE(traffic, 0x22Cu);

void traffic::sub_6DA3B0(Float a2, Float a3, Float a4) {
    auto &v4 = this->field_14C;
    v4[0] = a2;
    v4[1] = a3;
    v4[2] = a4;
}

void traffic::initialize_traffic()
{
    CDECL_CALL(0x006D1CC0);
}

void sub_6BA2A0(vhandle_type<entity> a1, uint32_t a2)
{
    if ( a2 <= 7 )
    {
        auto *v2 = vehicle::models()[a2];
        if ( v2 != nullptr )
        {
            v2->sub_6B9F30(a1);
        }
    }
}

//FIXME
void traffic::terminate_traffic()
{
    if ( !traffic_initialized() ) {
        return;
    }

    bool v13 = false;
    bool v14 = true;

    while (v14)
    {
        v14 = false;
        for ( auto &the_traffic : traffic_list() )
        {
            if ( the_traffic != nullptr )
            {
                if ( the_traffic->field_4 && the_traffic->field_158 )
                {
                    if ( !the_traffic->field_5 )
                    {
                        if ( the_traffic->get_current_lane() != nullptr) {
                            the_traffic->un_spawn();
                        }
                    }

                    the_traffic->field_158 = false;
                    auto *e = the_traffic->get_my_actor();
                    auto bodytype = the_traffic->field_C.bodytype;
                    the_traffic = nullptr;
                    assert(e != nullptr);

                    if ( e != nullptr ) {
                        sub_6BA2A0(vhandle_type<entity> {e->get_my_handle()}, bodytype);
                    }

                    v14 = true;
                    continue;
                }

                sp_log("traffic was disabled with mission cars in world - this may cause problems");
                v13 = true;
            }
        }
    }

    if ( !v13 )
    {
        vehicle::terminate_vehicles();
        traffic_list().clear();
    }

    traffic_initialized() = false;
}

void traffic::enable_traffic(bool a1, bool a2)
{
    TRACE("traffic::enable_traffic");

    if constexpr (1)
    {
        traffic_enabled() = a1;
        if ( a1 && !traffic_initialized() ) {
            initialize_traffic();
        }

        if (!a1 && traffic_initialized() ) {
            terminate_traffic();
        }
    }
    else
    {
        CDECL_CALL(0x006D33C0, a1, a2);
    }
}

void traffic::get_closest_point_on_lane_with_facing(vector3d *a1, vector3d *a2, bool a3)
{
    if constexpr (1)
    {
        auto *v3 = g_world_ptr->the_terrain->get_region_info_for_point(*a1);
        if (v3 != nullptr)
        {
            for ( auto &reg : (*v3) )
            {
                auto *v6 = reg->get_traffic_path_graph();
                if ( reg->is_interior()) {
                    continue;
                }

                if (v6 != nullptr)
                {
                    vector3d a5;
                    auto *lane = v6->get_closest_or_farthest_lane(true,
                                                          *a1,
                                                          ZEROVEC,
                                                          &a5,
                                                          static_cast<traffic_path_lane::eLaneType>(
                                                              static_cast<int>(!a3)),
                                                          false,
                                                          nullptr);
                    if (lane != nullptr)
                    {
                        int idx = 0;
                        lane->get_node_before_point(a5, &idx);

                        vector3d node1 = lane->get_node(idx);
                        vector3d node2 = lane->get_node(idx + 1);

                        if (idx < 1 || idx < lane->get_num_nodes() - 2)
                        {
                            node1 = lane->get_node(idx);
                            node2 = lane->get_node(idx + 1);
                        }
                        else
                        {
                            node1 = lane->get_node(idx - 1);
                            node2 = lane->get_node(idx);
                        }

                        assert((node2 - node1).xz_length2() > EPSILON);

                        auto v17 = (node2 - node1).normalized();

                        *a2 = v17;
                        *a1 = a5;
                        return;
                    }
                }
            }
        }

        sp_log("Something went wrong with the get_closest_point_on_lane_with_facing() function");
    }
    else
    {
        CDECL_CALL(0x006D0910, a1, a2, a3);
    }
}

bool traffic::is_unanimated_car(actor *a1) {
    if constexpr (1) {

        bool result = false;
        if ((a1->field_4 & 0x800) != 0) {
            auto *v1 = traffic::get_traffic_from_entity({a1->my_handle});
            if (v1 == nullptr || !v1->field_202) {
                result = true;
            }
        }

        return result;
    } else {
        return (bool) CDECL_CALL(0x004ADE20, a1);
    }
}

void traffic::_do_spawn(
        vector3d a4,
        vector3d a2,
        traffic_path_lane *lane,
        int a9,
        bool a10,
        bool a11)
{
    TRACE("traffic::do_spawn");
    void (__fastcall *func)(traffic *, void *edx,
            vector3d, vector3d, traffic_path_lane *, int, bool, bool) = CAST(func, 0x006D9070);

    func(this, nullptr, a4, a2, lane, a9, a10, a11);
}

void traffic::set_current_lane(traffic_path_lane *a2, int a3, bool a4)
{
    TRACE("traffic::set_current_lane");
    THISCALL(0x006C72D0, this, a2, a3, a4);
}

traffic *traffic::get_traffic_from_entity(vhandle_type<entity> a1) {
    return (traffic *) CDECL_CALL(0x006C3510, a1);
}

void traffic_patch()
{
    {
        FUNC_ADDRESS(address, &traffic::_do_spawn);
        //set_vfunc(0x008A5CF8, address);
    }

    {
        FUNC_ADDRESS(address, &traffic::set_current_lane);
        REDIRECT(0x006D9095, address);
    }

    //SET_JUMP(0x006D33C0, traffic::enable_traffic);
}
