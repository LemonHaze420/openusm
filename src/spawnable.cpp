#include <climits>
#include <cstdlib>

#include <cmath>

#include "spawnable.h"

#include "actor.h"
#include "base_ai_core.h"
#include "camera.h"
#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "os_developer_options.h"
#include "region.h"
#include "collide.h"

#include "ped_spawner.h"
#include "trace.h"
#include "traffic.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"
#include "wds.h"

VALIDATE_SIZE(spawnable, 0x8);

_std::vector<traffic_path_graph::laneInfoStruct> *&spawnable::spawnable_lanes =
    var<_std::vector<traffic_path_graph::laneInfoStruct> *>(0x0096C9B0);

traffic_path_graph::laneInfoStruct *&spawnable::last_spawn_lane_info =
    var<traffic_path_graph::laneInfoStruct *>(0x0096C9B4);

po &spawnable::last_camera_po = var<po>(0x00938140);

float &spawnable::spawn_spacing = var<float>(0x00937FA0);

static Var<bool> force_lane_update{0x00938180};

#if STANDALONE_SYSTEM
static const bool native_spawn_defaults = [] {
    spawnable::spawn_spacing = 1.0f;
    flt_937FA8 = 70.0f;
    return true;
}();
#endif

namespace {


void __fastcall base_spawn(spawnable *, void *, vector3d, vector3d, traffic_path_lane *, int, bool, bool) {}
void __fastcall base_unspawn(spawnable *, void *) {}
void __fastcall base_critical(spawnable *, void *, Float) {}
actor *__fastcall base_actor(spawnable *, void *)
{
    return nullptr;
}
void __fastcall base_set_actor(spawnable *, void *, vhandle_type<entity>) {}
bool __fastcall base_lane(spawnable *, void *, traffic_path_lane *)
{
    return true;
}
bool __fastcall base_position(spawnable *, void *, const vector3d &)
{
    return true;
}
}  // namespace

void *spawnable::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&base_spawn),
        reinterpret_cast<void *>(&base_unspawn),
        reinterpret_cast<void *>(&base_critical),
        reinterpret_cast<void *>(&base_actor),
        reinterpret_cast<void *>(&base_set_actor),
        reinterpret_cast<void *>(&base_lane),
        reinterpret_cast<void *>(&base_position),
    };
    return table;
}

spawnable::spawnable(vhandle_type<entity>)
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    if (spawnable_lanes == nullptr) {
        spawnable_lanes = new _std::vector<traffic_path_graph::laneInfoStruct>{};
        spawnable_lanes->reserve(30u);
    }

    this->field_4 = true;
    this->field_5 = true;
}

// 0x0068FB70
int count_active_ai_cores()
{
    int result = 0;
    if (ai::ai_core::the_ai_core_list_high != nullptr) {
        result = ai::ai_core::the_ai_core_list_high->size();
    }

    if (ai::ai_core::the_ai_core_list_low != nullptr) {
        result += ai::ai_core::the_ai_core_list_low->size();
    }

    return result;
}

bool spawnable::should_update_spawn_lanes(po &last_po, entity_base *camera_entity)
{
    const auto &camera_po = camera_entity->get_abs_po();
    vector3d forward = camera_po[2];
    if (std::fabs(forward.y) >= 0.99f) {
        forward = forward.y > 0.0f ? -camera_po[1] : camera_po[1];
    }
    forward.y = 0.0f;
    const float length_squared = forward.x * forward.x + forward.z * forward.z;
    if (length_squared > 0.00001f) {
        const float inverse_length = 1.0f / std::sqrt(length_squared);
        forward.x *= inverse_length;
        forward.z *= inverse_length;
    }

    const auto position = camera_po.get_position();
    const auto previous_position = last_po.get_position();
    const auto previous_forward = vector3d{last_po[2].x, 0.0f, last_po[2].z};
    const float facing_dot = forward.x * previous_forward.x + forward.z * previous_forward.z;
    if (facing_dot <= 0.5f) {
        const vector3d up{0.0f, 1.0f, 0.0f};
        last_po.set_po(forward, up, position);
        return true;
    }

    const float dx = previous_position.x - position.x;
    const float dz = previous_position.z - position.z;
    if (dx * dx + dz * dz > 400.0f || force_lane_update()) {
        last_po.set_position(position);
        return true;
    }
    return false;
}

// 0x006DC490
static void shuffle_spawnable_lanes(traffic_path_graph::laneInfoStruct *first, traffic_path_graph::laneInfoStruct *last)
{
    unsigned int count = 2;
    for (auto *current = first + 1; current != last; ++current, ++count) {
        unsigned int random_max = RAND_MAX;
        unsigned int random = static_cast<unsigned int>(std::rand()) & RAND_MAX;
        while (random_max < count) {
            if (random_max == UINT_MAX) {
                break;
            }
            random = (random << 15) | RAND_MAX;
            random_max = (random_max << 15) | RAND_MAX;
        }
        auto *selected = first + random % count;
        const auto temporary = *current;
        *current = *selected;
        *selected = temporary;
    }
}

void spawnable::update_spawn_lanes()
{
    last_spawn_lane_info = nullptr;
    spawnable_lanes->clear();

    auto *camera = g_game_ptr->get_current_view_camera(0);
    if (camera == nullptr) {
        return;
    }
    auto *primary_region = camera->get_primary_region();
    if (primary_region == nullptr) {
        return;
    }
    auto *graph = primary_region->get_traffic_path_graph();
    if (graph == nullptr) {
        return;
    }
    graph->get_spawnable_lane_list(camera, spawnable_lanes, Float{10.0f}, Float{90.0f});
    if (!spawnable_lanes->empty()) {
        auto *first = &*spawnable_lanes->begin();
        shuffle_spawnable_lanes(first, first + spawnable_lanes->size());
    }
    force_lane_update() = false;
}

static int &dword_937F9C = var<int>(0x00937F9C);

void sub_6D1800()
{
    if (g_world_ptr->time_manager.field_C != dword_937F9C) {
        dword_937F9C = g_world_ptr->time_manager.field_C;
        auto *current_view_camera = g_game_ptr->get_current_view_camera(0);
        if (current_view_camera != nullptr) {
            if (spawnable::should_update_spawn_lanes(spawnable::last_camera_po, current_view_camera)) {
                spawnable::update_spawn_lanes();
            }
        }
    }
}

void spawnable::advance_traffic_and_peds(Float a1)
{
    TRACE("spawnable::advance_traffic_and_peds");

    if constexpr (1) {
        if (spawnable_lanes != nullptr &&
            (traffic::traffic_enabled || os_developer_options::instance->get_flag(mString{"ENABLE_PEDESTRIANS"}))) {  //
            auto v1 = count_active_ai_cores();
            auto v3 = v1 - traffic::traffic_list.size();
            auto v5 = v3 - ped_spawner::ped_spawner_list.size();
            float v6;
            if (v5 > 3) {
                if (v5 < 7) {
                    v6 = (1.0f - (v5 - 3) * 0.25) * 0.80000001 + 0.2f;
                } else {
                    v6 = 0.2;
                    traffic::set_traffic_density(0.2);
                }
            } else {
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

void spawnable::do_spawn(vector3d a4, vector3d a2, traffic_path_lane *lane, int node_index, bool a10, bool a11)
{
    void(__fastcall * func)(void *, void *edx, vector3d, vector3d, traffic_path_lane *, int, bool, bool) =
        CAST(func, get_vfunc(m_vtbl, 0x0));
    func(this, nullptr, a4, a2, lane, node_index, a10, a11);
}

void spawnable::un_spawn()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this);
}

bool spawnable::is_viable_pos(const vector3d &a2)
{
    bool(__fastcall * func)(void *, void *edx, const vector3d *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this, nullptr, &a2);
}

actor *spawnable::get_my_actor()
{
    actor *(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    return func(this);
}

vector3d spawnable::prepare_for_spawn(traffic_path_graph::laneInfoStruct *next_lane_struct, vector3d &a4,
                                      int node_index)
{
    vector3d result = ZEROVEC;

    assert(next_lane_struct != nullptr);
    auto *v9 = this->get_my_actor();
    if (v9->sub_48AE20()) {
        vector3d v45{};
        auto *the_lane = next_lane_struct->field_4;
        if (the_lane->get_type()) {
            auto v12 = []() {
                return rand() * 0.000030518509;
            }();

            if (v12 >= 0.5f) {
                auto node = the_lane->get_node(1);
                auto v9 = the_lane->get_node(0);
                v45 = v9 - node;
                result = the_lane->get_node(0);
            } else {
                auto node = the_lane->get_node(0);
                auto v6 = the_lane->get_node(1);
                v45 = v6 - node;
                result = the_lane->get_node(1);
            }
        } else {
            auto num_nodes = the_lane->get_num_nodes();
            auto node = the_lane->get_node(num_nodes - 2);
            auto directional_node = the_lane->get_directional_node();
            v45 = directional_node - node;
            result = the_lane->get_directional_node();
        }

        v45.normalize();
        last_spawn_lane_info = next_lane_struct;
        if (this->is_viable_pos(a4)) {
            this->do_spawn(a4, v45, the_lane, node_index, next_lane_struct->field_E, true);
        }
    }

    return result;
}

traffic_path_graph::laneInfoStruct *spawnable::get_spawnable_lane(traffic_path_lane::eLaneType arg0, vector3d *arg4,
                                                                  traffic_path_graph **arg8, traffic_path_graph **argC,
                                                                  int *a6, bool a7, bool a8, bool a9)
{
    if (!spawnable_lanes || spawnable_lanes->empty())
        return nullptr;
    const int count = spawnable_lanes->size();
    int selected = static_cast<int>(std::rand() * (1.0 / 32768.0) * (count - 1));
    int attempts = 0;
    auto *camera = g_game_ptr->get_current_view_camera(0);
    vector3d camera_position = camera->get_abs_position();
    const float minimum_squared = var<float>(0x00937FA4) * var<float>(0x00937FA4);
    const float maximum_squared = flt_937FA8 * flt_937FA8;
    for (int visited = 0; visited < count; ++visited, selected = (selected + 1) % count) {
        auto *info = &(*spawnable_lanes)[selected];
        auto *lane = info->field_4;
        auto viable =
            reinterpret_cast<bool(__fastcall *)(spawnable *, void *, traffic_path_lane *)>(get_vfunc(m_vtbl, 0x14));
        if (!traffic_path_lane::lane_is_valid(lane) || !viable(this, nullptr, lane) || !lane->nodes ||
            !lane->is_valid(nullptr) || (last_spawn_lane_info && lane == last_spawn_lane_info->field_4))
            continue;
        const int segments = lane->get_num_nodes() - 1;
        const int start = static_cast<int>(std::rand() * (1.0 / 32768.0) * lane->get_num_nodes());
        for (int index = 0; index < segments; ++index) {
            if (++attempts > 12)
                return nullptr;
            const int node = (start + index) % segments;
            const vector3d first = lane->get_node(node);
            const vector3d next = lane->get_node(node + 1);
            camera_position.y = first.y;
            if (lane->get_type() != arg0 || !lane->has_room_for_me(10.0f, true, false) || lane->get_num_ais() > 5 ||
                (a9 && lane->is_clogged(true, false)) || (a7 && !info->field_C) || (a8 && !info->field_D))
                continue;
            if (!info->field_D && (a8 || a9))
                info->field_E = true;
            const auto actor_handle = info->field_E ? lane->last_ai() : lane->get_ai_for_actor();
            vector3d candidate;
            bool accepted = false;
            if (auto *other = actor_handle.get_volatile_ptr()) {
                candidate = other->get_abs_position();
                if (!lane->is_point_between_nodes(candidate, node, node + 1))
                    continue;
                vector3d direction = (info->field_E ? next : first) - candidate;
                if (direction.length2() <= EPSILON)
                    continue;
                direction.normalize();
                const double random = std::rand() * (1.0 / RAND_MAX);
                candidate +=
                    direction * static_cast<float>(((random * 2.0 - 1.0) * 0.25 + 1.75) * spawn_spacing * 10.0);
                if (!lane->is_point_between_nodes(candidate, node, node + 1))
                    continue;
                const vector3d remaining = info->field_E ? next - candidate : candidate - next;
                const vector3d segment = info->field_E ? next - first : first - next;
                const float dot = segment.x * remaining.x + segment.y * remaining.y + segment.z * remaining.z;
                if (dot <= 0.0f || remaining.xz_length2() < 100.0f)
                    accepted = !a9 && (candidate - camera_position).xz_length2() >= minimum_squared;
                else
                    accepted = (candidate - camera_position).xz_length2() >= minimum_squared;
            } else if ((camera_position - first).length2() <= maximum_squared ||
                       (camera_position - next).length2() <= maximum_squared) {
                vector3d hits[2];
                if (collide_segment_hollow_sphere(first, next, camera_position, flt_937FA8, hits) > 0) {
                    candidate = hits[0];
                    accepted = (candidate - first).length2() > 100.0f && (candidate - next).length2() > 100.0f;
                } else {
                    vector3d direction = first - next;
                    direction.normalize();
                    candidate = next + direction * 5.0f;
                    const int flags = visibility_flags(candidate, camera);
                    accepted = (flags & 8) && ((flags & 1) ? !(flags & 4) : !a9);
                }
            } else {
                accepted = closest_point_segment(camera_position, first, next, candidate);
            }
            if (accepted) {
                *arg4 = candidate;
                *arg8 = *argC = info->field_0;
                *a6 = node;
                return info;
            }
        }
    }
    return nullptr;
}

traffic_path_graph::laneInfoStruct *spawnable::get_new_spawn_pos(traffic_path_lane::eLaneType a1, vector3d *a2,
                                                                 traffic_path_graph **a3, traffic_path_graph **a4,
                                                                 int *a5)
{
    if (spawnable_lanes != nullptr && !spawnable_lanes->empty()) {
        return this->get_spawnable_lane(a1, a2, a3, a4, a5, false, false, true);
    }

    return nullptr;
}

void spawnable::sub_6B9B60(Float)
{
    if (!this->field_5) {
        auto current_view_camera = g_game_ptr->get_current_view_camera(0);
        if (current_view_camera != nullptr) {
            auto v7 = current_view_camera->get_abs_position();
            auto *v4 = this->get_my_actor();
            if (this->field_4) {
                auto v6 = v7 - v4->get_abs_position();
                auto v9 = v6.xz_length2();
                if (flt_937FA8 * flt_937FA8 * 1.21f < v9) {
                    this->un_spawn();
                    this->field_5 = true;
                }
            }
        }
    }
}

int spawnable::visibility_flags(const vector3d &position, entity_base *camera_entity)
{
    const auto &camera_po = camera_entity->get_abs_po();
    vector3d delta = position - camera_po.get_position();
    const float horizontal_squared = delta.xz_length2();
    if (delta.length2() > LARGE_EPSILON)
        delta.normalize();
    const auto forward = camera_po.get_z_facing();
    int result = delta.x * forward.x + delta.y * forward.y + delta.z * forward.z > 0.0f ? 1 : 0;
    const float minimum_squared = var<float>(0x00937FA4) * var<float>(0x00937FA4);
    if (horizontal_squared < minimum_squared * 0.25f)
        return result | 0xE;
    if (horizontal_squared < minimum_squared)
        return result | 0xC;
    if (horizontal_squared < flt_937FA8 * flt_937FA8)
        result |= 8;
    return result;
}

uint8_t spawnable::sub_6B9DD0()
{
    int flags = 0;
    if (!field_5)
        flags = visibility_flags(get_my_actor()->get_abs_position(), g_game_ptr->get_current_view_camera(0));

    static_cast<traffic *>(this)->field_8 = flags;
    return field_5 ? static_cast<uint8_t>(field_5) : static_cast<uint8_t>(flags);
}

void sub_6C2E10(Float a1)
{
    flt_937FF0 = std::clamp(static_cast<float>(a1), 0.0f, 1.0f);
    ;
}

void spawnable_patch()
{
    {
        REDIRECT(0x0055842F, spawnable::advance_traffic_and_peds);
    }
}
