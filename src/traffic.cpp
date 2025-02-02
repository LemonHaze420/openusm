#include "traffic.h"

#include "camera.h"
#include "common.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "game.h"
#include "parking_marker.h"
#include "poi.h"
#include "region.h"
#include "terrain.h"
#include "trace.h"
#include "traffic_path_graph.h"
#include "traffic_path_lane.h"
#include "traffic_signal_mgr.h"
#include "variables.h"
#include "vehicle.h"
#include "wds.h"

VALIDATE_OFFSET(traffic, field_1C4, 0x1C4);
VALIDATE_SIZE(traffic, 0x22Cu);

static bool & g_traffic_paused = var<bool>(0x0096CA0C);

static int & g_traffic_testscript = var<int>(0x0096CA14);

static int & g_traffic_single_step = var<int>(0x0096CA10);

static const vector3d HELLFORCARS {0.0, -99.0, 0.0};

bool & traffic::traffic_enabled = var<bool>(0x00937FF4);

float & traffic::traffic_density = var<float>(0x00937FF8);

float & traffic::parking_density = var<float>(0x00937FFC);

bool & traffic::traffic_initialized = var<bool>(0x0096C9E8);

int & traffic::lane_changes_this_frame = var<int>(0x0096CA18);
int & traffic::spawned_this_frame = var<int>(0x0096C9F4);
int & traffic::unspawned_this_frame = var<int>(0x0096C9F8);

_std::vector<traffic *> & traffic::traffic_list = var<_std::vector<traffic *>>(0x0096D2A8);

int & traffic::last_traffic_id = var<int>(0x0096CA08);

float (&stru_937FAC)[8] = var<float[8]>(0x00937FAC);

std::array<int, 5> &traffic::old_drivers = var<std::array<int, 5>>(0x0096CA40);

std::array<int, 5> &traffic::new_drivers = var<std::array<int, 5>>(0x0096CA40);

int &traffic::living_cars = var<int>(0x0096C9EC);
int &traffic::parked_cars = var<int>(0x0096C9F0);
int &traffic::visible_cars = var<int>(0x0096C9FC);

traffic *& traffic::manual_car = var<traffic *>(0x0096C9D8);

traffic *& traffic::getaway_car = var<traffic *>(0x0096C9E0);

traffic *& traffic::emergency_car = var<traffic *>(0x0096C9E4);

_std::vector<traffic *> *& traffic::ai_occupied_cars = var<_std::vector<traffic *> *>(0x0096CA00);

static float &stru_937FA4 = var<float>(0x00937FA4);

void traffic::sub_6DA3B0(Float a2, Float a3, Float a4) {
    auto &v4 = this->field_14C;
    v4[0] = a2;
    v4[1] = a3;
    v4[2] = a4;
}

char sub_6BA070()
{
    return (char) CDECL_CALL(0x006BA070);
}

traffic::~traffic()
{
    this->m_vtbl = 0x008A5CF8;
    this->field_C.m_vtbl = 0x008A5CE8;

    entity_base_vhandle v6;
    v6.field_0 = this->field_C.field_50.field_0.field_0;
    event_manager::remove_callback(this->field_210, event::DAMAGED, v6);

    v6.field_0 = this->field_C.field_50.field_0.field_0;
    event_manager::remove_callback(this->field_214, event::DESTROYED, v6);

    this->field_4 = true;

    this->set_driver_type(0);
    this->un_spawn();

    this->sub_6CD2D0(this);
}

void traffic::initialize_traffic()
{
    if constexpr (0) {
        if ( !traffic_initialized )
        {
            flt_937FA4 = 10.0;
            flt_937FA8 = 90.0;
            if ( spawnable::spawnable_lanes == nullptr )
            {
                spawnable::spawnable_lanes = new _std::vector<traffic_path_graph::laneInfoStruct> {};
                spawnable::spawnable_lanes->reserve(30u);
            }

            if ( sub_6BA070() )
            {
                for ( auto i = 29; i > -1; --i ) {
                    create_new_traffic(i);
                }
            }

            traffic_initialized = true;
            vector3d a4 {-1234.0};
            spawnable::last_camera_po.set_po(ZVEC, YVEC, a4);
        }
    } else {
        CDECL_CALL(0x006D1CC0);
    }
}

void traffic::sub_6CD2D0(traffic *a2)
{
    if constexpr (0) {
        bool v3 = false;
        for ( auto idx = 0; idx < traffic_list.size(); ++idx )
        {
            if ( traffic_list[idx] == a2 ) {
                v3 = true;
            }

            if ( v3 && idx < traffic_list.size() - 1 ) {
                traffic_list[idx] = traffic_list[idx + 1];
            }
        }

        if ( v3 && !traffic::traffic_list.empty() ) {
            --traffic_list.m_last;
        }

    } else {
        THISCALL(0x006CD2D0, this, a2);
    }
}

bool sub_6DA670(traffic_path_lane *a1, traffic_path_lane *a2)
{
    if ( a1 == a2 ) {
        return true;
    }

    if ( a1 != nullptr )
    {
        if ( a2 != nullptr )
        {
            if ( a1->is_valid(nullptr) && a2->is_valid(nullptr) && a1->my_road != a2->my_road )
            {
                auto *next_intersection = a1->get_next_intersection(1);
                auto *v4 = a2->sub_5C8460();
                auto *v5 = a1->sub_5C8460();
                auto *v6 = a2->get_next_intersection(1);
                if ( next_intersection != nullptr )
                {
                    if ( v4 != nullptr && v5 != nullptr && v6 != nullptr && v4 == next_intersection && v6 != v5 )
                        return 1;
                }
            }
        }
    }

    return false;
}

void traffic::sub_6DACB0(entity_base_vhandle a2)
{
    if ( a2 == this->get_my_actor()->get_my_vhandle() ) {
        this->field_170.field_0 = 0;
    } else {
        this->field_170.field_0 = a2.field_0;
    }
}

traffic * traffic::car_behind()
{
    vhandle_type<entity> v5 {};
    int v3;
    auto *v2 = this->field_140;
    if ( v2 != nullptr
            && (v3 = this->field_16C, v3 < v2->get_num_ais() - 1)
            && (v5.field_0 = this->field_140->get_ai_by_index(v3 + 1),
                v5.get_volatile_ptr() != nullptr) )
    {
        return get_traffic_from_entity(v5);
    }

    return nullptr;
}

vector3d traffic::pop_dest()
{
    auto v5 = this->field_1EC.back();
    this->field_1EC.pop_back();

    return v5;
}

bool traffic::is_not_chase_lane(traffic_path_lane *a1)
{
    if ( !traffic_list.empty() )
    {
        if (old_drivers[2] + old_drivers[3] > 0)
        {
            for ( auto &v4 : traffic_list )
            {
                if (v4 != nullptr) {
                    auto v5 = v4->field_1C4;
                    if ( v5 == 2 || v5 == 1 ) {
                        if ( sub_6DA670(v4->get_current_lane(), a1) || sub_6DA670(v4->field_148, a1) ) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void traffic::update_destination(Float a2)
{
    auto v3 = this->field_15C;
    if ( this->field_1BC )
    {
        switch ( v3 )
        {
            case 1: {
                this->field_C.manage_vehicle_height(false);
                auto *v6 = this->field_140;
                if ( v6 != nullptr )
                {
                    auto v7 = this->field_168 + 1;
                    this->field_190 = nullptr;
                    this->field_168 = v7;
                    auto v8 = v6->get_node(v7);
                    this->sub_6DA3B0(v8[0], v8[1], v8[2]);
                    this->field_1BC = false;
                    auto *v9 = this->get_my_actor();
                    if ( v9->get_floor_offset() > EPSILON ) {
                        this->field_15C = 3;
                    }

                    this->field_1BE = false;
                    auto v10 = this->car_ahead();
                    if ( v10 != nullptr )
                    {
                        auto v11 = v10->field_C.field_C8;
                        a2 = this->field_C.field_C8;
                        auto v26 = v11;
                        float v12 = v26;
                        if ( v11 >= a2 ) {
                            v12 = a2;
                        }

                        this->field_C.field_C8 = v12;
                    }
                }
                break;
            }
            case 2:
            case 3:
                goto LABEL_28;
            case 4: {
                this->clear_previous_lane();
                --this->field_168;
LABEL_28:
                this->release_turn();
                auto *v15 = this->field_140;
                auto v16 = this->field_168 + 1;
                this->field_168 = v16;
                if ( v16 >= v15->total_nodes )
                {
                    auto *next_intersection = v15->get_next_intersection(1);
                    if ( next_intersection->has_stopsign(false) )
                    {
                        this->field_178 = 0.0099999998;
                        this->field_15C = 8;
                    }
                    else if ( !this->start_turn() )
                    {
                        this->field_15C = 9;
                    }
                }
                else
                {
                    this->field_15C = 3;
                    auto v17 = v15->get_node(v16);
                    this->sub_6DA3B0(v17[0], v17[1], v17[2]);
                    this->field_1BC = false;
                }
                break;
            }
            case 5:
                if ( this->more_dests() )
                {
                    auto v14 = this->pop_dest();
                    this->sub_6DA3B0(v14[0], v14[1], v14[2]);
                    this->field_1BC = false;
                }

                if ( !this->more_dests() )
                {
                    this->field_174 = 0;
                    this->field_15C = 3;
                }

                break;
            case 6: {
                auto *the_traffic = traffic::get_traffic_from_entity(vhandle_type<entity>{this->field_174});
                if ( the_traffic != this->car_behind() || this->is_not_chase_lane(this->field_140) || !this->is_fully_pulled_over() )
                {
                    if ( this->more_dests() )
                    {
                        auto v14 = this->pop_dest();
                        this->sub_6DA3B0(v14[0], v14[1], v14[2]);
                        this->field_1BC = false;
                    }

                    if ( !this->more_dests() )
                    {
                        this->field_174 = 0;
                        this->field_15C = 3;
                    }
                }
                break;
            }
            case 8: {
                this->release_turn();
                if ( this->field_1C4 || std::abs(this->field_C.field_C8) < EPSILON ) {
                    this->field_15C = 9;
                } else {
                    this->field_178 += a2;
                }
                break;
            }
            case 9: {
                this->release_turn();
                this->field_140->get_next_intersection(1);
                if ( !this->start_turn() ) {
                    this->field_178 = 0.0099999998;
                }
                break;
            }
            case 10: {
                if ( !this->field_144 ) {
                    this->field_15C = 9;
                } else {
                    this->field_1BC = 0;
                    auto *v4 = this->field_144;
                    this->field_15C = 11;

                    auto v5 = 0;
                    if ( v4->total_nodes == 0 ) {
                        v5 = v4->total_nodes - 1;
                    }

                    this->sub_6DA3B0(v4->nodes[v5][0], v4->nodes[v5][1], v4->nodes[v5][2]);
                }
                break;
            }
            case 11: {
                this->start_lane(this->field_144, 1);
                break;
            }
            case 12: {
                this->release_turn();

                this->field_1BC = true;
                auto v20 = this->field_C.sub_6DA250();
                this->sub_6DA3B0(v20[0], v20[1], v20[2]);
                break;
            }
            case 13: {
                this->release_turn();

                this->field_1BC = true;
                auto v20 = this->field_C.sub_6DA250();
                this->sub_6DA3B0(v20[0], v20[1], v20[2]);
                break;
            }
            default:
                break;
        }
    }
    else if ( ((v3 == 3 || v3 == 2) && this->field_1C4) && !this->field_1BE )
    {
        this->check_lane_change();
    }

    auto v21 = this->field_140;
    if ( v21 != nullptr && v21->is_valid(nullptr) )
    {
        auto v22 = this->field_15C;
        if ( v22 != 11 && v22 != 10 )
        {
            auto v23 = this->field_16C;
            if ( v23 <= 0 )
            {
                this->sub_6DACB0(0);
            }
            else
            {
                auto ai_by_index = this->field_140->get_ai_by_index(v23 - 1);
                this->sub_6DACB0(ai_by_index);
            }
        }
    }
    else
    {
        int a2 = 0;
        if ( this->get_my_actor()->my_handle.field_0 ) {
            *(float *)&this->field_170.field_0 = a2;
        } else {
            this->field_170.field_0 = 0;
        }
    }
}

bool traffic::start_lane(traffic_path_lane *a2, bool a3)
{
    return (bool) THISCALL(0x006C7400, this, a2, a3);
}

bool traffic::start_turn()
{
    return (bool) THISCALL(0x006D24A0, this);
}

void traffic::clear_previous_lane()
{
    auto *v2 = this->previous_lane;
    if ( v2 != nullptr && v2->is_valid(nullptr) )
    {
        auto v3 = this->get_my_actor()->get_my_vhandle();
        this->previous_lane->get_ai_index(vhandle_type<actor> {v3});
        this->previous_lane->remove_ai_from_lane(vhandle_type<actor>{v3});
        this->previous_lane->update_lane_indexes();
    }

    this->previous_lane = nullptr;
}

traffic * traffic::car_ahead()
{
    return (traffic *) THISCALL(0x006C3650, this);
}

void traffic::release_turn()
{
    THISCALL(0x006C3DD0, this);
}

void traffic::check_lane_change()
{
    THISCALL(0x006C7680, this);
}

void traffic::test_script()
{
    if constexpr (0) {
    } else {
        CDECL_CALL(0x006D6350);
    }
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
    if ( !traffic_initialized ) {
        return;
    }

    bool v13 = false;
    bool v14 = true;

    while (v14)
    {
        v14 = false;
        for ( auto &the_traffic : traffic_list )
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
        traffic_list.clear();
    }

    traffic_initialized = false;
}

void traffic::enable_traffic(bool a1, bool a2)
{
    TRACE("traffic::enable_traffic");

    if constexpr (1)
    {
        traffic_enabled = a1;
        if ( a1 && !traffic_initialized ) {
            initialize_traffic();
        }

        if (!a1 && traffic_initialized ) {
            terminate_traffic();
        }
    }
    else
    {
        CDECL_CALL(0x006D33C0, a1, a2);
    }
}

static constexpr vector3d stru_9381D4 {0.0, -99.0, 0.0};

vhandle_type<entity> sub_6C6790(uint32_t vehicle_type, int a3)
{
    vhandle_type<entity> result;
    if ( vehicle_type <= 7 && vehicle::models()[vehicle_type] != nullptr )
    {
        vehicle::cur_vehicle_type = vehicle_type;
        auto v1 = vehicle::models()[vehicle_type]->create(a3);
        vehicle::cur_vehicle_type = -1;
        result.field_0 = {v1};
    }
    else
    {
        result.field_0 = {0};
    }

    return result;
}

void traffic::create_new_traffic(int a1)
{
    auto vehtype = (a1 >= 0
        ? vehicle::pick_model(a1)
        : vehicle::pick_random_model()
    );

    auto v3 = last_traffic_id++;
    auto v5 = sub_6C6790(vehtype, v3);

    if ( v5.get_volatile_ptr() != nullptr )
    {
        auto *t = get_traffic_from_entity_slow(v5);

        assert(t != nullptr);
        assert(t->field_C.bodytype == static_cast<uint32_t>(vehtype));

        t->field_C.bodytype = vehtype;

        t->field_158 = true;
        t->field_1C4 = 0;

        auto *v7 = t->get_my_actor();
        v7->set_visible(false, false);
        t->field_C.sub_6BAED0(stru_9381D4);
    }
}

traffic * traffic::get_traffic_from_entity_slow(vhandle_type<entity> a1)
{
    if ( !traffic_list.empty() )
    {
        for (auto &t : traffic_list)
        {
            if (t != nullptr) {
                if ( t->get_my_actor()->my_handle == a1 ) {
                    return t;
                }
            }
        }
    }

    return nullptr;
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

bool traffic::is_ai_car_occupied() const
{
    if ( ai_occupied_cars == nullptr ) {
        return false;
    }

    for ( auto &the_traffic : (*ai_occupied_cars) )
    {
        if ( the_traffic == this )
        {
            assert(this->is_ai_potential_car());

            return true;
        }
    }

    return false;
}

void traffic::_do_spawn(
        vector3d a4,
        vector3d a2,
        traffic_path_lane *lane,
        bool a10,
        bool a11)
{
    TRACE("traffic::do_spawn");

    if constexpr (0) {
    } else {
        void (__fastcall *func)(traffic *, void *edx,
                vector3d, vector3d, traffic_path_lane *, bool, bool) = CAST(func, 0x006D9070);

        func(this, nullptr, a4, a2, lane, a10, a11);
    }
}

bool sub_6DA550(const vector3d &a1, Float a2)
{
    auto *current_view_camera = g_game_ptr->get_current_view_camera(0);
    if ( current_view_camera == nullptr )
    {
        auto abs_pos = current_view_camera->get_abs_position();

        auto abs_po = current_view_camera->get_abs_po();
        auto z_facing = abs_po.get_z_facing();

        auto v9 = a1 - abs_pos;

        if ( a2 > v9.xz_length2() )
        {
            auto v8 = dot(v9, z_facing);
            if ( v8 > 0.0 ) {
                return true;
            }
        }
    }

    return false;
}

bool sub_6DA630(const vhandle_type<parking_marker> &a1)
{
    auto *marker = a1.get_volatile_ptr();
    auto abs_pos = marker->get_abs_position();

    return sub_6DA550(abs_pos, 50.0 * 50.0);
}

void traffic::_critical_processing(Float a2)
{
    bool v3 = false;
    bool v4 = false;
    if ( spawned_this_frame < 1 ) {
        v3 = true;
    }

    if ( parking_density * traffic_density * 30.0f > parked_cars ) {
        v4 = true;
    }

    if ( !traffic_enabled
            || (traffic_density * 30.0f) <= (parked_cars + living_cars)
            || (traffic_density * 15.0f) <= visible_cars )
    {
        v3 = false;
    }

    if ( this->field_4 && v3 )
    {
        if ( this->is_ai_potential_car() ) {
            return;
        }

        if ( this->field_5 )
        {
            parking_marker *open_parking_marker = nullptr;
            parking_marker *v7 = nullptr;
            if ( this->field_C.bodytype == 1
                    && v4
                    && (open_parking_marker = find_open_parking_marker(), (v7 = open_parking_marker) != nullptr)
                    && !sub_6DA630(vhandle_type<parking_marker>{int(v7)}) )
            {
                this->spawn_at_marker(open_parking_marker);
            }
            else if ( parking_density < 1.0f )
            {
                traffic_path_graph *a3 = nullptr;
                traffic_path_graph *a4 = nullptr;
                int a5 = 0;
                auto *new_spawn_pos = this->get_new_spawn_pos(
                        static_cast<traffic_path_lane::eLaneType>(0),
                        &this->field_1D4,
                        &a3,
                        &a4,
                        &a5);

                if ( new_spawn_pos == nullptr || poi_manager::near_violence_poi(this->field_1D4) )
                {
                    ++unspawned_this_frame;
                }
                else
                {
                    this->field_14C = this->prepare_for_spawn(new_spawn_pos, this->field_1D4);
                    ++spawned_this_frame;
                }

                this->field_1D4 = FARAWAY;
            }
        }
    }

    if ( !this->is_ai_potential_car() && this->field_4 ) {
        this->sub_6B9B60(a2);
    }
}

bool traffic::_is_viable_pos(const vector3d &a2)
{
    if constexpr (0) {
        auto *the_terrain = g_world_ptr->get_the_terrain();
        auto *reg = the_terrain->find_region(a2, nullptr);
        if ( reg == nullptr || !reg->is_loaded() )
        {
            return false;
        }

        if ( !traffic_list.empty() && old_drivers[2] > 0 )
        {
            for ( auto &the_traffic : traffic_list )
            {
                if ( the_traffic != nullptr && uint16_t(the_traffic->field_1C4) == 2 )
                {
                    auto v6 = the_traffic->field_C.sub_6DA250();
                    auto v7 = v6 - a2;
                    if ( v7.xz_length2() < 625.0 ) {
                        return false;
                    }

                }
            }
        }

        return true;
    } else {
        return (bool) THISCALL(0x006CB300, this, &a2);
    }
}

void traffic::spawn_at_marker(parking_marker *a2)
{
    assert(this->get_my_actor() != nullptr);
    assert(a2 != nullptr);

    if constexpr (0)
    {
        a2->set_active(true);
        this->field_228 = a2->my_handle.field_0;

        auto *v5 = this->field_C.get_my_actor();
        v5->set_allow_tunnelling_into_next_frame(1);

        entity_set_abs_po(v5, a2->get_abs_po());

        auto *v7 = this->get_my_actor();
        event_manager::raise_event(event::RESPAWNED_THIS_FRAME, v7->my_handle);

        this->field_C.set_collidable(true);
        this->field_C.set_visible(true);

        this->field_C.pick_body_and_color();
        this->field_C.sub_6BA920(0);
        this->field_C.field_C8 = 0.0;
        this->field_15C = 14;
        this->field_5 = false;

        auto *v12 = this->get_my_actor();
        v12->invalidate_frame_delta();

        auto *v13 = this->get_my_actor();
        v13->compute_sector(g_world_ptr->the_terrain, false, nullptr);

        this->field_C.manage_engine_sounds(0.000099999997, false);
    } else {
        THISCALL(0x006D8F20, this, a2);
    }
}

void traffic::set_traffic_density(Float density)
{
    assert(density >= 0.0f);
    assert(density <= 1.0f);

    traffic_density = density;
}

void traffic::set_ai_controller(ai::ai_car_inode *a2)
{
    this->field_220 = a2;
}

void traffic::set_ai_car_occupied(bool a2)
{
    if ( a2 )
    {
        assert(this->is_ai_potential_car());

        if ( !this->is_ai_car_occupied() )
        {
            if (ai_occupied_cars == nullptr) {
                ai_occupied_cars = new _std::vector<traffic *> {};
            }

            ai_occupied_cars->push_back(this);
        }

        assert(this->is_ai_car_occupied());
    }
    else
    {
        this->set_ai_controller(nullptr);

        if ( ai_occupied_cars != nullptr )
        {
            for ( auto &the_traffic : (*ai_occupied_cars) )
            {
                if (the_traffic == this) {
                    assert(this->is_ai_potential_car());
                    break;
                }
            }

            if ( ai_occupied_cars->empty() )
            {
                if ( ai_occupied_cars != nullptr) {
                    delete ai_occupied_cars;
                }

                ai_occupied_cars = nullptr;
            }
        }

        assert(!this->is_ai_car_occupied());
    }
}

void sub_6BC0A0(traffic *a1)
{
    if ( traffic::getaway_car != a1 )
    {
        traffic::getaway_car = a1;
        spawnable::spawn_spacing = 2.0;
        if ( a1 == nullptr ) {
            spawnable::spawn_spacing = 1.0;
        }
    }
}

void sub_6BC060(traffic *a1)
{
    if ( traffic::manual_car != a1 )
    {
        if ( traffic::manual_car != nullptr ) {
            traffic::manual_car->field_1C4 = 0;
        }
        
        traffic::manual_car = a1;
        if ( a1 != nullptr )
        {
            a1->field_1C4 = -1;
            traffic::manual_car->field_4 = false;
        }
    }
}

void sub_6BC0D0(traffic *a1)
{
    if ( traffic::emergency_car != a1 ) {
        traffic::emergency_car = a1;
    }
}

void traffic::set_driver_type(int a2)
{
    if ( this->field_1C4 != a2 )
    {
        if ( this == manual_car ) {
            sub_6BC060(nullptr);
        }

        if ( this == getaway_car ) {
            sub_6BC0A0(nullptr);
        }

        if ( this == emergency_car ) {
            sub_6BC0D0(nullptr);
        }

        this->field_1C4 = a2;
        switch ( a2 )
        {
            case -1:
                sub_6BC060(this);
                break;
            case 0:
            case 3:
                return;
            case 1:
                sub_6BC0D0(this);
                break;
            case 2:
                sub_6BC0A0(this);
                break;
            default:
                assert(0 && "unknown driver type");
                return;
        }
    }
}



void traffic::set_current_lane(traffic_path_lane *a2, int a3, bool a4)
{
    TRACE("traffic::set_current_lane");
    THISCALL(0x006C72D0, this, a2, a3, a4);
}

traffic *traffic::get_traffic_from_entity(vhandle_type<entity> a1) {
    return (traffic *) CDECL_CALL(0x006C3510, a1);
}

void traffic::set_traffic_model_usage(int modelid, Float a2)
{
    assert(modelid >= VEHICLE_MODEL_TAXI && modelid < VEHICLE_MODEL_MAX);
    if ( modelid < 8 ) {
        stru_937FAC[modelid] = a2;
    }
}

bool traffic::add_traffic_model(int a1, mString &a2)
{
    float v4 = 1.0;
    mString v3 = a2;
    return vehicle::add_model(a1, v3, v4);
}

static float & flt_96C9D4 = var<float>(0x0096C9D4);

static void sub_6C3430() {
    CDECL_CALL(0x006C3430);
}

static void sub_6C3490(Float a1)
{
    if ( flt_96C9D4 > 0.0f )
    {
        auto v1 = flt_96C9D4 - a1;
        flt_96C9D4 = v1;
        if ( v1 <= 0.0f ) {
            sub_6C3430();
        }
    }
}

void traffic::advance_traffic(Float a1)
{
    //update_global_cheats();
    if ( g_traffic_single_step )
    {
        --g_traffic_single_step;
    }
    else if ( g_traffic_paused )
    {
        return;
    }

    if ( traffic_enabled )
    {
        sub_6C3490(a1);

        if ( spawned_this_frame && stru_937FA4 < 40.0f ) {
            stru_937FA4 += 1.0f;
        }

        old_drivers = new_drivers;

        new_drivers = {};

        living_cars = 0;
        parked_cars = 0;
        visible_cars = 0;

        if ( g_traffic_testscript )
        {
            --g_traffic_testscript;
            test_script();
        }

        traffic_signal_mgr::frame_advance(a1);
        if ( !traffic_initialized )
        {
            if ( nullptr == g_game_ptr->get_current_view_camera(0) ) {
                return;
            }

            initialize_traffic();
        }

        lane_changes_this_frame = 0;
        spawned_this_frame = 0;
        unspawned_this_frame = 0;
    }
}

parking_marker * traffic::find_open_parking_marker()
{
    return (parking_marker *) CDECL_CALL(0x006BC150);
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
