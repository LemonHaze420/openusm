#include "traffic_path_lane.h"

#include "common.h"
#include "func_wrapper.h"
#include "traffic_path.h"
#include "trace.h"
#include "utility.h"
#include "vector3d.h"
#include "traffic.h"
#include "ped_spawner.h"
#include "poi.h"
#include "oldmath_po.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

VALIDATE_SIZE(traffic_path_lane, 0x14);

traffic_path_graph *traffic_path_lane::get_graph() const
{
    return my_road->field_1C->field_10;
}

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

namespace {

bool seed_line_intersection(const vector3d &a, const vector3d &b, const vector3d &c, const vector3d &d,
                            vector3d &out)
{
    if (std::max(a.x, b.x) < std::min(c.x, d.x) || std::max(c.x, d.x) < std::min(a.x, b.x) ||
        std::max(a.z, b.z) < std::min(c.z, d.z) || std::max(c.z, d.z) < std::min(a.z, b.z))
        return false;
    const float ax = b.x - a.x;
    const float az = b.z - a.z;
    const float bx = c.x - d.x;
    const float bz = c.z - d.z;
    const float cx = a.x - c.x;
    const float cz = a.z - c.z;
    const float numerator = cx * bz - cz * bx;
    const float denominator = az * bx - bz * ax;
    if (denominator <= 0.0f ? numerator > 0.0f || numerator < denominator
                            : numerator < 0.0f || numerator > denominator)
        return false;
    const float other_numerator = cz * ax - cx * az;
    if (denominator <= 0.0f ? other_numerator > 0.0f || other_numerator < denominator
                            : other_numerator < 0.0f || other_numerator > denominator)
        return false;
    if (std::fabs(denominator) < 0.0001f)
        return false;
    auto rounded_coordinate = [denominator](float product) {
        const bool same_sign = (product > 0.0f && denominator > 0.0f) ||
                               (product < 0.0f && denominator < 0.0f);
        return (product + denominator * (same_sign ? 0.5f : -0.5f)) / denominator;
    };
    out = vector3d{a.x + rounded_coordinate(numerator * ax), 0.0f,
                   a.z + rounded_coordinate(numerator * az)};
    return true;
}

bool seed_boundary_intersection(const vector3d &start, const vector3d &end, const vector3d &left,
                                const vector3d &right, bool exclude_start, bool exclude_end,
                                vector3d &position)
{
    if (!seed_line_intersection(left, right, start, end, position))
        return false;
    if ((position - start).length2() < 9.0f) {
        if (exclude_start)
            return false;
        auto direction = end - start;
        direction.normalize();
        position = start + direction * 3.0f;
    } else if ((position - end).length2() < 9.0f) {
        if (exclude_end)
            return false;
        auto direction = start - end;
        direction.normalize();
        position = end + direction * 3.0f;
    }
    return true;
}

ped_spawner *available_ped_spawner()
{
    for (auto *spawner : ped_spawner::ped_spawner_list)
        if (spawner->field_5)
            return spawner;
    return nullptr;
}

}

bool pedestrian_seed_blocked(entity &camera, const vector3d &position)
{
    auto toward = position - camera.get_abs_position();
    if (toward.length2() < 35.0f * 35.0f) {
        toward.normalize();
        if (dot(toward, camera.get_abs_po().get_z_facing()) > 0.65f)
            return true;
    }
    poi_manager::check_init();
    if (poi_manager::poi_list != nullptr)
        for (int index = 0; index <= dword_938004; ++index) {
            auto *poi = poi_manager::poi_list[index];
            if (poi != nullptr && poi->field_C == 3 &&
                (poi->get_location() - position).xz_length2() <= poi->field_18 * poi->field_18)
                return true;
        }
    return false;
}

void traffic_path_lane::seed_with_pedestrians(entity &camera, int requested)
{
    if (get_num_ais() == 20)
        return;
    const auto camera_position = camera.get_abs_position();
    auto forward = camera.get_abs_po().get_z_facing();
    forward.y = 0.0f;
    forward.normalize();
    const vector3d right{forward.z, 0.0f, -forward.x};
    for (int node = 0; node < total_nodes - 1; ++node) {
        if (requested == 0)
            return;
        const auto original_start = get_node(node);
        const auto original_end = get_node(node + 1);
        auto start = original_start;
        auto end = original_end;
        start.y = end.y = 0.0f;
        auto toward_start = start - camera_position;
        auto toward_end = end - camera_position;
        toward_start.y = toward_end.y = 0.0f;
        const float start_distance = toward_start.length2();
        const float end_distance = toward_end.length2();
        toward_start.normalize();
        toward_end.normalize();
        const float start_dot = dot(forward, toward_start);
        const float end_dot = dot(forward, toward_end);
        vector3d position;
        if (start_dot > 0.5f && end_dot > 0.5f) {
            if (start_distance >= 2500.0f && end_distance >= 2500.0f)
                continue;
            bool endpoint_selected = false;
            if (start_dot > 0.9397f && end_dot > 0.9397f) {
                if (start_distance < end_distance && start_distance > 1600.0f && start_distance < 2500.0f) {
                    auto direction = end - start;
                    direction.normalize();
                    position = start + direction * 3.0f;
                    endpoint_selected = true;
                } else if (end_distance < start_distance && end_distance > 1600.0f && end_distance < 2500.0f) {
                    auto direction = start - end;
                    direction.normalize();
                    position = end + direction * 3.0f;
                    endpoint_selected = true;
                }
            }
            if (!endpoint_selected) {
                auto center = camera_position + forward * 40.0f;
                center.y = 0.0f;
                if (!seed_boundary_intersection(start, end, center - right * 15.0f, center + right * 15.0f,
                                                false, false, position))
                    continue;
            }
        } else {
            if ((start_dot <= 0.0f && end_dot <= 0.0f) || (start_dot >= 0.5f && end_dot >= 0.5f))
                continue;
            float angle = 1.0471976f;
            auto side = -right;
            if (dot((start + end) * 0.5f - camera_position, right) < 0.0f) {
                angle = -angle;
                side = right;
            }
            const float sine = std::sin(angle);
            const float cosine = std::cos(angle);
            const vector3d rotated{forward.x * cosine + forward.z * sine, 0.0f,
                                   forward.z * cosine - forward.x * sine};
            auto boundary_start = camera_position - forward * 4.0f + side * 3.0f;
            auto boundary_end = camera_position + rotated * 25.0f;
            boundary_start.y = boundary_end.y = 0.0f;
            const bool intersected = seed_boundary_intersection(start, end, boundary_start, boundary_end,
                                                                start_dot > 0.707f, end_dot > 0.707f, position);
            if (!intersected) {
                if (start_dot > end_dot && start_dot > 0.0f && start_dot < 0.5f && start_distance < 625.0f) {
                    auto direction = end - start;
                    direction.normalize();
                    position = start + direction * 3.0f;
                } else if (end_dot > start_dot && end_dot > 0.0f && end_dot < 0.5f && end_distance < 625.0f) {
                    auto direction = start - end;
                    direction.normalize();
                    position = end + direction * 3.0f;
                } else {
                    continue;
                }
                if ((position - vector3d{camera_position.x, 0.0f, camera_position.z}).length2() <= 9.0f)
                    continue;
            }
        }
        const float length = std::sqrt((end - start).xz_length2());
        if (length > 0.0f)
            position = original_start + (original_end - original_start) * ((start - position).length() / length);
        if (pedestrian_seed_blocked(camera, position))
            continue;
        const int occupants = get_num_ais();
        bool crowded = false;
        for (int index = 0; index < occupants; ++index)
            if (auto *actor = get_ai_by_index(index).get_volatile_ptr())
                if ((actor->get_abs_position() - position).length2() < 16.0f) {
                    crowded = true;
                    break;
                }
        if (crowded)
            continue;
        auto facing = start - position;
        facing.normalize();
        const vector3d lateral{facing.z, 0.0f, -facing.x};
        if (requested > 1 && occupants < 19 &&
            static_cast<int>(std::rand() * 0.0030517578125) < 10) {
            if (auto *spawner = available_ped_spawner()) {
                spawner->spawnable::do_spawn(position - lateral * 0.5f, facing, this, -1, false, true);
                --requested;
            }
            if (auto *spawner = available_ped_spawner()) {
                spawner->spawnable::do_spawn(position + lateral * 0.5f, facing, this, -1, false, true);
                --requested;
            }
        } else if (occupants < 20) {
            if (auto *spawner = available_ped_spawner()) {
                float offset = (std::rand() * 0.00006103515625 - 1.0) * 0.25 + 0.3;
                if ((occupants & 1) == 0)
                    offset = -offset;
                spawner->spawnable::do_spawn(position + lateral * offset, facing, this, -1, false, true);
                --requested;
            }
        }
    }
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


vector3d traffic_path_lane::get_node_before_point(const vector3d &position, int *index)
{
    int before = 0;
    const float length_squared = lane_length * lane_length;
    if (total_nodes > 2) {
        int nearest = 0;
        float nearest_distance = length_squared;
        for (int node = 1; node < total_nodes; ++node) {
            const float distance = (position - nodes[node]).length2();
            if (distance < nearest_distance) {
                nearest_distance = distance;
                nearest = node;
            }
        }
        if (nearest >= total_nodes - 1)
            before = nearest - 1;
        else if (nearest >= 1) {
            const auto segment = nodes[nearest] - nodes[nearest + 1];
            const auto offset = position - nodes[nearest + 1];
            before = offset.x * offset.x + offset.z * offset.z >=
                     segment.x * segment.x + segment.z * segment.z ? nearest - 1 : nearest;
        }
    } else {
        for (int node = 1; node < total_nodes; ++node) {
            if ((nodes[node] - nodes[0]).length2() >= length_squared)
                break;
            if ((position - nodes[node]).length2() < length_squared)
                before = node;
        }
    }
    *index = before;
    return nodes[before];
}

void traffic_path_lane::remove_ai_from_lane(vhandle_type<actor> actor_handle)
{
    if (field_10 < 0)
        return;
    auto &list = traffic_ai_list::ai_lists[field_10];
    list.remove_ai(actor_handle);
    if (list.num_ais == 0) {
        list.release();
        field_10 = -1;
    }
}

int traffic_path_lane::add_ai_to_lane(vhandle_type<actor> actor_handle)
{
    if (field_10 < 0) {
        field_10 = static_cast<int16_t>(traffic_ai_list::allocate(1, this));
        if (field_10 < 0)
            return 0;
    }
    auto *owner = actor_handle.get_volatile_ptr();
    if (owner == nullptr || !owner->is_an_actor())
        return -1;
    auto &list = traffic_ai_list::ai_lists[field_10];
    list.ais[list.num_ais++] = actor_handle;
    return list.num_ais - 1;
}

void traffic_path_lane::update_lane_indexes()
{
    if (field_10 < 0)
        return;
    auto &list = traffic_ai_list::ai_lists[field_10];
    for (int index = 0; index < list.num_ais; ++index) {
        if (list.ais[index].get_volatile_ptr()) {
            if (auto *car = traffic::get_traffic_from_entity(
                    vhandle_type<entity>{list.ais[index].field_0}))
                car->set_lane_position_index(index, static_cast<traffic_path_lane *>(list.owner));
        }
    }
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

int traffic_path_lane::add_ai_to_lane(vhandle_type<actor> actor_handle, int index)
{
    if (field_10 < 0) {
        field_10 = static_cast<int16_t>(traffic_ai_list::allocate(1, this));
        if (field_10 < 0)
            return 0;
    }
    auto &list = traffic_ai_list::ai_lists[field_10];
    if (!actor_handle.get_volatile_ptr()->is_an_actor() || list.type != 1)
        return 0;
    for (int destination = list.num_ais; destination > index; --destination) {
        list.ais[destination] = list.ais[destination - 1];
        auto *car = list.ais[destination].get_volatile_ptr() != nullptr
                        ? traffic::get_traffic_from_entity(vhandle_type<entity>{list.ais[destination].field_0})
                        : nullptr;
        if (car != nullptr)
            car->set_lane_position_index(destination, static_cast<traffic_path_lane *>(list.owner));
        else
            list.ais[destination] = vhandle_type<actor>{};
    }
    list.ais[index] = actor_handle;
    ++list.num_ais;
    return index;
}

traffic_path_road *traffic_path_lane::get_my_road(traffic_path_intersection *intersection) const
{
    for (auto *road : intersection->roads) {
        if (road != nullptr) {
            for (unsigned index = 0; index < road->total_in_lanes + road->total_out_lanes; ++index)
                if (road->all_lanes[index] == this)
                    return road;
        }
    }
    return my_road;
}

traffic_path_lane *traffic_path_lane::get_other_lane()
{
    bool present = false;
    for (unsigned index = 0; index < my_road->total_in_lanes + my_road->total_out_lanes; ++index)
        if (my_road->all_lanes[index] == this) {
            present = true;
            break;
        }
    if (!present)
        return nullptr;
    const bool incoming = my_road->is_an_in_lane(this);
    auto **lanes = incoming ? my_road->in_lanes : my_road->out_lanes;
    const unsigned count = incoming ? my_road->total_in_lanes : my_road->total_out_lanes;
    for (unsigned index = 0; index < count; ++index)
        if (lanes[index] != this && lanes[index]->get_type() == get_type())
            return lanes[index];
    return nullptr;
}

vector3d traffic_path_lane::car_pos(int index) const
{
    auto *car = get_ai_by_index(index).get_volatile_ptr();
    if (car != nullptr)
        return car->get_abs_position();

    return ZEROVEC;
}

entity_base_vhandle traffic_path_lane::get_car_before_point(const vector3d &position, int *index)
{
    const auto start = get_node(0);
    const float distance = (position - start).length2();
    int before = -1;
    for (int car = get_num_ais() - 1; car >= 0; --car) {
        if (!((car_pos(car) - start).length2() < distance))
            break;
        before = car;
    }
    *index = before;
    return before > -1 ? get_ai_by_index(before) : INVALID_HANDLE;
}

float traffic_path_lane::get_free_space_sq()
{
    const auto start = get_node(0);
    auto end = get_node(1);
    if (get_num_ais() != 0) {
        auto *last = get_ai_by_index(get_num_ais() - 1).get_volatile_ptr();
        if (last != nullptr)
            end = last->get_abs_position();
    }
    return (start - end).length2();
}

bool traffic_path_lane::has_room_for_me(float spacing, bool check_intersection, bool ignore_last_node)
{
    if (get_num_ais() >= 19)
        return false;
    const auto start = get_node(0);
    const float spacing_squared = spacing * spacing;
    if ((flags & 2) != 0) {
        const auto next = get_node(1);
        const auto handle = get_num_ais() != 0 ? get_ai_by_index(get_num_ais() - 1) : INVALID_HANDLE;
        if (auto *last = handle.get_volatile_ptr()) {
            const auto position = last->get_abs_position();
            const auto from_start = position - start;
            const auto from_next = position - next;
            const auto segment = start - next;
            const float segment_squared = segment.x * segment.x + segment.z * segment.z;
            const float distance_squared = from_start.x * from_start.x + from_start.z * from_start.z;
            if (from_next.x * from_next.x + from_next.z * from_next.z >= segment_squared &&
                distance_squared < segment_squared)
                return false;
            if (distance_squared <= spacing_squared)
                return false;
            if (!ignore_last_node &&
                traffic::get_traffic_from_entity(vhandle_type<entity>{handle})->field_168 < 1)
                return false;
        }
        if (check_intersection && my_road != nullptr && my_road->field_18 != nullptr) {
            auto *list = my_road->field_18->get_ai_list();
            if (list != nullptr && list->num_ais != 0)
                return false;
        }
    } else {
        const auto end = get_node(total_nodes - 1);
        for (int index = 0; index < get_num_ais(); ++index) {
            if (auto *car = get_ai_by_index(index).get_volatile_ptr()) {
                const auto position = car->get_abs_position();
                if ((position - start).length2() <= spacing_squared ||
                    (position - end).length2() <= spacing_squared)
                    return false;
            }
        }
    }
    return true;
}

bool traffic_path_lane::is_clogged(bool double_spacing, bool ignore_last_node)
{
    return get_num_ais() >= 17 ||
           !has_room_for_me(double_spacing ? 20.0f : 10.0f, false, ignore_last_node);
}
int traffic_path_lane::get_lane_position() const
{
    const int type = get_type();
    return my_road->count_lanes(type, true) <= 1 ? 4 : my_road->get_lane_position(this);
}

entity_base_vhandle traffic_path_lane::get_ai_for_actor()
{
    const int count = get_num_ais();
    return count != 0 ? get_ai_by_index(count - 1) : entity_base_vhandle{INVALID_HANDLE};
}

entity_base_vhandle traffic_path_lane::last_ai()
{
    return get_num_ais() > 0 ? get_ai_by_index(0) : entity_base_vhandle{INVALID_HANDLE};
}

bool traffic_path_lane::is_point_between_nodes(const vector3d &position, int first, int second) const
{
    const auto start = get_node(first);
    const auto end = get_node(second);
    const float segment_length_squared = (end - start).length2();
    return (start - position).length2() < segment_length_squared &&
           (end - position).length2() < segment_length_squared;
}

int traffic_path_lane::get_nearest_node_xz(const vector3d &position) const
{
    int closest = 0;
    float distance = 3.402823466e38f;
    for (int index = 0; index < total_nodes; ++index) {
        const float candidate = (position - get_node(index)).xz_length2();
        if (candidate < distance) {
            closest = index;
            distance = candidate;
        }
    }
    return closest;
}


void traffic_path_lane_patch()
{
    {
        FUNC_ADDRESS(address, &traffic_path_lane::is_valid);
        //SET_JUMP(0x005C8380, address);
    }

    {
        FUNC_ADDRESS(address, &traffic_path_lane::seed_with_pedestrians);
        REDIRECT(0x006D0736, address);
    }
}
