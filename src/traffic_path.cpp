#include "traffic_path.h"

#include "common.h"
#include "traffic.h"
#include "traffic_path_lane.h"
#include "collide.h"
#include "glass_house_manager.h"
#include <algorithm>
#include <cfloat>
#include <cstdlib>

VALIDATE_SIZE(traffic_ai_list, 0x5C);
VALIDATE_SIZE(traffic_path_intersection, 0x18);

#if STANDALONE_SYSTEM
namespace {
traffic_ai_list native_ai_lists[64]{};
}
traffic_ai_list (&traffic_ai_list::ai_lists)[64] = native_ai_lists;
#else
traffic_ai_list (&traffic_ai_list::ai_lists)[64] = var<traffic_ai_list[64]>(0x00968690);
#endif

bool traffic_path_intersection::has_stopsign(bool)
{

    return false;
}

bool traffic_path_road::is_an_in_lane(const traffic_path_lane *a2) const
{
    for (auto i = 0u; i < this->total_in_lanes; ++i) {
        if (this->in_lanes[i] == a2) {
            return true;
        }
    }

    return false;
}

bool traffic_path_road::road_is_valid(const traffic_path_road *a1)
{
    return a1 != nullptr && a1->total_in_lanes <= 7u && a1->total_out_lanes <= 7u;
}

int traffic_ai_list::get_ai_index(vhandle_type<actor> me)
{
    assert(me.field_0 != INVALID_HANDLE);

    for (int i = 0; i < this->num_ais; ++i) {
        if (this->ais[i].field_0 == me.field_0) {
            assert(this->ais[i].get_volatile_ptr()->is_an_actor());
            return i;
        }
    }

    return -1;
}

int traffic_ai_list::allocate(uint16_t list_type, void *list_owner)
{
    for (int index = 0; index < 64; ++index) {
        auto &list = ai_lists[index];
        if (list.type == 0) {
            list.type = list_type;
            list.num_ais = 0;
            list.owner = list_owner;
            return index;
        }
    }
    return -1;
}

void traffic_ai_list::release()
{
    num_ais = 0;
    type = 0;
    owner = nullptr;
}

void traffic_ai_list::remove_ai(vhandle_type<actor> actor_handle)
{
    for (int index = 0; index < num_ais; ++index) {
        if (ais[index].field_0 == actor_handle.field_0 || ais[index].get_volatile_ptr() == nullptr) {
            ais[index] = vhandle_type<actor>{};
            for (int next = index; next < num_ais - 1; ++next) {
                ais[next] = ais[next + 1];
                if (ais[next].get_volatile_ptr() != nullptr) {
                    auto *car = traffic::get_traffic_from_entity(vhandle_type<entity>{ais[next].field_0});
                    if (car != nullptr && type == 1)
                        car->set_lane_position_index(next, static_cast<traffic_path_lane *>(owner));
                }
            }
            --num_ais;
        }
    }
}

traffic_ai_list *traffic_path_intersection::get_ai_list()
{
    return field_16 < 0 ? nullptr : &traffic_ai_list::ai_lists[field_16];
}

bool traffic_path_intersection::remove_ai_from_intersection(vhandle_type<actor> actor_handle, int)
{
    auto *list = get_ai_list();
    if (list == nullptr)
        return false;
    list->remove_ai(actor_handle);
    if (list->num_ais == 0) {
        list->release();
        field_16 = -1;
    }
    return true;
}

void traffic_path_intersection::release_semaphore(bool)
{

}

traffic_path_lane *traffic_path_road::get_lane(int index, int type, bool incoming) const
{
    auto **lanes = type == 1 ? all_lanes : incoming ? in_lanes : out_lanes;
    const unsigned count = type == 1 ? total_in_lanes + total_out_lanes
                                    : incoming ? total_in_lanes : total_out_lanes;
    int match = 0;
    for (unsigned lane = 0; lane < count; ++lane) {
        if (lanes[lane]->get_type() == type) {
            if (match == index)
                return lanes[lane];
            ++match;
        }
    }
    return nullptr;
}

int traffic_path_road::count_lanes(int type, bool incoming) const
{
    auto **lanes = incoming ? in_lanes : out_lanes;
    const unsigned count = incoming ? total_in_lanes : total_out_lanes;
    int result = 0;
    for (unsigned index = 0; index < count; ++index)
        if (lanes[index]->get_type() == type)
            ++result;
    return result;
}

int traffic_path_road::get_lane_index(const traffic_path_lane *lane) const
{
    for (unsigned index = 0; index < total_in_lanes + total_out_lanes; ++index)
        if (all_lanes[index] == lane)
            return static_cast<int>(index);
    return -1;
}

int traffic_path_road::get_lane_position(const traffic_path_lane *lane) const
{
    const int type = lane->get_type();
    if (get_lane(0, type, true) == lane)
        return 2;
    if (get_lane(count_lanes(type, true) - 1, type, true) == lane ||
        get_lane(0, type, false) == lane)
        return 1;
    return get_lane(count_lanes(type, false) - 1, type, false) == lane ? 2 : 4;
}

traffic_path_lane *traffic_path_road::get_indexed_lane(int index, int type) const
{
    traffic_path_lane *last = nullptr;
    for (unsigned current = 0; current < total_in_lanes + total_out_lanes; ++current) {
        auto *lane = all_lanes[current];
        if (type == 3 || lane->get_type() == type) {
            if (static_cast<int>(current) == index)
                return lane;
            last = lane;
        }
    }
    return last;
}

traffic_path_lane *traffic_path_road::get_closest_lane(const vector3d &position, int type) const
{
    float best = FLT_MAX;
    traffic_path_lane *result = nullptr;
    for (unsigned index = 0; index < total_in_lanes + total_out_lanes; ++index) {
        auto *lane = all_lanes[index];
        if (lane->get_type() != type || lane->total_nodes <= 1)
            continue;

        vector3d closest;
        closest_point_segment(position, lane->get_node(0), lane->get_node(1), closest);
        const float distance = (closest - position).length2();
        if (distance < best) {
            best = distance;
            result = lane;
        }
    }
    return result;
}

int traffic_path_road::map_lane_index(traffic_path_lane *lane, const traffic_path_road *next_road,
                                      int index, bool allow_incoming) const
{
    const int type = lane->get_type();
    if (type != 1) {
        if (next_road == nullptr)
            return -1;
        const int count = next_road->total_in_lanes + next_road->total_out_lanes;
        const int mirrored = count - index - 1;
        if (mirrored >= 0) {
            auto *candidate = next_road->all_lanes[mirrored];
            if (candidate != nullptr && candidate->get_type() == type &&
                (allow_incoming || !next_road->is_an_in_lane(candidate)))
                return mirrored;
        }
        int traversed = 0;
        int distance_after_match = 0;
        int fallback = -1;
        bool found = false;
        bool have_fallback = false;
        for (int current = count - 1; current >= 0; --current, ++traversed) {
            if (have_fallback)
                ++fallback;
            if (found) {
                ++distance_after_match;
            } else if (next_road->all_lanes[current]->get_type() == type) {
                if (traversed < index) {
                    if (allow_incoming || !next_road->is_an_in_lane(next_road->all_lanes[current])) {
                        have_fallback = true;
                        fallback = 0;
                    }
                } else {
                    found = true;
                }
            }
        }
        int result = found || !have_fallback ? distance_after_match : fallback;
        if (!allow_incoming && next_road->is_an_in_lane(next_road->all_lanes[result])) {
            result = fallback;
            if (next_road->is_an_in_lane(next_road->all_lanes[result]))
                return -1;
        }
        return result;
    }
    int matched = 0;
    int remaining = 0;
    bool found = false;
    for (int current = next_road->total_in_lanes + next_road->total_out_lanes - 1; current >= 0; --current) {
        if (next_road->all_lanes[current]->get_type() == 1) {
            if (found)
                ++remaining;
            else if (matched == index)
                found = true;
            else
                ++matched;
        }
    }
    return remaining;
}

int traffic_path_intersection::get_ai_index(vhandle_type<actor> actor_handle)
{
    const auto *list = get_ai_list();
    if (list != nullptr)
        for (int index = 0; index < list->num_ais; ++index)
            if (list->ais[index].field_0 == actor_handle.field_0)
                return index;
    return -1;
}

bool traffic_path_intersection::add_ai(vhandle_type<actor> actor_handle)
{
    if (field_16 < 0)
        field_16 = static_cast<int16_t>(traffic_ai_list::allocate(2, this));
    auto *list = get_ai_list();
    if (list == nullptr || list->num_ais >= 19)
        return false;
    auto *actor = actor_handle.get_volatile_ptr();
    if (actor == nullptr || !actor->is_an_actor())
        return false;
    list->ais[list->num_ais++] = actor_handle;
    return true;
}

bool traffic_path_intersection::reserve_stopsign(vhandle_type<actor>, int)
{

    return true;
}

bool traffic_path_intersection::reserve_turn(int direction, bool)
{
    const unsigned state = next_direction;
    if ((state & 2) != 0)
        return false;
    switch (direction) {
    case -1:
        if ((state & 0x1C) == 0x1C)
            return false;
        next_direction = static_cast<uint8_t>((state & ~0x1Cu) | ((state + 4) & 0x1C));
        return true;
    case 1:
        if ((state & 0xFC) != 0)
            return false;
        next_direction |= 2;
        return true;
    case 2:
        if ((state & 0xE0) == 0xE0)
            return false;
        next_direction = static_cast<uint8_t>((state & 0x1F) | ((state + 32) & 0xE0));
        return true;
    default:
        return false;
    }
}

void traffic_path_intersection::release_turn(int direction, bool)
{
    const unsigned state = next_direction;
    switch (direction) {
    case -1:
        if ((state & 0x1C) != 0)
            next_direction = static_cast<uint8_t>((state & ~0x1Cu) | ((state - 4) & 0x1C));
        break;
    case 1:
        next_direction &= ~2u;
        break;
    case 2:
        if ((state & 0xE0) != 0)
            next_direction = static_cast<uint8_t>((state & 0x1F) | ((state - 32) & 0xE0));
        break;
    }
}

int traffic_path_intersection::get_direction_to_lane(traffic_path_lane *lane, traffic_path_lane *next_lane)
{
    auto *source = lane->my_road;
    auto *destination = next_lane->my_road;
    if ((destination != nullptr ? destination->field_18 : nullptr) !=
        (source != nullptr ? source->field_1C : nullptr))
        return 0;
    if ((source->n2 + 1) % 4 == destination->n2_1)
        return -1;
    return (source->n2 + 2) % 4 == destination->n2_1 ? 2 : 1;
}

namespace {
int road_slot(int slot)
{
    return slot >= 4 ? slot % 4 : slot < 0 ? 3 : slot;
}

int lane_turn_position(traffic_path_lane *lane)
{
    auto *road = lane->my_road;
    const int type = lane->get_type();
    if (type != 1)
        return road->count_lanes(type, true) <= 1 ? 4 : road->get_lane_position(lane);
    return road->all_lanes[0]->get_type() != type ? 2 : 1;
}
}

bool traffic_path_intersection::get_allowed_ai_roads(traffic_path_lane *lane, traffic_path_road **out_roads)
{
    out_roads[0] = out_roads[1] = out_roads[2] = nullptr;
    const int position = lane_turn_position(lane);
    const int offsets[] = {-1, 1, 2};
    const unsigned restrictions[] = {0x80, 0x40, 0x100};
    const unsigned road_flag = (lane->flags & 2) != 0 ? 4 : 8;
    for (int index = 0; index < 3; ++index) {
        if ((position == 1 && index == 0) || (position == 2 && index == 1) ||
            (lane->flags & restrictions[index]) != 0)
            continue;
        auto *road = roads[road_slot(lane->my_road->n2 + offsets[index])];
        if (road != nullptr && (road->flags & road_flag) != 0)
            out_roads[index] = road;
    }
    return out_roads[0] != nullptr || out_roads[1] != nullptr || out_roads[2] != nullptr;
}

traffic_path_lane *traffic_path_intersection::get_next_lane(vector3d position, int direction,
                                                            traffic_path_lane *lane,
                                                            traffic_path_graph **graph,
                                                            int orientation, bool)
{
    auto *source = lane->my_road;
    const int slot = road_slot((orientation > 0 ? source->n2 : source->n2_1) + direction);
    auto *destination = roads[slot];
    if (destination == source)
        return nullptr;
    *graph = field_10;
    if (destination == nullptr || destination->total_out_lanes == 0)
        return nullptr;
    traffic_path_lane *result;
    if ((lane->flags & 2) != 0) {
        const int index = source->map_lane_index(lane, destination, source->get_lane_index(lane), false);
        result = destination->get_indexed_lane(index, 0);
    } else if (direction == 1 || direction == -1) {
        result = destination->get_closest_lane(position, 1);
    } else {
        const int index = source->map_lane_index(lane, destination, source->get_lane_index(lane), false);
        result = destination->get_indexed_lane(index, 1);
    }
    return result != nullptr && result->get_type() == lane->get_type() ? result : nullptr;
}

float traffic_path_intersection::evaluate_road_chance(
    traffic_path_lane *lane, traffic_path_lane **next_lane, float bias, int road_index,
    int excluded_direction, int direction, const vector3d &target, bool weigh_flags, bool randomize)
{
    auto *road = roads[road_slot(road_index)];
    if (road == nullptr || (road->flags & ((lane->flags & 2) != 0 ? 4 : 8)) == 0)
        return 0.0f;
    if (excluded_direction == direction)
        return FLT_MAX;
    float chance = 1.0f;
    if (direction != 2) {
        *next_lane = road->get_lane(0, lane->get_type(), false);
        if (*next_lane != nullptr) {
            const auto source_direction = lane->get_directional_node() - lane->get_node(0);
            const auto target_direction = (*next_lane)->get_directional_node() - (*next_lane)->get_node(0);
            if (randomize) {
                if (dot(target_direction, source_direction) < 0.2f &&
                    (lane->get_directional_node() - (*next_lane)->get_node(0)).length2() > 400.0f)
                    chance = 0.1f;
            } else {
                chance = 1.0f - compute_angle_between_vectors(source_direction, target_direction) * 0.3183098733f;
            }
        }
    }
    if (randomize)
        chance *= static_cast<float>(std::rand()) * 0.000030517578125f + bias;
    if (excluded_direction == 4) {
        *next_lane = road->get_lane(0, 0, false);
        if (!randomize && *next_lane == nullptr)
            *next_lane = road->get_lane(0, 0, true);
        const auto start = (*next_lane)->get_node(0);
        if (glass_house_manager::is_point_in_glass_house((*next_lane)->get_directional_node())) {
            auto lane_direction = (*next_lane)->get_node(1) - start;
            lane_direction.normalize();
            auto target_direction = target - start;
            target_direction.normalize();
            chance = (dot(target_direction, lane_direction) + 1.0f) * 0.5f;
            if (road->total_out_lanes < 1)
                chance *= 0.5f;
        } else {
            chance = 0.0f;
        }
        if (randomize) {
            if ((*next_lane)->is_clogged(false, false))
                chance *= 0.01f;
            if ((*next_lane)->my_road->total_out_lanes > 1)
                chance *= 1.5f;
        }
    }
    if (!randomize && weigh_flags) {
        if (((*next_lane)->flags & 0x1000) != 0)
            chance *= 0.001f;
        if (((*next_lane)->flags & 0x800) != 0)
            chance *= 0.01f;
    }
    return chance;
}

int traffic_path_intersection::get_next_direction(
    traffic_path_lane *lane, traffic_path_lane **next_lane, int orientation, int excluded_direction,
    const vector3d &target, bool ignore_restrictions, bool randomize)
{
    const int base = orientation > 0 ? lane->my_road->n2 : lane->my_road->n2_1;
    const int position = lane_turn_position(lane);
    traffic_path_lane *left = nullptr;
    traffic_path_lane *right = nullptr;
    traffic_path_lane *straight = nullptr;
    float left_chance = 0.0f;
    float right_chance = 0.0f;
    float straight_chance = 0.0f;
    auto evaluate_left = [&] {
        left_chance = evaluate_road_chance(lane, &left, 0.1f, base - 1, excluded_direction,
                                           -1, target, false, randomize);
    };
    auto evaluate_right = [&] {
        right_chance = evaluate_road_chance(lane, &right, 0.1f, base + 1, excluded_direction,
                                            1, target, false, randomize);
    };
    auto evaluate_straight = [&] {
        straight_chance = evaluate_road_chance(lane, &straight, 0.2f, base + 2, excluded_direction,
                                               2, target, false, randomize);
    };
    if (randomize) {
        if (position != 1 && ((lane->flags & 0x80) == 0 || ignore_restrictions))
            evaluate_left();
        if (position == 1 && ((lane->flags & 0x40) == 0 || ignore_restrictions))
            evaluate_right();
        if ((lane->flags & 0x100) == 0 || ignore_restrictions)
            evaluate_straight();
        straight_chance *= 2.0f;
        if (position == 1) {
            if (std::max(right_chance, straight_chance) <= 0.0f &&
                ((lane->flags & 0x80) == 0 || ignore_restrictions))
                evaluate_left();
        } else if (position != 2 || std::max(left_chance, straight_chance) <= 0.0f) {
            if ((lane->flags & 0x40) == 0 || ignore_restrictions)
                evaluate_right();
        }
        const float total = straight_chance + right_chance + left_chance;
        if (total <= 0.0f) {
            *next_lane = nullptr;
            return 0;
        }
        const double selected = static_cast<double>(std::rand()) * 0.000030517578125f * total;
        if (selected <= straight_chance) {
            *next_lane = straight;
            return 2;
        }
        if (selected <= straight_chance + left_chance) {
            *next_lane = left;
            return -1;
        }
        *next_lane = right;
        return 1;
    }
    if (position == 1)
        evaluate_right();
    else
        evaluate_left();
    evaluate_straight();
    float best = std::max(position == 1 ? right_chance : left_chance, straight_chance);
    if (position != 1 && position != 2) {
        evaluate_right();
        if (straight_chance > 0.0f && lane->my_road->count_lanes(lane->get_type(), true) > 1)
            best = straight_chance;
        else
            best = std::max(straight_chance, std::max(left_chance, right_chance));
    } else if (ignore_restrictions || best <= 0.0f) {
        if (position == 1)
            evaluate_left();
        else
            evaluate_right();
        best = std::max(best, position == 1 ? left_chance : right_chance);
    }
    if (best <= 0.0f) {
        *next_lane = nullptr;
        return 0;
    }
    if (left_chance <= best && left_chance >= best) {
        *next_lane = left;
        return -1;
    }
    if (right_chance <= best && right_chance >= best) {
        *next_lane = right;
        return 1;
    }
    *next_lane = straight;
    return 2;
}

bool traffic_route::get_next_turn(traffic_path_lane *lane, traffic_path_intersection::eDirection *direction,
                                  traffic_path_lane **next_lane)
{
    while (!lanes.empty() && lanes.back() == lane)
        lanes.pop_back();
    if (lanes.empty())
        return false;
    auto *next = lanes.back();
    *next_lane = next;
    *direction = static_cast<traffic_path_intersection::eDirection>(
        lane != nullptr ? lane->get_next_intersection(1)->get_direction_to_lane(lane, next) : 0);
    lanes.pop_back();
    return true;
}
