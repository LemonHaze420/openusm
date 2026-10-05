#include "traffic.h"

#include "ai_state_car.h"
#include "camera.h"
#include "conglom.h"
#include "event.h"
#include "event_manager.h"
#include "game.h"
#include "poi.h"
#include "region.h"
#include "terrain.h"
#include "traffic_path_graph.h"
#include "traffic_path_lane.h"
#include "traffic_signal_mgr.h"
#include "wds.h"
#include "parking_marker.h"
#include "ai_player_controller.h"
#include "attach_state.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "base_state.h"
#include "damage_inode.h"
#include "damage_interface.h"
#include "interactable_interface.h"
#include "interaction.h"
#include "physical_interface.h"
#include "sound_and_pfx_interface.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iterator>

namespace {
float traffic_angle(const vector3d &a, const vector3d &b)
{
    const float lengths = std::sqrt(a.length2()) * std::sqrt(b.length2());
    return (lengths <= 0.0f && lengths >= 0.0f) ? 0.0f : std::acos(std::clamp(dot(a, b) / lengths, -1.0f, 1.0f));
}


bool turn_intersection(const vector3d &a, const vector3d &b,
                       const vector3d &c, const vector3d &d, vector3d &out)
{
    const float dx = b.x - a.x, dz = b.z - a.z;
    const float ex = c.x - d.x, ez = c.z - d.z;
    const float numerator = (a.x - c.x) * ez - (a.z - c.z) * ex;
    const float denominator = dz * ex - ez * dx;
    if (std::abs(denominator) < EPSILON)
        return false;
    const auto coordinate = [&](float direction, float origin) {
        const float product = direction * numerator;
        const bool same_sign = (product > 0.0f && denominator > 0.0f) ||
            (product < 0.0f && denominator < 0.0f) ||
            (product <= 0.0f && product >= 0.0f && denominator <= 0.0f && denominator >= 0.0f);
        return origin + (product + denominator * (same_sign ? 0.5f : -0.5f)) / denominator;
    };
    out.x = coordinate(dx, a.x);
    out.z = coordinate(dz, a.z);
    return true;
}
}


void traffic::update_follow()
{
    if (auto *target = field_1E4.get_volatile_ptr()) {
        auto *car = get_traffic_from_entity(vhandle_type<entity>{field_1E4.field_0});
        field_1D4 = car ? car->field_14C : target->get_abs_position();
        field_1BD = false;
    }
}


void traffic::finish_goto()
{
    delete field_1E8;
    field_1E8 = nullptr;
    if (!field_1E4.get_volatile_ptr()) {
        field_1D4 = FARAWAY;
        field_1E0 = 0.0f;
        field_1BD = true;
    }
    if (field_1C8 & 1)
        event_manager::raise_event(event::FINISHED_TRAFFIC_GOTO, get_my_actor()->get_my_vhandle());
    if (field_1C8 & 8) {
        screeching_halt();
    } else if ((field_1C8 & 2) && field_15C != 13) {
        if (field_15C != 12)
            field_160 = field_15C;
        field_15C = 13;
    }
}


void traffic::pull_over_for_chase()
{
    vhandle_type<entity> handle{entity_base_vhandle{static_cast<uint32_t>(field_174)}};
    if (!handle.get_volatile_ptr())
        return;
    auto *chaser = get_traffic_from_entity(handle);
    if (!chaser)
        return;
    const float distance = (chaser->field_C.get_abs_position() - field_C.get_abs_position()).xz_length2();
    if (field_15C == 6) {
        if (chaser->field_140 == field_140 && is_fully_pulled_over() && distance < 400.0f && chaser == car_behind())
            chaser->pass_car_in_front(this);
        return;
    }
    update_facing_lane();
    const vector3d forward{field_1AC, field_1B0, field_1B4};
    const vector3d right{field_194, field_198, field_19C};
    const auto front = field_C.get_abs_position() + forward * 2.0f;
    if (distance < 900.0f || (front - field_14C).xz_length2() < 729.0f) {
        field_1EC.push_back(field_14C);
        field_1EC.push_back(front + forward * 20.0f);
        field_1EC.push_back(front + forward * 15.0f + right * 3.5f);
        field_15C = 6;
        field_14C = front + forward * 5.0f + right * 6.0f;
    }
}


void traffic::pass_car_in_front(traffic *)
{
    if ((field_15C != 3 && field_15C != 2) || field_178 > 0.0f)
        return;
    field_15C = 5;
    auto *front_car = get_traffic_from_entity(field_170);
    if (!front_car)
        return;
    auto *behind = car_behind();
    auto *ahead = front_car->car_ahead();
    if (behind)
        behind->sub_6DACB0(front_car->get_my_actor()->get_my_vhandle());
    front_car->sub_6DACB0(get_my_actor()->get_my_vhandle());
    const int index = front_car->field_16C;
    front_car->field_16C = field_16C;
    field_16C = index;
    const vhandle_type<actor> handle{get_my_actor()->get_my_vhandle()};
    field_140->remove_ai_from_lane(handle);
    field_140->add_ai_to_lane(handle, index);
    sub_6DACB0(ahead ? ahead->get_my_actor()->get_my_vhandle() : entity_base_vhandle{0});
    field_178 = 3.5f;
    update_facing_lane();
    const vector3d forward{field_1AC, field_1B0, field_1B4};
    const vector3d right{field_194, field_198, field_19C};
    const auto front = field_C.get_abs_position() + forward * 2.0f;
    field_1EC.push_back(field_14C);
    field_1EC.push_back(front + forward * 20.0f);
    field_1EC.push_back(front + forward * 15.0f - right * 2.25f);
    field_15C = 5;
    field_14C = front + forward * 5.0f - right * 2.75f;
}


void traffic::clear_chase_lane(traffic_path_lane *lane, bool same_lane, bool unspawn)
{
    for (int index = lane->get_num_ais() - 1; index >= 0; --index) {
        vhandle_type<entity> handle{lane->get_ai_by_index(index)};
        if (!handle.get_volatile_ptr())
            continue;
        auto *car = get_traffic_from_entity(handle);
        if (car && !car->field_1C4 && car->field_4 && !car->is_ai_potential_car() &&
            !vhandle_type<actor>{entity_base_vhandle{static_cast<uint32_t>(car->field_174)}}.get_volatile_ptr()) {
            if (unspawn)
                car->un_spawn();
            else
                car->yield_to_chase(this, same_lane);
        }
    }
}


void traffic::yield_to_chase(traffic *chaser, bool same_lane)
{
    if (chaser == this)
        return;
    const auto chase_handle = chaser->get_my_actor()->get_my_vhandle();
    const bool already_yielding = vhandle_type<actor>{entity_base_vhandle{static_cast<uint32_t>(field_174)}}.get_volatile_ptr() != nullptr;
    if (already_yielding && static_cast<uint32_t>(field_174) != chase_handle.field_0) {
        un_spawn();
        return;
    }
    if (field_15C == 6 || already_yielding)
        return;
    bool lane_yielding = false;
    for (int index = 0; index < field_140->get_num_ais(); ++index) {
        auto *car = get_traffic_from_entity(vhandle_type<entity>{field_140->get_ai_by_index(index)});
        if (car && vhandle_type<actor>{entity_base_vhandle{static_cast<uint32_t>(car->field_174)}}.get_volatile_ptr()) {
            lane_yielding = true;
            break;
        }
    }
    bool can_pull_over = !lane_yielding && !old_drivers[3] && field_140->get_num_nodes() <= 2 &&
        chaser == car_behind() && field_15C != 10 && field_15C != 11 && field_140->get_lane_position() != 1;
    if (can_pull_over) {
        update_facing_lane();
        const auto position = field_C.get_abs_position();
        const auto front = position + vector3d{field_1AC, field_1B0, field_1B4} * 2.0f;
        can_pull_over = (front - field_14C).xz_length2() > 625.0f &&
            (position - field_140->get_node(0)).xz_length2() > 625.0f;
    }
    if (can_pull_over) {
        field_174 = chase_handle.field_0;
        if (same_lane)
            pull_over_for_chase();
    } else if (field_140->my_road->total_out_lanes <= 1 || old_drivers[3]) {
        un_spawn();
    }
}

bool traffic::signal_requires_stop()
{
    const auto orientation = field_140->get_my_road(field_140->get_next_intersection(1))->n2;
    const int direction = orientation != 0 && orientation != 2;
    const int state = var<int>(0x00921D74);
    return direction == var<int>(0x0095C86C) ? state != 2 : state != 0;
}

bool traffic::signal_is_yellow()
{
    const auto orientation = field_140->get_my_road(field_140->get_next_intersection(1))->n2;
    const int direction = orientation != 0 && orientation != 2;
    const int state = var<int>(0x00921D74);
    return direction == var<int>(0x0095C86C) ? state == 1 : state == 0;
}

bool traffic::reserve_turn()
{
    if (!field_17C) {
        release_turn();
        return field_180;
    }
    field_188 = field_140->get_next_intersection(1);
    field_184 = field_17C;
    const vhandle_type<actor> handle{get_my_actor()->get_my_vhandle()};
    if (field_188->get_ai_index(handle) == -1) {
        field_188->add_ai(handle);
        if (field_188->get_ai_index(handle) == -1)
            return false;
    }
    if (field_188->has_stopsign(false)) {
        field_180 = field_188->reserve_stopsign(handle, 0);
        return field_180;
    }
    release_turn();
    field_188 = field_140->get_next_intersection(1);
    field_180 = field_188->reserve_turn(field_184, false);
    if (!field_180)
        field_188->remove_ai_from_intersection(handle, 0);
    return field_180;
}

void traffic::which_way_do_i_go()
{
    int direction = field_17C;
    if (field_1E8) {
        auto route_direction = field_17C;
        if (field_1E8->get_next_turn(field_140, &route_direction, &field_144)) {
            if (field_144 == field_140 && !field_1E8->get_next_turn(field_140, &route_direction, &field_144)) {
                delete field_1E8;
                field_1E8 = nullptr;
            }
            field_17C = route_direction;
            field_148 = field_144;
            return;
        }
        delete field_1E8;
        field_1E8 = nullptr;
    }
    auto *intersection = field_140->get_next_intersection(1);
    const bool randomize = !field_1E4.get_volatile_ptr();
    if (!randomize) {
        field_17C = static_cast<traffic_path_intersection::eDirection>(0);
        update_follow();
    }
    direction = 0;
    if (field_1C9) {
        direction = intersection->get_next_direction(field_140, &field_148, 1, 4, field_1D4, true, randomize);
    } else {
        traffic_path_road *roads[4]{};
        if (is_ai_car_occupied() && field_220 && intersection->get_allowed_ai_roads(field_140, roads))
            direction = field_220->get_next_direction(roads);
        if (!direction)
            direction = intersection->get_next_direction(field_140, &field_148, 1, 3, ZEROVEC, false, randomize);
    }
    field_17C = static_cast<traffic_path_intersection::eDirection>(direction);
    auto *graph = field_140->get_graph();
    field_144 = intersection->get_next_lane(field_C.get_abs_position(), direction, field_140, &graph, 1, false);
    if (field_1C4 && field_144) {
        auto *other = field_144->get_other_lane();
        auto *target = field_1E4.get_volatile_ptr() ?
            get_traffic_from_entity(vhandle_type<entity>{field_1E4.field_0}) : nullptr;
        if (!(field_1C4 == 1 && target && target->field_140 == field_144)) {
            if (field_1C4 == 1 && target && target->field_140 == other)
                field_144 = other;
            else if (other && field_144->get_free_space_sq() < other->get_free_space_sq())
                field_144 = other;
        }
    }
    field_148 = field_144;
}

bool traffic::_is_viable_lane(traffic_path_lane *lane)
{
    if (lane->is_clogged(old_drivers[2] > 0, false))
        return false;
    if ((lane->my_road->total_out_lanes <= 1 || old_drivers[3]) &&
        (lane->get_num_ais() || old_drivers[3]))
        return is_not_chase_lane(lane);
    return true;
}


void traffic::driver(Float dt)
{
    if (field_1C4 == -1) {
        field_C.field_BC = YVEC;
        field_C.drive(dt, 0.0f, 0.0f, true, true, true);
        auto *camera = g_game_ptr->get_current_view_camera(0);
        auto *graph = camera->get_primary_region()->get_traffic_path_graph();
        if (graph) {
            vector3d closest;
            auto *lane = graph->get_closest_or_farthest_lane(true, field_C.get_abs_position(), ZEROVEC,
                &closest, static_cast<traffic_path_lane::eLaneType>(0), false, nullptr);
            set_current_lane(lane, -1, true);
            field_144 = field_140;
            field_148 = field_140;
        }
        if (field_15C != 1)
            static_cast<conglomerate *>(get_my_actor())->field_110 &= ~1u;
    } else if (field_1C4 >= 0 && field_1C4 <= 2) {
        driver_x(dt);
    }
}


void traffic::driver_x(Float dt)
{
    if (!field_140 || !field_140->is_valid(nullptr))
        return;
    if (field_1C4) {
        field_1FC -= dt;
        clear_chase_lane(field_140, true, false);
        if (field_148 && field_17C)
            clear_chase_lane(field_148, false, false);
    }
    field_1C0 = 0;
    bool stop = false, slow = false, clear = false;
    if (field_1FC <= 0.0f && field_1C4) {
        if (!is_halted() && !is_halting() && !is_destroyed_halt()) {
            damage_obstacles();
            field_1FC = 0.2f;
        }
    } else {
        field_1BF = player_in_front();
    }
    if (field_1BF && field_1C4)
        field_1BF = false;
    field_C.set_horn(field_1BF);
    if (field_1BF || (!field_1C4 && !field_180 && poi_manager::near_violence_poi(field_C.get_abs_position()))) {
        stop = true;
    } else {
        auto *ahead = actor_ahead();
        if (ahead && (ahead->field_4 & 0x200))
            check_obstacle(ahead, stop, slow, clear, false);
        if (!stop && field_144 && (field_15C == 10 || field_15C == 11)) {
            auto *last = vhandle_type<actor>{field_144->get_ai_by_index(field_144->get_num_ais() - 1)}.get_volatile_ptr();
            if (last && last != get_my_actor() && last != ahead)
                check_obstacle(last, stop, slow, clear, false);
        }
    }
    if ((field_15C == 10 || field_15C == 11) && !field_1BF && stop) {
        auto *blocking = get_traffic_from_entity(vhandle_type<entity>{entity_base_vhandle{static_cast<uint32_t>(field_1C0)}});
        if (blocking && blocking->field_C.field_C8 < EPSILON) {
            slow = true;
            stop = false;
        }
    }
    if (vhandle_type<actor>{entity_base_vhandle{static_cast<uint32_t>(field_174)}}.get_volatile_ptr())
        pull_over_for_chase();
    drive_to_destination(dt, stop, slow);
}


void traffic::release_turn()
{
    if (field_188)
        field_188->remove_ai_from_intersection(vhandle_type<actor>{get_my_actor()->get_my_vhandle()}, 0);
    if (field_180) {
        if (field_188->has_stopsign(false))
            field_188->release_semaphore(false);
        else
            field_188->release_turn(field_184, false);
        field_180 = false;
    }
    field_188 = nullptr;
}


bool traffic::start_turn()
{
    const bool stopsign = field_140->get_next_intersection(1)->has_stopsign(false);
    if (!field_17C)
        which_way_do_i_go();
    const bool chase_lane = !is_not_chase_lane(field_140);
    if (!field_1C4 && old_drivers[2] + old_drivers[3] &&
        !traffic_list.empty() && !chase_lane) {
        release_turn();
        return false;
    }
    const auto wait = [&](int state) {
        if (stopsign)
            field_178 = 0.01f;
        else {
            release_turn();
            const int random_bit = static_cast<int>(std::rand() * 0.0006103515625);
            const float random_offset = random_bit + 0.01f;
            field_178 = (random_offset <= 0.0f && random_offset >= 0.0f) ? 1.5f : 0.0f;
        }
        field_15C = state;
        return false;
    };
    if (!field_17C)
        return wait(8);
    if (!field_144) {
        field_17C = static_cast<traffic_path_intersection::eDirection>(0);
        which_way_do_i_go();
        if (!field_144)
            return false;
    }
    if (chase_lane) {
        while (field_144->is_clogged(false, false)) {
            auto *car = get_traffic_from_entity(vhandle_type<entity>{
                field_144->get_ai_by_index(field_144->get_num_ais() - 1)});
            if (!car || car->field_C.field_C8 > 2.5f)
                break;
            if (!car->field_4) {
                release_turn();
                return false;
            }
            car->un_spawn();
        }
    }
    if (!field_1C4 && signal_requires_stop())
        return wait(8);
    if (!field_144 || field_144->is_clogged(false, false))
        return wait(9);
    if (!reserve_turn()) {
        if (!field_1C4)
            field_17C = static_cast<traffic_path_intersection::eDirection>(0);
        return wait(9);
    }
    if (field_17C == 2) {
        start_lane(field_144, false);
        return true;
    }
    const auto previous = field_140->get_node(field_168 - 2);
    const auto last = field_140->get_node(field_168 - 1);
    const auto first = field_144->get_node(0);
    const auto next = field_144->get_node(1);
    vector3d crossing;
    if ((last - first).xz_length2() >= 100.0f &&
        turn_intersection(previous, last, first, next, crossing) &&
        (crossing - last).xz_length2() < 2500.0f) {
        crossing.y = previous.y;
        field_14C = last + (crossing - last) / (field_C.field_E8 * field_C.field_E8 * 0.1953125f);
        field_15C = 10;
        field_1BC = false;
    } else {
        start_lane(field_144, true);
    }
    return true;
}


void traffic::check_lane_change()
{
    const auto forward = get_my_actor()->get_abs_po().get_z_facing();
    const auto front = field_C.get_abs_position() + forward * 2.0f;
    if (lane_changes_this_frame || field_1BF || field_178 > 0.0f ||
        (front - field_14C).xz_length2() <= 400.0f)
        return;
    auto *lane = field_140->get_other_lane();
    if (!lane)
        return;
    int insertion = 0;
    const auto rear_handle = lane->get_car_before_point(front, &insertion);
    entity_base_vhandle ahead_handle{0};
    if (insertion > 0)
        ahead_handle = lane->get_ai_by_index(insertion - 1);
    else if (lane->get_num_ais())
        ahead_handle = lane->get_ai_by_index(lane->get_num_ais() - 1);
    if (!field_1C4)
        return;
    auto *follow = field_1E4.get_volatile_ptr() ?
        get_traffic_from_entity(vhandle_type<entity>{field_1E4.field_0}) : nullptr;
    if (field_1C4 == 1 && follow && lane != follow->field_140)
        return;
    if (field_140->get_num_nodes() > 2)
        return;
    auto *blocking_actor = field_170.get_volatile_ptr();
    if (!blocking_actor)
        return;
    const auto blocking_position = blocking_actor->get_abs_position();
    if ((blocking_position - front).xz_length2() < 56.25f)
        return;
    auto *rear_actor = vhandle_type<actor>{rear_handle}.get_volatile_ptr();
    auto *ahead_actor = vhandle_type<actor>{ahead_handle}.get_volatile_ptr();
    if (rear_actor && (rear_actor->get_abs_position() - front).xz_length2() < 25.0f)
        return;
    const auto ahead_position = ahead_actor ? ahead_actor->get_abs_position() : lane->get_directional_node();
    if (ahead_actor && (ahead_position - front).xz_length2() < 25.0f)
        return;
    auto *blocking = get_traffic_from_entity(field_170);
    auto *ahead = get_traffic_from_entity(vhandle_type<entity>{ahead_handle});
    auto *rear = get_traffic_from_entity(vhandle_type<entity>{rear_handle});
    if ((blocking && blocking->field_15C == 4) || (ahead && ahead->field_15C == 4) ||
        (rear && rear->field_15C == 4))
        return;
    const float ratio = blocking && blocking->field_15C == 6 ? 1.75f : 1.25f;
    if (ahead && (front - ahead_position).xz_length2() <
        (front - blocking_position).xz_length2() * ratio * ratio)
        return;
    if ((front - g_game_ptr->get_current_view_camera(0)->get_abs_position()).xz_length2() > 2025.0f)
        return;
    auto destination = lane->get_node(field_168);
    int node = 0;
    lane->get_node_before_point(front, &node);
    if ((front - destination).xz_length2() <= 400.0f)
        return;
    const auto first = lane->get_node(0);
    const float distance = std::sqrt((front - first).xz_length2()) +
        std::min(15.0f, std::max(5.0f, field_C.field_C8 * 2.0f));
    destination = first + (destination - first).normalized() * distance;
    if (traffic_angle(destination - front, forward) > 2.3561945f)
        return;
    ++lane_changes_this_frame;
    if (!field_1C4)
        field_1BE = true;
    set_current_lane(lane, insertion, false);
    field_168 = node < lane->get_num_nodes() - 1 ? node + 1 : node;
    field_178 = 2.5f;
    field_14C = destination;
    field_1BC = false;
    field_15C = 4;
    if (rear)
        rear->sub_6DACB0(get_my_actor()->get_my_vhandle());
    sub_6DACB0(ahead_handle);
}


void traffic::drive_to_destination(Float dt, bool stop, bool slow)
{
    float acceleration = field_1C4 == 2 ? 0.5f : 0.33f;
    float braking = -acceleration;
    const auto &pose = get_my_actor()->get_abs_po();
    const auto forward = pose.get_z_facing();
    const auto right = pose.get_x_facing();
    const auto front = field_C.get_abs_position() + forward * 2.0f;
    const float distance = std::sqrt((front - field_14C).xz_length2());
    bool reached = field_1BC;
    if (reached)
        stop = true;
    if (field_15C == 13 && field_C.field_C8 < 0.1f && !field_1C9 && field_C.is_grounded()) {
        field_C.field_C8 = 0.0f;
        field_C.audio_advance(dt);
        return;
    }
    float speed_scale = field_1B8;
    if (auto *target = field_1E4.get_volatile_ptr()) {
        auto *car = get_traffic_from_entity(vhandle_type<entity>{field_1E4.field_0});
        const float follow_distance = std::sqrt((target->get_abs_position() - front).xz_length2());
        if (car && car->field_15C == 12 && follow_distance < 15.0f) {
            const bool was_halting = field_15C == 12;
            screeching_halt();
            if (was_halting)
                field_1D0 *= 0.5f;
        } else if (follow_distance > 30.0f) {
            speed_scale *= 2.0f;
        } else if (follow_distance > 3.0f) {
            const float t = follow_distance - 3.0f;
            speed_scale *= (1.0f - t) * (1.0f / 27.0f) + t * (2.0f / 27.0f);
        } else if (follow_distance < 1.5f) {
            speed_scale = 0.0f;
            stop = true;
        }
    }
    float target_speed = speed_scale * 20.0f;
    const float wheel = field_C.field_E8;
    const float arrival = std::max(1.0f, speed_scale) * wheel * wheel * wheel * 0.030517576f;
    if (field_15C == 10) {
        if (field_17C != 2)
            target_speed = speed_scale * 15.0f;
    }
    if (field_15C != 10 && field_15C != 11 && field_15C != 4 &&
        !(field_140->flags & 0x800) && field_140->get_num_nodes() <= 2) {
        if (distance < 3.0f * arrival)
            field_1BC = true;
        if (distance < 5.0f * arrival)
            reached = true;
    } else if (distance < 1.5f * arrival) {
        field_1BC = true;
    }
    if (field_140->flags & 0x800)
        target_speed *= 0.65f;
    if (field_15C != 10 && field_15C != 11 && field_15C != 4 &&
        (front - field_1D4).xz_length2() < field_1E0)
        finish_goto();
    const float approach = field_C.field_C8 * 0.2f >= 1.0f ? field_C.field_C8 * 2.0f : 10.0f;
    auto *intersection = field_140->get_next_intersection(1);
    if (field_15C == 3 || field_15C == 2) {
        if (!field_17C && !field_1E4.get_volatile_ptr())
            which_way_do_i_go();
        if (distance < approach) {
            if (field_1C4) {
                if (field_17C != 2)
                    target_speed *= 0.75f;
            } else if (field_168 < field_140->get_num_nodes() - 1) {
                target_speed *= field_140->flags & 0x800 ? 0.5f : 0.75f;
            } else if (intersection->has_stopsign(false) || signal_requires_stop()) {
                target_speed *= intersection->has_stopsign(false) ? 0.22f : 0.25f;
                if (distance < approach * 0.5f)
                    braking = -0.66f;
                if (distance < approach * 0.25f && field_C.field_C8 > 10.0f)
                    braking = -1.0f;
            } else if (signal_is_yellow()) {
                target_speed *= 0.33f;
                if (distance < approach * 0.5f)
                    braking = -0.66f;
            } else {
                target_speed *= field_17C != 2 ? 0.5f : 0.75f;
            }
        }
    }
    if (stop) {
        target_speed = 0.0f;
        braking = field_1BF ? -1.0f : -0.5f;
    } else if (slow) {
        target_speed = speed_scale * 2.0f;
        braking = -0.5f;
    }
    if (field_15C == 8 || field_15C == 9)
        target_speed = 0.0f;
    if (field_1C4)
        target_speed *= 1.33f;
    if (field_C.field_C8 < 0.25f && target_speed < 0.25f && !field_1C9 && field_C.is_grounded()) {
        field_C.field_C8 = 0.0f;
        field_C.audio_advance(dt);
        return;
    }
    const float minimum_throttle = std::abs(field_C.field_C8) < 1.0f ? 0.1f : 0.0f;
    float throttle = 0.0f;
    if (field_C.field_C8 < target_speed)
        throttle = std::min(acceleration, std::max(minimum_throttle, (target_speed - field_C.field_C8) * 0.1f));
    else if (field_C.field_C8 > target_speed)
        throttle = std::max(braking * speed_scale,
            std::min(-minimum_throttle, (target_speed - field_C.field_C8) * 0.1f));
    const auto delta = field_14C - front;
    float angle = traffic_angle(delta, forward);
    if (angle < (10.0f - std::clamp(distance / 3.0f, 0.0f, 10.0f)) * 0.017453292f)
        angle = 0.0f;
    if (dot(right, delta) < 0.0f)
        angle = -angle;
    float steering = 0.0f;
    if (std::abs(angle) > 2.3561945f && distance < 15.0f) {
        field_1BC = true;
        if (field_C.field_C8 > EPSILON)
            throttle = -1.0f;
    } else if (std::abs(angle) >= (field_15C == 11 ? 0.0f : 0.017453292f)) {
        steering = std::abs(angle) * 1.9098593f;
    }
    if (((field_8 & 1) && (field_8 & 4)) || field_1C9) {
        if (angle < 0.0f)
            steering = -steering;
        if (reached)
            steering = 0.0f;
        steering = std::clamp(steering, field_C.field_F0 - dt * 7.5f, field_C.field_F0 + dt * 7.5f);
        field_C.field_F0 = steering;
        if (field_1CC && field_140->get_num_nodes() <= 2 && (field_15C == 3 || field_15C == 2)) {
            const int index = 8 - field_1CC--;
            static constexpr float weave[8]{0.5f, 1.0f, 0.5f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
            steering = field_1D0 * weave[index] * 0.5f;
            if (field_C.field_C8 > 15.0f)
                steering *= 15.0f / field_C.field_C8;
        }
        if (field_15C == 12) {
            if (field_C.field_C8 <= EPSILON)
                throttle = steering = 0.0f;
            else {
                throttle = -1.0f;
                if (field_C.field_C8 <= 1.0f) {
                    steering = 0.0f;
                    field_C.field_F4 = 1.5f;
                } else {
                    steering = field_1D0;
                }
            }
        } else if (field_15C == 13) {
            throttle = field_C.field_C8 <= EPSILON ? 0.0f : -0.5f;
            steering = 0.0f;
        }
        if ((field_15C == 12 || field_15C == 13) && (field_1C8 & 4)) {
            update_follow();
            if ((front - field_1D4).xz_length2() >= field_1E0)
                field_15C = field_160;
        }
        field_C.field_BC = YVEC;
        if (field_15C != 10 && field_15C != 11) {
            update_facing_lane();
            const vector3d up{field_1A0, field_1A4, field_1A8};
            if (up.length2() > EPSILON)
                field_C.field_BC = up;
        }
        if (field_140->flags & 0x20)
            field_C.manage_vehicle_height(true);
        field_C.drive(dt, throttle, steering, field_1C4 != 0,
            (field_8 & 1) || field_1C9, ((field_8 & 1) && (field_8 & 2)) || field_1C9);
    } else {
        field_C.field_BC = YVEC;
        if (field_15C != 10 && field_15C != 11) {
            update_facing_lane();
            const vector3d up{field_1A0, field_1A4, field_1A8};
            if (up.length2() > EPSILON)
                field_C.field_BC = up;
        }
        if (field_140->flags & 0x20)
            field_C.manage_vehicle_height(true);
        field_C.drive_to(dt, target_speed, field_14C, true, field_8 & 1);
    }
}


void traffic::_do_spawn(vector3d position, vector3d facing, traffic_path_lane *lane,
                        int node_index, bool first, bool moving)
{
    set_current_lane(lane, static_cast<int>(first) - 1, true);
    auto *owner = get_my_actor();
    event_manager::raise_event(event::RESPAWNED_THIS_FRAME, owner->get_my_vhandle());
    field_C.set_collidable(true);
    owner->set_fade_distance(90.0f);
    field_C.set_visible(true);
    if (!is_ai_potential_car() && field_4)
        field_C.pick_body_and_color();
    field_C.field_C8 = !moving || first ? 0.0f :
        ((std::rand() / 32767.0f) * 2.0f - 1.0f) * 2.5f + 10.0f;
    field_17C = static_cast<traffic_path_intersection::eDirection>(0);
    field_15C = moving ? 1 : 3;
    field_1BC = moving;
    field_1BD = true;
    if (!first && field_140->get_num_nodes() <= 2)
        field_168 = 0;
    else if (node_index == -1)
        field_140->get_node_before_point(position, &field_168);
    else
        field_168 = node_index;
    if (moving) {
        const int start = std::min(field_168, field_140->get_num_nodes() - 2);
        const auto from = field_140->get_node(start);
        const auto to = field_140->get_node(start + 1);
        facing = (to - from).normalized();
        float distance = std::sqrt((position - from).xz_length2());
        if (first)
            distance -= 5.0f;
        position = from + facing * distance;
    }
    if (field_140) {
        po pose;
        pose.set_po(facing, YVEC, position);
        owner->set_allow_tunnelling_into_next_frame(true);
        entity_set_abs_po(owner, pose);
    }
    field_5 = false;
    if (!moving && node_index != -1)
        ++field_168;
    field_C.set_collidable(true);
    owner->invalidate_frame_delta();
    owner->compute_sector(g_world_ptr->get_the_terrain(), false, nullptr);
}


void traffic::advance_fade()
{
    if (field_20C == 255)
        return;
    field_20C = field_20C <= 220 ? static_cast<uint8_t>(field_20C + var<uint8_t>(0x0093826C)) : 255;
    const float alpha = field_20C * 0.0039215689f;
    auto *owner = static_cast<conglomerate *>(get_my_actor());
    for (auto *member : owner->members) {
        if (member->is_an_entity() && member->field_40 == 255 &&
            static_cast<int32_t>(member->field_4) >= 0)
            static_cast<entity *>(member)->set_render_alpha_mod(alpha);
    }
}


void traffic::advance_parked(Float)
{
    advance_fade();
    const auto camera = g_game_ptr->get_current_view_camera(0)->get_abs_position();
    const float distance = var<float>(0x00937FA8) * 1.05f;
    if (!vhandle_type<parking_marker>{entity_base_vhandle{static_cast<uint32_t>(field_228)}}.get_volatile_ptr() ||
        (camera - get_my_actor()->get_abs_position()).xz_length2() > distance * distance) {
        --parked_cars;
        unspawn_parked();
    }
}


void traffic::advance(Float dt)
{
    if (!var<int>(0x0096CA10) && var<bool>(0x0096CA0C))
        return;
    if (field_1C4 >= -1 && field_1C4 <= 3)
        ++new_drivers[field_1C4 + 1];
    if (vhandle_type<parking_marker>{entity_base_vhandle{static_cast<uint32_t>(field_228)}}.get_volatile_ptr()) {
        ++parked_cars;
        advance_parked(dt);
        return;
    }
    if (field_1C4 == 3 || !get_my_actor())
        return;
    field_178 -= dt;
    _critical_processing(dt);
    sub_6B9DD0();
    advance_fade();
    if (field_8 & 1)
        ++visible_cars;
    if (field_5)
        return;
    if (field_140) {
        ++living_cars;
        const bool keep = (field_8 & 1) ||
            ((visible_cars >= 8 || unspawned_this_frame >= 1 || (field_8 & 4) || field_15C == 1) &&
             traffic_density * 30.0f >= parked_cars + living_cars) ||
            is_ai_potential_car() || !field_4;
        const bool retained = keep &&
            (((traffic_enabled || is_ai_potential_car()) &&
              (get_my_actor()->get_primary_region() || field_15C == 1)) || !field_4);
        if (!retained) {
            if (!keep)
                ++unspawned_this_frame;
            un_spawn();
            po pose;
            pose.set_po(ZVEC, YVEC, vector3d{0.0f, -99.0f, 0.0f});
            get_my_actor()->set_allow_tunnelling_into_next_frame(true);
            entity_set_abs_po(get_my_actor(), pose);
            --living_cars;
        }
    }
    if (!field_5 && field_140) {
        update_destination(dt);
        driver(dt);
    }
}


void traffic::damage_obstacles()
{
    const auto forward = get_my_actor()->get_abs_po().get_z_facing();
    const auto impact_position = field_C.get_abs_position() + forward * field_C.field_E8;
    float radius = field_C.bodytype == 1 ? 3.5f : 4.5f;
    if (field_C.field_C8 > 5.0f)
        radius += field_C.field_C8 * 0.06666667f;
    auto *primary = g_world_ptr->get_the_terrain()->find_region(impact_position, nullptr);
    if (!primary)
        return;
    region_array regions{};
    build_region_list_radius(&regions, primary, impact_position, radius, true);
    const float radius_sq = radius * radius;
    const auto disable_car_interactions = [](actor *owner) {
        if (!owner || !owner->m_interactable_ifc)
            return;
        for (auto *interaction : owner->m_interactable_ifc->field_4) {
            if (interaction->field_28 == interaction_type_enum{2}) {
                interaction->field_44 = false;
                interaction->field_38 = 0.0f;
            }
        }
    };
    for (int index = 0; index < regions.count; ++index) {
        auto &entities = *static_cast<_std::list<entity *> *>(regions.m_data[index]->region_entities);

        auto iterator = entities.empty() ? entities.end() : std::prev(entities.end());
        while (iterator != entities.end()) {
            auto *other = *iterator;
            iterator = iterator == entities.begin() ? entities.end() : std::prev(iterator);
            if (!other || !(other->field_4 & 0x200) || (other->field_4 & 0x800))
                continue;
            const auto delta = other->get_abs_position() - impact_position;
            if (dot(forward.normalized(), delta) <= 0.8f || delta.length2() > radius_sq ||
                !other->has_damage_ifc() || !other->is_alive())
                continue;
            auto *damage = other->damage_ifc();
            float last_damage;
            std::memcpy(&last_damage, &damage->field_1DC, sizeof(last_damage));
            if (g_world_ptr->time_manager.field_8 - last_damage < 6.0f && damage->field_104.field_28 == 6)
                continue;
            bool damage_hero = true;
            if (other->is_hero()) {
                auto *core = other->get_ai_core();
                damage_hero = core->my_base_machine->my_curr_state->get_name() != ai::attach_state::default_id;
                if (damage_hero) {
                    auto &disabled_handle = var<int>(0x0096CA78);
                    auto *disabled = vhandle_type<actor>{entity_base_vhandle{static_cast<uint32_t>(disabled_handle)}}.get_volatile_ptr();
                    auto *owner = get_my_actor();
                    if (!disabled && owner->m_interactable_ifc &&
                        owner->m_interactable_ifc->has_enabled_interaction_of_this_kind(interaction_type_enum{2})) {
                        disable_car_interactions(owner);
                        disable_car_interactions(static_cast<actor *>(g_world_ptr->get_hero_ptr(0)));
                        disabled_handle = owner->get_my_vhandle().field_0;
                        var<float>(0x0096C9D4) = 2.0f;
                    } else if (static_cast<uint32_t>(disabled_handle) == owner->get_my_vhandle().field_0) {
                        var<float>(0x0096C9D4) = 2.0f;
                    }
                }
                if (other->has_sound_and_pfx_ifc())
                    other->sound_and_pfx_ifc()->play_sound_grp(string_hash{"PAIN"}, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
            }
            if (!damage_hero)
                continue;
            const float amount = std::min(25.0f, field_C.field_C8 * field_C.field_C8 * 0.06f);
            if (amount < 1.0f)
                continue;
            if (other->has_physical_ifc()) {
                const auto &pose = get_my_actor()->get_abs_po();
                const auto impulse = (pose.get_y_facing() * 3.0f - pose.get_z_facing()).normalized() *
                    std::max(20.0f, amount * 2.0f);
                other->physical_ifc()->apply_force_increment(impulse,
                    static_cast<physical_interface::force_type>(1), var<vector3d>(0x00938184), 0);
            }
            const string_hash attack{amount < 2.5f ? "Wounded_Upper" : "Enter_Prop_Physics"};
            auto *core = other->is_an_actor() ? other->get_ai_core() : nullptr;
            if (amount >= 2.5f && core) {
                auto *node = static_cast<ai::damage_inode *>(core->get_info_node(ai::damage_inode::default_id, true));
                auto *hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0));
                const bool car_damage = other->is_hero() && hero && hero->get_player_controller()->m_hero_type == 1;
                node->apply_forced_damage(static_cast<int>(amount), delta,
                    car_damage ? string_hash{"Enter_Prop_Physics_Car_Damage"} : attack, true);
            } else {
                damage->apply_damage(other, amount, 6, impact_position, delta * 3.0f, 0,
                    attack, string_hash{}, string_hash{}, false, ZEROVEC, 17, false);
            }
        }
    }
}
