#include "spawnable.h"

#include "actor.h"
#include "base_ai_core.h"
#include "camera.h"
#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "os_developer_options.h"
#include "ped_spawner.h"
#include "trace.h"
#include "traffic.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"
#include "wds.h"

VALIDATE_SIZE(spawnable, 0x8);

_std::vector<traffic_path_graph::laneInfoStruct> *& spawnable::spawnable_lanes = var<_std::vector<traffic_path_graph::laneInfoStruct> *>(0x0096C9B0);

traffic_path_graph::laneInfoStruct *& spawnable::last_spawn_lane_info = var<traffic_path_graph::laneInfoStruct *>(0x0096C9B4);

po & spawnable::last_camera_po = var<po>(0x00938140);

float & spawnable::spawn_spacing = var<float>(0x00937FA0);

spawnable::spawnable(vhandle_type<entity> )
{
    this->m_vtbl = 0x008A59FC;
    if ( spawnable_lanes == nullptr )
    {
        spawnable_lanes = new _std::vector<traffic_path_graph::laneInfoStruct> {};
        spawnable_lanes->reserve(30u);
    }

    this->field_4 = true;
    this->field_5 = true;
}

int sub_68FB70()
{
    int result = 0;
    if ( ai::ai_core::the_ai_core_list_high() != nullptr ) {
        result = ai::ai_core::the_ai_core_list_high()->size();
    }

    if ( ai::ai_core::the_ai_core_list_low() != nullptr ) {
        result += ai::ai_core::the_ai_core_list_low()->size();
    }

    return result;
}

bool spawnable::should_update_spawn_lanes(po &a1, entity_base *ent)
{
    return (bool) CDECL_CALL(0x006B9560, &a1, ent);
}

void spawnable::update_spawn_lanes()
{
    CDECL_CALL(0x006CFF20);
}

static int & dword_937F9C = var<int>(0x00937F9C); 

void sub_6D1800()
{
    if ( g_world_ptr->field_158.field_C != dword_937F9C )
    {
        dword_937F9C = g_world_ptr->field_158.field_C;
        auto *current_view_camera = g_game_ptr->get_current_view_camera(0);
        if ( current_view_camera != nullptr )
        {
            if ( spawnable::should_update_spawn_lanes(spawnable::last_camera_po, current_view_camera) ) {
                spawnable::update_spawn_lanes();
            }
        }
    }
}

void spawnable::advance_traffic_and_peds(Float a1) {
    TRACE("spawnable::advance_traffic_and_peds");

    if constexpr (0) {
        if ( spawnable_lanes != nullptr
                && (traffic::traffic_enabled || os_developer_options::instance->get_flag(mString {"ENABLE_PEDESTRIANS"})) )
        {                                             // 
            auto v1 = sub_68FB70();
            auto v3 = v1 - traffic::traffic_list.size();
            auto v5 = v3 - ped_spawner::ped_spawner_list.size();
            float v6;
            if ( v5 > 3 )
            {
                if ( v5 < 7 ) {
                    v6 = (1.0f - (v5 - 3) * 0.25) * 0.80000001 + 0.2f;
                } else {
                    v6 = 0.2;
                    traffic::set_traffic_density(0.2);
                }
            }
            else
            {
                v6 = 1.0f;
                traffic::set_traffic_density(1.0);
            }

            sub_6C2E10(v6);
            sub_6D1800();
            traffic::advance_traffic(a1);
            ped_spawner::advance_peds(a1);
        }
    } else {
        CDECL_CALL(0x006D8610, a1);
    }
}

void spawnable::do_spawn(
        vector3d a4,
        vector3d a2,
        traffic_path_lane *lane,
        bool a10,
        bool a11)
{
    void (__fastcall *func)(void *, void *edx,
            vector3d, vector3d, traffic_path_lane *,
            bool, bool) = CAST(func, get_vfunc(m_vtbl, 0x0));
    func(this, nullptr, a4, a2, lane, a10, a11);
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
        vector3d &a4)
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
        last_spawn_lane_info = next_lane_struct;
        if ( this->is_viable_pos(a4) ) {
            this->do_spawn(a4, v45, the_lane, next_lane_struct->field_E, 1);
        }
    }

    return result;
}

traffic_path_graph::laneInfoStruct * spawnable::get_spawnable_lane(
        traffic_path_lane::eLaneType arg0,
        vector3d *arg4,
        traffic_path_graph **arg8,
        traffic_path_graph **argC,
        int *a6,
        bool a7,
        bool a8,
        bool a9)
{
    if constexpr (0) {
    } else {
        return (traffic_path_graph::laneInfoStruct *) THISCALL(0x006C9990, this, arg0, arg4, arg8, argC, a6, a7, a8, a9);
    }
}

traffic_path_graph::laneInfoStruct * spawnable::get_new_spawn_pos(
        traffic_path_lane::eLaneType a1,
        vector3d *a2,
        traffic_path_graph **a3,
        traffic_path_graph **a4,
        int *a5)
{
    if ( spawnable_lanes != nullptr
            && !spawnable_lanes->empty())
    {
        return this->get_spawnable_lane(a1, a2, a3, a4, a5,
            false,
            false,
            true
        );
    }

    return nullptr;
}

void spawnable::sub_6B9B60(Float )
{
    if ( !this->field_5 )
    {
        auto current_view_camera = g_game_ptr->get_current_view_camera(0);
        if ( current_view_camera != nullptr )
        {
            auto v7 = current_view_camera->get_abs_position();
            auto *v4 = this->get_my_actor();
            if ( this->field_4 )
            {
                auto v6 = v7 - v4->get_abs_position();
                auto v9 = v6.xz_length2();
                if ( flt_937FA8 * flt_937FA8 * 1.21f < v9 )
                {
                    this->un_spawn();
                    this->field_5 = true;
                }
            }
        }
    }
}

void sub_6C2E10(Float a1)
{
    flt_937FF0 = std::clamp(static_cast<float>(a1), 0.0f, 1.0f);;
}


void spawnable_patch()
{
    {
        REDIRECT(0x0055842F, spawnable::advance_traffic_and_peds);
    }
}
