#include "traffic_path_graph.h"
#include "entity.h"
#include "traffic_path.h"
#include "vector3d.h"
#include "collide.h"
#include "line_info.h"
#include <cfloat>
#include <cmath>
#include "traffic_path_brew.h"
#include "limited_timer.h"
#include "oldmath_po.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(traffic_path_graph::laneInfoStruct, 0x10);

traffic_path_graph::traffic_path_graph() {}

traffic_path_lane *traffic_path_graph::get_closest_or_farthest_lane(
    bool closest, const vector3d &position, const vector3d &direction, vector3d *point,
    traffic_path_lane::eLaneType type, bool check_collision, float *distance)
{
    float best = distance != nullptr ? *distance : closest ? FLT_MAX : -1.0f;
    traffic_path_lane *result = nullptr;
    const bool directional = !(direction.x <= 0.0f && direction.x >= 0.0f &&
                               direction.y <= 0.0f && direction.y >= 0.0f &&
                               direction.z <= 0.0f && direction.z >= 0.0f);
    auto facing = direction;
    if (directional)
        facing.normalize();
    auto **lanes = type == 0 ? vehicle_lanes : type == 1 ? pedestrian_lanes : parking_lanes;
    const int count = type == 0 ? vehicle_lane_count : type == 1 ? pedestrian_lane_count : parking_lane_count;
    for (int index = 0; index < count; ++index) {
        auto *lane = lanes[index];
        if (lane == nullptr || !lane->is_valid(this) || lane->get_type() != type)
            continue;
        float candidate = closest ? FLT_MAX : -1.0f;
        vector3d candidate_point = ZEROVEC;
        vector3d end = ZEROVEC;
        for (int node = 0; node < lane->get_num_nodes() - 1; ++node) {
            vector3d projected;
            const auto next = lane->get_node(node + 1);
            closest_point_segment(position, lane->get_node(node), next, projected);
            const float segment_distance = (projected - position).length2();
            if (closest ? segment_distance < candidate : segment_distance > candidate) {
                candidate = segment_distance;
                candidate_point = projected;
                end = next;
            }
        }
        auto toward_end = end - position;
        toward_end.normalize();
        if ((closest ? candidate < best : candidate > best) &&
            (!directional || dot(toward_end, facing) > 0.0f)) {
            *point = candidate_point;
            best = candidate;
            result = lane;
        }
    }
    if (result == nullptr && directional)
        return get_closest_or_farthest_lane(closest, position, ZEROVEC, point, type, check_collision, distance);
    if (result != nullptr && check_collision) {
        line_info query{position, *point};
        if (query.check_collision(*local_collision::entfilter_entity_no_capsules,
                                  *local_collision::obbfilter_lineseg_test, nullptr))
            result = nullptr;
    }
    if (result != nullptr && distance != nullptr)
        *distance = best;
    return result;
}
namespace {
traffic_path_intersection *visited_intersections[2]{};

vector3d approximate_intersection_position(traffic_path_intersection *intersection)
{
    vector3d sum = ZEROVEC;
    int count = 0;
    for (auto *road : intersection->roads) {
        if (road == nullptr)
            continue;
        const auto first = road->total_in_lanes != 0 ? road->in_lanes[0]->get_directional_node()
            : road->total_out_lanes != 0 ? road->out_lanes[0]->get_node(0) : road->all_lanes[0]->get_node(0);
        const auto last = road->total_out_lanes != 0 ? road->out_lanes[road->total_out_lanes - 1]->get_node(0)
            : road->total_in_lanes != 0 ? road->in_lanes[road->total_in_lanes - 1]->get_directional_node()
                                      : road->all_lanes[0]->get_node(0);
        sum += (first + last) * 0.5f;
        ++count;
    }
    return count != 0 ? sum * (1.0f / count) : sum;
}

void mark_intersection_roads(traffic_path_intersection *intersection, bool visited)
{
    if (intersection != nullptr)
        for (auto *road : intersection->roads)
            if (road != nullptr)
                road->flags = visited ? road->flags | 0x10 : road->flags & ~0x10;
}

void store_lane_information(traffic_path_road *road, traffic_path_graph *graph, const vector3d &camera_position,
                            _std::vector<traffic_path_graph::laneInfoStruct> *result, int priority)
{
    const bool parked = (road->flags & 2) != 0;
    for (unsigned index = 0; index < road->total_in_lanes + road->total_out_lanes; ++index) {
        auto *lane = road->all_lanes[index];
        const auto start = lane->get_node(0);
        auto lane_direction = lane->get_directional_node() - start;
        lane_direction.normalize();
        auto toward_camera = camera_position - start;
        toward_camera.normalize();
        result->push_back(traffic_path_graph::laneInfoStruct{
            graph, lane, priority, static_cast<char>(parked),
            static_cast<char>(dot(toward_camera, lane_direction) > std::cos(0.7853981852531433)), false, 0});
    }
}
}

void traffic_path_graph::get_spawnable_lane_list(
    entity *camera, _std::vector<laneInfoStruct> *result, Float, Float max_distance)
{
    for (auto *intersection : visited_intersections)
        mark_intersection_roads(intersection, false);
    visited_intersections[0] = visited_intersections[1] = nullptr;
    float nearest[2]{FLT_MAX, FLT_MAX};
    int visited_count = 0;
    const auto camera_position = camera->get_abs_position();
    auto forward = camera->get_abs_po().get_z_facing();
    forward.y = 0.0f;
    forward.normalize();
    for (int index = 0; index < intersection_manager->count; ++index) {
        auto *intersection = intersection_manager->intersections[index];
        auto toward_intersection = approximate_intersection_position(intersection) - camera_position;
        toward_intersection.y = 0.0f;
        const float distance = toward_intersection.length2();
        toward_intersection.normalize();
        if (dot(toward_intersection, forward) < 0.0f)
            continue;
        if (distance < nearest[0]) {
            visited_intersections[1] = visited_intersections[0];
            nearest[1] = nearest[0];
            visited_intersections[0] = intersection;
            nearest[0] = distance;
            mark_intersection_roads(intersection, true);
            if (visited_count < 2)
                ++visited_count;
        } else if (distance < nearest[1]) {
            mark_intersection_roads(visited_intersections[1], false);
            visited_intersections[1] = intersection;
            nearest[1] = distance;
            mark_intersection_roads(intersection, true);
            if (visited_count < 2)
                ++visited_count;
        }
    }
    for (int index = 0; index < visited_count; ++index)
        for (auto *road : visited_intersections[index]->roads)
            if (road != nullptr)
                store_lane_information(road, this, camera_position, result, index);
    int priority = visited_count;
    for (int index = 0; index < vehicle_lane_count; ++index) {
        auto *lane = vehicle_lanes[index];
        if (lane->lane_length > 500.0f) {
            const auto node = lane->get_node(lane->get_nearest_node_xz(camera_position));
            if ((camera_position - node).xz_length2() < max_distance.value * max_distance.value)
                store_lane_information(lane->my_road, this, camera_position, result, priority++);
        }
    }
}

namespace {
bool timer_expired(limited_timer *timer)
{
    return timer != nullptr && timer->elapsed() >= timer->field_4;
}

int un_mash_road(traffic_path_road *road, char *image, traffic_path_graph *graph)
{
    if (image != nullptr) {
        road->in_lanes = reinterpret_cast<traffic_path_lane **>(image + sizeof(traffic_path_road));
        road->out_lanes = road->in_lanes + road->total_in_lanes;
        road->all_lanes = road->out_lanes + road->total_out_lanes;
        return sizeof(traffic_path_road) + 2 * sizeof(traffic_path_lane *) *
            (road->total_in_lanes + road->total_out_lanes);
    }
    auto resolve_lane = [graph](traffic_path_lane *encoded) {
        const auto index = reinterpret_cast<std::intptr_t>(encoded);
        return index < 0 ? graph->vehicle_lanes[-index - 1] : graph->pedestrian_lanes[index - 1];
    };
    for (unsigned index = 0; index < road->total_in_lanes; ++index) {
        auto *lane = resolve_lane(road->in_lanes[index]);
        road->in_lanes[index] = lane;
        for (auto *connected : road->field_1C->roads)
            if (connected == road) {
                lane->my_road = road;
                break;
            }
    }
    for (unsigned index = 0; index < road->total_out_lanes; ++index)
        road->out_lanes[index] = resolve_lane(road->out_lanes[index]);
    for (unsigned index = 0; index < road->total_in_lanes + road->total_out_lanes; ++index)
        road->all_lanes[index] = resolve_lane(road->all_lanes[index]);
    return 0;
}

void release_intersection(traffic_path_intersection *intersection)
{
    for (auto &visited : visited_intersections)
        if (visited == intersection) {
            mark_intersection_roads(visited, false);
            visited = nullptr;
            break;
        }
    intersection->field_10 = nullptr;
    intersection->field_16 = -1;
}
}

bool traffic_path_graph::intersection_manager::un_mash(
    char *image, int *bytes, traffic_path_graph *graph, region *, intersection_manager_brew &brew)
{
    if (image == nullptr) {
        if (brew.field_0 != 2) {
            for (int index = 0; index < count; ++index)
                for (auto *road : intersections[index]->roads)
                    if (road != nullptr)
                        un_mash_road(road, nullptr, graph);
            brew.field_0 = 2;
        }
        return false;
    }
    if (brew.field_0 == 2)
        return false;
    if (brew.field_0 == 0) {
        brew.field_0 = 1;
        brew.cursor = image;
        brew.special_output = nullptr;
    }
    auto *cursor = brew.cursor;
    if (brew.header_state != 2) {
        intersections = reinterpret_cast<traffic_path_intersection **>(cursor + sizeof(*this));
        brew.special_output = &special_intersections;
        cursor += sizeof(*this) + sizeof(traffic_path_intersection *) * count;
        brew.header_state = 2;
    }
    if (brew.intersection_state != 2) {
        if (brew.intersection_state == 0) {
            brew.intersection_state = 1;
            brew.index = 0;
        }
        while (brew.index < count) {
            auto *intersection = reinterpret_cast<traffic_path_intersection *>(cursor);
            intersections[brew.index++] = intersection;
            intersection->field_10 = graph;
            cursor += sizeof(*intersection);
            for (auto &road : intersection->roads) {
                road = reinterpret_cast<traffic_path_road *>(cursor);
                cursor += un_mash_road(road, cursor, graph);
                if (road->total_in_lanes + road->total_out_lanes == 0)
                    road = nullptr;
            }
            if (timer_expired(brew.timer)) {
                brew.cursor = cursor;
                return true;
            }
        }
        brew.intersection_state = 2;
    }
    if (brew.road_state != 2) {
        if (brew.road_state == 0) {
            brew.road_state = 1;
            brew.index = 0;
        }
        while (brew.index < count) {
            for (auto *road : intersections[brew.index++]->roads)
                if (road != nullptr) {
                    road->field_18 = intersections[reinterpret_cast<std::uintptr_t>(road->field_18)];
                    road->field_1C = intersections[reinterpret_cast<std::uintptr_t>(road->field_1C)];
                }
            if (timer_expired(brew.timer)) {
                brew.cursor = cursor;
                return true;
            }
        }
        brew.road_state = 2;
    }
    if (brew.special_state != 2) {
        if (brew.special_state == 0) {
            brew.special_state = 1;
            *brew.special_output = reinterpret_cast<traffic_path_intersection **>(cursor);
            brew.index = 0;
        }
        while (brew.index < special_count) {
            const auto index = *reinterpret_cast<uint32_t *>(cursor);
            special_intersections[brew.index++] = intersections[index];
            cursor += sizeof(uint32_t);
            if (timer_expired(brew.timer)) {
                brew.cursor = cursor;
                return true;
            }
        }
        brew.special_state = 2;
    }
    *bytes = static_cast<int>(cursor - image);
    brew.field_0 = 2;
    return false;
}

void traffic_path_graph::intersection_manager::release_mem()
{
    if (field_10) {
        for (int index = 0; index < count; ++index)
            release_intersection(intersections[index]);
        intersections = nullptr;
        count = 0;
        for (int index = 0; index < special_count; ++index)
            release_intersection(special_intersections[index]);
    } else {
        for (int index = 0; index < count; ++index) {
            auto *intersection = intersections[index];
            for (auto *road : intersection->roads)
                if (road != nullptr) {
                    if ((road->flags & 1) == 0) {
                        ::operator delete[](road->in_lanes);
                        ::operator delete[](road->out_lanes);
                        ::operator delete[](road->all_lanes);
                    }
                    road->in_lanes = road->out_lanes = road->all_lanes = nullptr;
                    road->field_18 = road->field_1C = nullptr;
                    delete road;
                }
            delete intersection;
        }
        ::operator delete[](intersections);
        intersections = nullptr;
        count = 0;
    }
    special_intersections = nullptr;
    special_count = 0;
}

bool traffic_path_graph::un_mash(char *image, int *bytes, region *region, traffic_path_brew &brew)
{
    if (brew.field_0.is_done())
        return false;
    if (!brew.field_0.is_started()) {
        brew.field_0.start();
        brew.field_40 = image;
    }
    auto *cursor = brew.field_40;
    if (!brew.field_8.is_done()) {
        vehicle_lanes = pedestrian_lanes = parking_lanes = nullptr;
        intersection_manager = reinterpret_cast<struct intersection_manager *>(image + sizeof(*this));
        reg = region;
        cursor += sizeof(*this);
        brew.field_8.done();
    }
    if (brew.field_C.field_0 != 2) {
        int manager_bytes = 0;
        if (intersection_manager->un_mash(cursor, &manager_bytes, this, region, brew.field_C)) {
            brew.field_40 = cursor;
            return true;
        }
        cursor += manager_bytes;
    }
    auto un_mash_lanes = [&](traffic_path_lane **&lanes, int count, int &state) {
        if (state == 2)
            return false;
        if (state == 0) {
            state = 1;
            lanes = reinterpret_cast<traffic_path_lane **>(cursor);
            cursor += count * sizeof(traffic_path_lane *);
            brew.field_44 = 0;
        }
        while (brew.field_44 < count) {
            auto *lane = reinterpret_cast<traffic_path_lane *>(cursor);
            lanes[brew.field_44++] = lane;
            lane->nodes = reinterpret_cast<vector3d *>(cursor + sizeof(*lane));
            cursor += sizeof(*lane) + sizeof(vector3d) * lane->total_nodes;
            if (timer_expired(brew.field_4))
                return true;
        }
        state = 2;
        return false;
    };
    if (un_mash_lanes(vehicle_lanes, vehicle_lane_count, brew.vehicle_state) ||
        un_mash_lanes(pedestrian_lanes, pedestrian_lane_count, brew.pedestrian_state) ||
        un_mash_lanes(parking_lanes, parking_lane_count, brew.parking_state)) {
        brew.field_40 = cursor;
        return true;
    }
    if (brew.fixup_state != 2) {
        intersection_manager_brew fixup;
        intersection_manager->un_mash(nullptr, nullptr, this, region, fixup);
        *bytes = static_cast<int>(cursor - image);
        brew.fixup_state = 2;
    }
    brew.field_0.done();
    return false;
}

void traffic_path_graph::release_mem()
{
    if (intersection_manager != nullptr) {
        intersection_manager->release_mem();
        if (!field_20)
            delete intersection_manager;
    }
    auto release_lanes = [this](traffic_path_lane **lanes, int count) {
        for (int index = 0; index < count; ++index) {
            auto *lane = lanes[index];
            if (lane == nullptr && !field_20)
                continue;
            if ((lane->flags & 0x80) == 0)
                ::operator delete[](lane->nodes);
            lane->nodes = nullptr;
            lane->total_nodes = 0;
            lane->flags = 0;
            lane->lane_length = 0.0f;
            lane->my_road = nullptr;
            lane->field_10 = -1;
            if (field_20)
                lanes[index] = nullptr;
            else
                delete lane;
        }
        if (!field_20)
            ::operator delete[](lanes);
    };
    release_lanes(vehicle_lanes, vehicle_lane_count);
    release_lanes(pedestrian_lanes, pedestrian_lane_count);
    release_lanes(parking_lanes, parking_lane_count);
    reg = nullptr;
    vehicle_lanes = pedestrian_lanes = parking_lanes = nullptr;
    intersection_manager = nullptr;
}
