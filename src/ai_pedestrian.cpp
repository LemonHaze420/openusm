#include "ai_pedestrian.h"

#include "ai_std_combat_target.h"
#include "ai_std_avoidance.h"
#include "ai_team.h"
#include "als_animation_logic_system.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "common.h"
#include "game_clock.h"
#include "ped_spawner.h"
#include "physical_interface.h"
#include "std_default_trans_inode.h"
#include "std_fear_inode.h"
#include "wds.h"
#include "ai_voice_box_inode.h"
#include "ai_std_hero.h"
#include "poi.h"
#include "traffic_path.h"
#include "traffic_path_lane.h"
#include "terrain.h"
#include "color32.h"
#include "controller_inode.h"
#include "core_ai_resource.h"
#include "info_node_desc_list.h"
#include "loco_inode.h"
#include "oldmath_po.h"
#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdlib>

namespace ai {

VALIDATE_SIZE(pedestrian_inode, 0xD8);

VALIDATE_SIZE(pedestrian_idle_state, 0x30);

namespace {
float pedestrian_time()
{
#if STANDALONE_SYSTEM
    const auto ticks = static_cast<uint32_t>(game_clock::ticks - pedestrian_inode::timer);
#else
    const auto ticks = var<uint32_t>(0x00965AC4) - pedestrian_inode::timer;
#endif
    return static_cast<float>(static_cast<double>(ticks) * 0.0001f);
}

void *__fastcall pedestrian_delete(pedestrian_inode *self, void *, unsigned char flags)
{
    self->~pedestrian_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(*self));
    return self;
}
unsigned __fastcall pedestrian_type(pedestrian_inode *, void *)
{
    return 159;
}
bool __fastcall pedestrian_subclass(pedestrian_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall pedestrian_needs_advance(pedestrian_inode *self, void *)
{
    return self->does_need_advance();
}
void __fastcall pedestrian_advance(pedestrian_inode *self, void *, Float elapsed)
{
    self->frame_advance(elapsed);
}
void __fastcall pedestrian_activate(pedestrian_inode *self, void *, ai_core *core)
{
    self->activate(core);
}
void __fastcall pedestrian_reset(pedestrian_inode *self, void *)
{
    self->reset();
}
int __fastcall pedestrian_size(pedestrian_inode *, void *)
{
    return sizeof(pedestrian_inode);
}
void __fastcall pedestrian_initialize(pedestrian_inode *self, void *, mash::allocation_scope scope)
{
    if (scope == mash::FROM_MASH) {
        self->m_ped_spawner = ped_spawner::next_ped_spawner;
        self->m_ped_spawner_cleared = false;
    }
}

float planar_distance_squared(const vector3d &a, const vector3d &b)
{
    const float x = a.x - b.x;
    const float z = a.z - b.z;
    return x * x + z * z;
}
}  // namespace

void *pedestrian_inode::native_vtable()
{
    static const std::array<void *, 13> table = [] {
        std::array<void *, 13> result{};
        std::copy_n(static_cast<void **>(info_node::native_vtable()), 12, result.begin());
        result[2] = reinterpret_cast<void *>(&pedestrian_delete);
        result[3] = reinterpret_cast<void *>(&pedestrian_type);
        result[4] = reinterpret_cast<void *>(&pedestrian_subclass);
        result[6] = reinterpret_cast<void *>(&pedestrian_needs_advance);
        result[7] = reinterpret_cast<void *>(&pedestrian_advance);
        result[8] = reinterpret_cast<void *>(&pedestrian_activate);
        result[10] = reinterpret_cast<void *>(&pedestrian_reset);
        result[11] = reinterpret_cast<void *>(&pedestrian_size);
        result[12] = reinterpret_cast<void *>(&pedestrian_initialize);
        return result;
    }();
    return const_cast<void **>(table.data());
}

pedestrian_inode::pedestrian_inode()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    this->initialize(mash::ALLOCATED);
}

pedestrian_inode::pedestrian_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), field_30(constructor), field_3C(constructor), field_4C(constructor),
      field_58(constructor), dodge_position(constructor), dodge_velocity_direction(constructor),
      dodge_direction(constructor), field_9C(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    initialize(mash::FROM_MASH);
}

void pedestrian_inode::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::FROM_MASH) {
        this->m_ped_spawner = ped_spawner::next_ped_spawner;
    }

    this->m_ped_spawner_cleared = false;
    if (a2 == mash::ALLOCATED)
        this->field_D1 = false;
}

void pedestrian_inode::set_ped_spawner(ped_spawner *a2)
{
    if (this->m_ped_spawner != nullptr) {
        if (a2 == nullptr) {
            this->m_ped_spawner_cleared = true;
            this->m_ped_spawner = nullptr;
            return;
        }
    } else if (a2 != nullptr) {
        this->m_ped_spawner_cleared = false;
    }

    this->m_ped_spawner = a2;
}

traffic_path_lane *pedestrian_inode::get_cur_lane() const
{
    auto *v1 = this->m_ped_spawner;
    if (v1 != nullptr) {
        return v1->field_10;
    } else {
        return nullptr;
    }
}

void pedestrian_inode::set_flag(uint32_t a3, bool a4)
{
    if (a4) {
        this->field_1C |= a3;
    } else {
        this->field_1C &= ~a3;
    }
}

void pedestrian_inode::set_can_dodge(int a1)
{
    static const string_hash can_dodge_hash{int(to_hash("can_dodge"))};
    this->my_param_block.set_pb_int(can_dodge_hash, a1, true);
}

void pedestrian_inode::restore_hit_pts()
{
    float a3 = 10.0;
    this->m_hit_points =
        this->field_8->get_param_block()->get_optional_pb_float(pedestrian_inode::hit_points_hash, a3, nullptr);

    this->field_D1 = false;
}

void pedestrian_inode::sub_696AF0(Float a2)
{
    this->field_C4 = a2;
    this->field_C8 = 0;
    this->field_CC = g_world_ptr->time_manager.field_C;
    auto *v2 = this->field_20;

    auto *v3 = v2->field_1C->get_als_layer(static_cast<als::layer_types>(0));

    als::param v4{74, this->field_C4};
    v3->set_desired_param(v4);
}

void pedestrian_inode::activate(ai::ai_core *a2)
{
    info_node::_activate(a2);
    if (is_a_pedestrian(this->field_8)) {
        auto *trans_inode = (std_default_trans_inode *)a2->get_info_node(std_default_trans_inode::default_id, false);
        if (trans_inode != nullptr) {
            trans_inode->set_enabled(false);
        }
    }

    auto *v5 = this->field_8;
    this->field_20 = (ai::als_inode *)v5->get_info_node(als_inode::default_id, true);

    this->field_48 = nullptr;
    this->restore_hit_pts();

    float v2 = 0;
    auto *v7 = this->field_8->get_param_block();
    this->m_elevation_adj = v7->get_optional_pb_float(pedestrian_inode::elevation_adj_hash, v2, nullptr);

    this->reset();
#if STANDALONE_SYSTEM
    timer = static_cast<uint32_t>(game_clock::ticks);
#else
    timer = var<uint32_t>(0x00965AC4);
#endif
    auto *v8 = this->get_actor();
    this->sub_696AF0(v8->get_abs_position().y);
    this->set_can_dodge(this->is_flagged(1u));
}

bool pedestrian_inode::does_need_advance() const
{
    return m_ped_spawner != nullptr && (!m_ped_spawner->field_5 || field_AC != nullptr);
}

void pedestrian_inode::reset()
{
    field_1C = is_a_pedestrian(field_8) ? 1 : 0;
    field_30 = ZEROVEC;
    field_3C = ZEROVEC;
    field_94 = 0.0f;
    field_98 = -1.0f;
    field_9C = ZEROVEC;
    field_A8 = 0;
    field_B0 = 0.0f;
    field_2C = 0;
    field_C0 = 0.0f;
    static const string_hash cower_always{int(to_hash("cower_always"))};
    my_param_block.set_pb_int(cower_always, 0, true);
    if (!field_D1)
        restore_hit_pts();
    if (m_ped_spawner != nullptr) {
        m_ped_spawner->exit_intersection();
        field_90 = m_ped_spawner->field_48;
    }
    update_lane_confinement_als_params();
    if (m_ped_spawner == nullptr || m_ped_spawner->field_5)
        field_AC = nullptr;
    auto *avoidance = static_cast<ped_avoidance_inode *>(field_8->get_info_node(ped_avoidance_inode::default_id, true));
    avoidance->set_respect_obbs(false);
    if (is_flagged(1)) {
        avoidance->field_48 = false;
        avoidance->field_50 = FLT_MAX;
    }
    update_lane_endpoints(get_cur_lane());
}

vector3d pedestrian_inode::get_desired_dir() const
{
    vector3d direction = field_C->get_abs_po().get_z_facing();
    if (is_flagged(2) && is_flagged(0x80)) {
        direction = {field_30.x - field_3C.x, 0.0f, field_30.z - field_3C.z};
        direction.normalize();
    }
    return direction;
}

float pedestrian_inode::get_lane_fraction(const vector3d &direction) const
{
    if (get_cur_lane() == nullptr || !is_flagged(2) || !is_flagged(0x80))
        return 1.0f;
    const auto &position = field_C->get_abs_position();
    const float side_x = -direction.z;
    const float side_z = direction.x;
    return -side_z * (position.z - (side_z + field_30.z)) - side_x * (position.x - (side_x + field_30.x));
}

void pedestrian_inode::update_lane_endpoints(traffic_path_lane *lane)
{
    if (field_48 == lane)
        return;
    if (lane == nullptr) {
        field_4C = ZEROVEC;
        field_58 = ZEROVEC;
    } else {
        field_4C = lane->get_node(0);
        vector3d extension = field_4C - lane->get_node(1);
        extension.normalize();
        field_4C += extension * 1.5f;
        field_58 = lane->get_node(lane->total_nodes - 1);
        extension = field_58 - lane->get_node(lane->total_nodes - 2);
        extension.normalize();
        field_58 += extension * 1.5f;
    }
    field_48 = lane;
}

vector3d pedestrian_inode::get_adjusted_lane_node(traffic_path_lane *lane, int index)
{
    if (lane == nullptr)
        return ZEROVEC;
    update_lane_endpoints(lane);
    if (index == 0)
        return field_4C;
    if (index == lane->total_nodes - 1)
        return field_58;
    return lane->get_node(index);
}

int pedestrian_inode::get_nearest_adjusted_node_index(traffic_path_lane *lane, const vector3d &position)
{
    int nearest = 0;
    float best = 999999.0f;
    if (lane != nullptr) {
        for (int index = 0; index < lane->total_nodes; ++index) {
            const float distance = planar_distance_squared(position, get_adjusted_lane_node(lane, index));
            if (distance < best) {
                best = distance;
                nearest = index;
            }
        }
    }
    return nearest;
}

bool pedestrian_inode::get_next_and_prev_lane_node(vector3d &next, vector3d &previous)
{
    auto *lane = get_cur_lane();
    if (lane == nullptr)
        return false;
    const auto position = field_C->get_abs_position();
    const auto facing = field_C->get_abs_po().get_z_facing();
    float best = FLT_MAX;
    float peripheral_best = FLT_MAX;
    int nearest = -1;
    int peripheral = -1;
    vector3d peripheral_position;
    for (int index = 0; index < lane->total_nodes; ++index) {
        const auto node = get_adjusted_lane_node(lane, index);
        auto direction = node - position;
        const float distance = direction.length();
        if (distance > 0.0f)
            direction *= 1.0f / distance;
        const float alignment = dot(facing, direction);
        if (alignment > 0.9f) {
            if (distance < best) {
                next = node;
                nearest = index;
                best = distance;
            }
        } else if (alignment > 0.0f && distance < peripheral_best) {
            peripheral_position = node;
            peripheral = index;
            peripheral_best = distance;
        }
    }
    if (nearest == -1) {
        nearest = peripheral;
        if (nearest == -1) {
            const auto reference =
                is_flagged(2) && planar_distance_squared(field_30, position) < 9.0f ? field_30 : position;
            nearest = get_nearest_adjusted_node_index(lane, reference);
            next = get_adjusted_lane_node(lane, nearest);
        } else {
            next = peripheral_position;
        }
    }
    if (nearest == 0)
        previous = get_adjusted_lane_node(lane, 1);
    else if (nearest == lane->total_nodes - 1)
        previous = get_adjusted_lane_node(lane, lane->total_nodes - 2);
    else {
        previous = get_adjusted_lane_node(lane, nearest - 1);
        if (dot(next - position, previous - position) >= 0.0f)
            previous = get_adjusted_lane_node(lane, nearest + 1);
    }
    return true;
}

bool pedestrian_inode::is_on_right_side_of_road()
{
    auto *lane = get_cur_lane();
    if (lane == nullptr)
        return false;
    bool approaching_end = false;
    if (is_flagged(2) &&
        planar_distance_squared(get_adjusted_lane_node(lane, lane->total_nodes - 1), field_30) < 1.0f) {
        approaching_end = true;
    } else {
        update_lane_endpoints(lane);
        if (!is_flagged(2) || planar_distance_squared(field_4C, field_30) >= 1.0f) {
            field_C->get_abs_po();
            if (is_flagged(2) && !is_flagged(0x80)) {
                const int nearest = get_nearest_adjusted_node_index(lane, field_30);
                get_adjusted_lane_node(lane, nearest == lane->total_nodes - 1 ? nearest - 1 : nearest + 1);
            }
            get_adjusted_lane_node(lane, lane->total_nodes - 1);
        }
    }
    return (lane->my_road->get_lane(0, true, true) == lane) == approaching_end;
}

void pedestrian_inode::update_lane_confinement_als_params()
{
    vector3d point = ZEROVEC;
    vector3d direction = ZEROVEC;
    float right_side = 0.0f;
    if (!is_flagged(0x10) && get_cur_lane() != nullptr) {
        if (is_flagged(2) && is_flagged(0x80)) {
            direction = get_desired_dir();
            point = field_30;
        } else {
            vector3d previous = ZEROVEC;
            if (get_next_and_prev_lane_node(point, previous)) {
                direction = point - previous;
                direction.normalize();
            }
        }
        right_side = is_on_right_side_of_road() ? 1.0f : 0.0f;
    }
    const float values[] = {point.x, point.y, point.z, -direction.z, 0.0f, direction.x, right_side};
    for (int index = 0; index < 7; ++index) {
        als::param value{76 + index, values[index]};
        field_20->field_1C->get_als_layer(static_cast<als::layer_types>(0))->set_desired_param(value);
    }
}

void pedestrian_inode::update_cur_poi()
{
    auto *previous = field_AC;
    field_AC = nullptr;
    poi_manager::check_init();
    const bool rescan = g_world_ptr->time_manager.field_C % 3 == 0;
    point_of_interest *best = nullptr;
    float total_weight = 0.0f;
    float best_weight = 0.0f;
    for (int index = 0; index <= dword_938004; ++index) {
        auto *point = poi_manager::poi_list[index];
        if (point == nullptr)
            continue;
        if (!rescan) {
            if (point == previous) {
                field_AC = previous;
                break;
            }
        } else if ((point->get_location() - field_C->get_abs_position()).length2() <
                   point->field_18 * point->field_18) {
            total_weight += point->field_10;
            if (best_weight < point->field_10) {
                best_weight = point->field_10;
                best = point;
            }
        }
    }
    if (field_AC == nullptr && total_weight > 0.0f) {
        if (previous != nullptr && best_weight <= previous->field_10)
            field_AC = previous;
        else {
            field_AC = best;
            field_1C &= ~0x1000u;
        }
    }
}

void pedestrian_inode::update_est_speed()
{
    const float now = pedestrian_time();
    const auto position = field_C->get_abs_position();
    if (!(field_98 <= -1.0f && field_98 >= -1.0f)) {
        if (is_flagged(0x100))
            field_94 = 0.0f;
        else if (now - field_98 > 0.0f)
            field_94 = std::min(4.0f, (position - field_9C).length() / (now - field_98));
    }
    field_98 = now;
    field_9C = position;
}

void pedestrian_inode::calc_elevation(float &elevation, float &ground, const vector3d &position, bool limit_rise)
{
    if (field_C->has_physical_ifc())
        return;
    const float old_y = field_C->get_abs_position().y;
    const float floor_offset = std::max(0.2f, field_C->get_floor_offset());
    auto sample = position + YVEC;
    vector3d normal = YVEC;
    entity *hit = nullptr;
    subdivision_node_obb_base *obb = nullptr;
    float terrain_y = g_world_ptr->the_terrain->get_elevation(sample, normal, field_C, &hit, &obb, 3.0f);
    if (terrain_y <= -10000.0f && terrain_y >= -10000.0f) {
        sample = position + YVEC * 3.0f;
        normal = YVEC;
        hit = nullptr;
        obb = nullptr;
        terrain_y = g_world_ptr->the_terrain->get_elevation(sample, normal, field_C, &hit, &obb, 10.0f);
        if (terrain_y <= -10000.0f && terrain_y >= -10000.0f)
            terrain_y = position.y - floor_offset - m_elevation_adj;
    }
    elevation = std::max(position.y - 10.0f, std::min(position.y + 10.0f, terrain_y + m_elevation_adj + floor_offset));
    if (limit_rise && elevation - old_y > 2.0f)
        elevation = old_y;
    ground = elevation - floor_offset - m_elevation_adj;
}

void pedestrian_inode::update_elevation()
{
    if (m_ped_spawner == nullptr || field_C->has_physical_ifc())
        return;
    const int frame = g_world_ptr->time_manager.field_C;
    if (frame == general_last_elev_update_frame || (field_CC <= frame && frame <= field_CC + 10))
        return;
    const auto position = field_C->get_abs_position();
    vector3d direction = field_C->get_velocity();
    direction.y = 0.0f;
    const float speed = direction.length();
    if (speed > 0.0001f)
        direction *= 1.0f / speed;
    else
        direction = ZEROVEC;
    float ground;
    if (get_cur_lane() != nullptr) {
        calc_elevation(field_C4, ground, position, true);
    } else {
        const float distance = std::min(4.0f, speed * 0.67f);
        calc_elevation(field_C4, ground, position + direction * distance, true);
        if (speed > 0.0001f)
            field_C8 = (field_C4 - position.y) / distance;
        else {
            auto adjusted = position;
            adjusted.y = field_C4;
            entity_set_abs_position(field_C, adjusted);
            field_C8 = 0.0f;
        }
    }
    field_CC = frame;
    general_last_elev_update_frame = frame;
    als::param elevation{74, field_C4};
    field_20->field_1C->get_als_layer(static_cast<als::layer_types>(0))->set_desired_param(elevation);
    als::param slope{75, get_cur_lane() != nullptr ? 0.0f : field_C8};
    field_20->field_1C->get_als_layer(static_cast<als::layer_types>(0))->set_desired_param(slope);
}

void pedestrian_inode::update_dodge_direction()
{
    dodge_direction = {1.0f, 0.0f, 0.0f};
    auto away = field_C->get_abs_position() - dodge_position;
    const float distance = away.length2();
    if (distance > 1.0e-10f)
        away *= 1.0f / std::sqrt(distance);
    const auto facing = field_C->get_abs_po().get_z_facing();
    const float alignment = dot(facing, dodge_velocity_direction);
    if (alignment < -0.5f || alignment > 0.5f) {
        float left_limit = 1.0f;
        float right_limit = 1.0f;
        if (!is_flagged(0x10)) {
            if (is_on_right_side_of_road())
                left_limit = 0.9f;
            else
                right_limit = 0.9f;
        }
        const float fraction = get_lane_fraction(get_desired_dir());
        if (dot(away, vector3d{-facing.z, facing.y, facing.x}) > 0.0f) {
            if (is_flagged(0x10) || fraction > left_limit) {
                dodge_direction = {0.0f, 0.0f, 1.0f};
                return;
            }
        } else if (is_flagged(0x10) || fraction < 2.0f - right_limit) {
            dodge_direction = {0.0f, 0.0f, -1.0f};
            return;
        }
    }
    if (dot(away, facing) <= 0.0f)
        dodge_direction = {-1.0f, 0.0f, 0.0f};
}

void pedestrian_inode::check_dodge(entity_base *other, const vector3d &direction, float distance_squared, float speed)
{
    if (distance_squared >= 25.0f)
        return;
    if (distance_squared < 0.6400000453f && speed < 2.0f) {
        set_flag(0x200, true);
        dodge_distance_squared = distance_squared;
        dodge_position = other->get_abs_position();
        dodge_velocity_direction = direction;
        update_dodge_direction();
        return;
    }
    auto facing = field_C->get_abs_po().get_z_facing();
    if (is_flagged(2) && is_flagged(0x80))
        facing = field_30 - field_3C;
    facing.normalize();
    const int iterations = is_flagged(0x100) ? 1 : 3;
    float time = 0.0f;
    for (int index = 0; index < iterations; ++index) {
        const auto predicted = field_C->get_abs_position() + facing * (time * field_94);
        const auto other_position = other->get_abs_position();
        const float projected_distance = dot(predicted - other_position, direction);
        if (index == iterations - 1 && projected_distance < 0.0f)
            return;
        const auto closest = other_position + direction * projected_distance;
        const float separation = (closest - predicted).length2();
        if (iterations > 1)
            time = (closest - other_position).length() / speed;
        if (index == iterations - 1 && separation < 1.0f) {
            set_flag(0x200, true);
            dodge_distance_squared = separation;
            dodge_position = closest;
            dodge_velocity_direction = direction;
            update_dodge_direction();
        }
    }
}

void pedestrian_inode::update_dodging()
{
    set_flag(0x200, false);
    if (non_ped_list == nullptr)
        return;
    set_flag(0x800, false);
    for (auto &handle : *non_ped_list) {
        auto *other = handle.get_volatile_ptr();
        if (other == nullptr)
            continue;
        const float distance_squared = (field_C->get_abs_position() - other->get_abs_position()).length2();
        if (distance_squared < 49.0f) {
            set_flag(0x800, true);
            auto velocity = other->get_velocity();
            const float speed = velocity.length();
            if (speed > 0.5f) {
                velocity *= 1.0f / speed;
                check_dodge(other, velocity, distance_squared, speed);
            }
        }
    }
}

void pedestrian_inode::update_screaming()
{
    if (!is_flagged(1))
        return;
    const float now = pedestrian_time();
    if (now > field_B8) {
        auto *voice = static_cast<voice_box_inode *>(field_8->get_info_node(voice_box_inode::default_id, false));
        if (voice != nullptr) {
            static const string_hash fear{int(to_hash("Fear"))};
            static const string_hash terror{int(to_hash("Terror"))};
            voice->say_gab(is_flagged(0x2000) ? fear : terror, 0, 0, nullptr);
        }
        field_B8 = now + field_BC;
    }
}

void pedestrian_inode::update_hidden_time()
{
    if (field_C->get_render_color().get_alpha() == 0 && field_B0 <= 0.0f && field_B0 >= 0.0f)
        field_B0 = pedestrian_time();
}

bool pedestrian_inode::should_show() const
{
    if (field_C->get_render_color().get_alpha() != 0)
        return false;
    if (m_ped_spawner == nullptr)
        return true;
    return !m_ped_spawner->field_5 && field_B0 + 1.0f < pedestrian_time();
}

void pedestrian_inode::update_greeting()
{
    const float now = g_world_ptr->time_manager.field_8;
    if (!(now > next_greeting_time || next_greeting_time - now > 999.0f))
        return;
    if (static_cast<unsigned>(std::rand() * 0.000091552734375) != 0 || field_D1 || is_flagged(0x200) ||
        is_flagged(0x400))
        return;
    static const string_hash ped_combat{int(to_hash("combat_inode"))};
    auto *combat = field_8->get_info_node(ped_combat, true);
    if (reinterpret_cast<const unsigned char *>(combat)[0x82] != 0)
        return;
    auto *voice = static_cast<voice_box_inode *>(field_8->get_info_node(voice_box_inode::default_id, false));
    auto *hero = g_world_ptr->get_hero_ptr(0);
    if (voice == nullptr || hero == nullptr)
        return;
    hero->get_ai_core()->get_info_node(hero_inode::default_id, true);
    if (get_hero_type_helper() != 1)
        return;
    const auto offset = hero->get_abs_position() - field_C->get_abs_position();
    const float distance_squared = offset.length2();
    if (distance_squared < 144.0f) {
        static const string_hash idle{int(to_hash("thug_idle"))};
        static const string_hash greet{int(to_hash("greet"))};
        voice->say_gab(distance_squared < 25.0f && dot(offset, field_C->get_abs_po().get_z_facing()) > 0.0f ? greet
                                                                                                            : idle,
                       0,
                       0,
                       nullptr);
        next_greeting_time = now + 3.0f + static_cast<float>(std::rand()) * (2.0f / 32768.0f);
    }
}

void pedestrian_inode::frame_advance(Float elapsed)
{
    if (m_ped_spawner == nullptr || m_ped_spawner->field_5) {
        field_AC = nullptr;
        return;
    }
    update_elevation();
    update_lane_confinement_als_params();
    update_cur_poi();
    update_est_speed();
    static const string_hash can_dodge{int(to_hash("can_dodge"))};
    int default_dodge = 1;
    if (!is_flagged(0x400) && my_param_block.get_optional_pb_int(can_dodge, default_dodge, nullptr) && !field_D1)
        update_dodging();
    if (is_flagged(1)) {
        if (is_flagged(0x6000))
            update_screaming();
        else if (field_AC == nullptr || field_AC->field_C != 1)
            update_greeting();
    }
    update_hidden_time();
    should_show();
    if (!is_flagged(1) && field_C->has_physical_ifc() && (field_C->physical_ifc()->field_C & 0x20)) {
        field_C0 += elapsed;
        if (field_C0 >= 1.0f) {
            if (m_ped_spawner != nullptr)
                m_ped_spawner->sub_6BBD30(nullptr);
            field_1C &= 0xFFFFFF31u;
            if (auto *hero = g_world_ptr->get_hero_ptr(0))
                field_8->set_facing_point(hero->get_abs_position());
            field_8->stop_movement();
        }
    } else if (field_C0 < 1.0f) {
        field_C0 = 0.0f;
    }
}

bool pedestrian_inode::is_a_pedestrian(ai::ai_core *a1)
{
    if (a1 != nullptr) {
        auto *p_pb = a1->get_param_block();
        if (p_pb->does_parameter_exist(combat_target_inode::team_hash())) {
            auto pb_hash = p_pb->get_pb_hash(combat_target_inode::team_hash());
            if (team::manager::get_team_enum_by_hash(pb_hash) == 14) {
                return true;
            }
        }
    }

    return false;
}

void pedestrian_inode::register_non_ped(vhandle_type<actor> actor_handle)
{
    TRACE("pedestrian_inode::register_non_ped");

    if (non_ped_list == nullptr) {
        non_ped_list = new _std::list<vhandle_type<actor>>{};
    }
    non_ped_list->push_back(actor_handle);
}

void pedestrian_inode::unregister_non_ped(vhandle_type<actor> actor_handle)
{
    if (non_ped_list == nullptr) {
        return;
    }
    for (auto it = non_ped_list->begin(); it != non_ped_list->end(); ++it) {
        if (it->field_0 == actor_handle.field_0) {
            non_ped_list->erase(it);
            break;
        }
    }
    if (non_ped_list->empty()) {
        delete non_ped_list;
        non_ped_list = nullptr;
    }
}

namespace {
unsigned __fastcall idle_state_type(pedestrian_idle_state *, void *)
{
    return 177;
}
bool __fastcall idle_state_subclass(pedestrian_idle_state *, void *, unsigned type)
{
    return type == 535 || type == 567 || type == 573;
}
void __fastcall idle_state_activate(pedestrian_idle_state *self, void *, ai_state_machine *machine,
                                    const mashed_state *state, const mashed_state *previous,
                                    const param_block *parameters, base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, parameters, flags);
}
state_trans_messages __fastcall idle_state_advance(pedestrian_idle_state *self, void *, Float elapsed)
{
    return self->frame_advance(elapsed);
}
void __stdcall idle_state_nodes(info_node_desc_list *list)
{
    list->add_entry({pedestrian_inode::default_id, 159});
    list->add_entry({ped_avoidance_inode::default_id, 157});
    list->add_entry({controller_inode::default_id, 357});
    list->add_entry({string_hash("nonpath_loco_layer"), 156});
}
}  // namespace

pedestrian_idle_state::pedestrian_idle_state() : enhanced_state()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

pedestrian_idle_state::pedestrian_idle_state(from_mash_in_place_constructor *constructor) : enhanced_state(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[177]);
}

void *pedestrian_idle_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> callbacks;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), callbacks.size(), callbacks.begin());
        callbacks[3] = reinterpret_cast<void *>(&idle_state_type);
        callbacks[4] = reinterpret_cast<void *>(&idle_state_subclass);
        callbacks[6] = reinterpret_cast<void *>(&idle_state_activate);
        callbacks[8] = reinterpret_cast<void *>(&idle_state_advance);
        callbacks[9] = reinterpret_cast<void *>(&idle_state_nodes);
        return callbacks;
    }();
    return table.data();
}

void pedestrian_idle_state::activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                     const param_block *parameters, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, parameters, flags);
    auto *core = get_core();
    auto *pedestrian = static_cast<pedestrian_inode *>(core->get_info_node(pedestrian_inode::default_id, true));
    pedestrian->reset();
    core->stop_movement();
    if (!pedestrian->is_flagged(1) && pedestrian->m_ped_spawner == nullptr && !pedestrian->m_ped_spawner_cleared)
        pedize_non_pedestrian();
    core->change_locomotion_machine(string_hash("nonpath_loco_layer"));
    if (core->field_40 != nullptr)
        core->field_40->field_5F = false;
    if (core->field_6C->field_44)
        core->field_4C |= 1;
}

state_trans_messages pedestrian_idle_state::frame_advance(Float elapsed)
{
    const auto result = enhanced_state::frame_advance(elapsed);
    auto *core = get_core();
    auto *pedestrian = static_cast<pedestrian_inode *>(core->get_info_node(pedestrian_inode::default_id, true));
    if (!pedestrian->is_flagged(1) && pedestrian->m_ped_spawner == nullptr && !pedestrian->m_ped_spawner_cleared)
        pedize_non_pedestrian();
    if (pedestrian->m_ped_spawner == nullptr || core->field_64 == nullptr)
        return result;
    if (pedestrian->my_param_block.get_optional_pb_int(string_hash("stay_idle"), 0, nullptr))
        return pedestrian->is_flagged(0x200) ? static_cast<state_trans_messages>(50) : result;
    if (pedestrian->m_ped_spawner->field_5) {
        core->field_64->suspend(true);
        return result;
    }
    pedestrian->sub_696AF0(core->field_64->get_abs_position().y);
    if (pedestrian->is_flagged(1)) {
        auto *voice = static_cast<voice_box_inode *>(core->get_info_node(voice_box_inode::default_id, false));
        static float last_city_gab_time;
        const auto now = g_world_ptr->time_manager.field_8;
        if (voice != nullptr && (now - last_city_gab_time > 0.5f || last_city_gab_time > now)) {
            last_city_gab_time = now;
            voice->say_gab(string_hash("City"), 0, 0, nullptr);
        }
    }
    pedestrian->field_1C &= ~0x1000u;
    if (pedestrian->field_D1)
        return static_cast<state_trans_messages>(65);
    bool blocked = false;
    if (pedestrian->field_AC != nullptr) {
        auto *avoidance =
            static_cast<ped_avoidance_inode *>(core->get_info_node(ped_avoidance_inode::default_id, false));
        blocked = avoidance != nullptr && (avoidance->field_48 || avoidance->field_50 > 2.25f);
    }
    if (blocked) {
        pedestrian->field_1C |= 0x1000u;
        return static_cast<state_trans_messages>(52);
    }
    return static_cast<state_trans_messages>(2 - (pedestrian->get_cur_lane() != nullptr));
}

void pedestrian_idle_state::pedize_non_pedestrian()
{
    assert(!pedestrian_inode::is_a_pedestrian(this->get_core()) && "This is a pedestrian already");

    auto *my_core = this->field_C->my_core;
    auto *v3 = my_core->field_64;
    if (v3 != nullptr) {
        if (v3->has_physical_ifc()) {
            auto *v5 = v3->physical_ifc();
            v5->set_gravity(true);
        }

        auto *v6 = this->field_C->my_core;
        auto *ped_inode = (pedestrian_inode *)v6->get_info_node(pedestrian_inode::default_id, true);
        auto *v8 = ped_spawner::assign_non_ped_actor(vhandle_type<actor>{v3->my_handle.field_0});
        if (v8 != nullptr) {
            ped_inode->set_ped_spawner(v8);
            ped_inode->reset();
            pedestrian_inode::unregister_non_ped(vhandle_type<actor>{v3->my_handle.field_0});
            if (ped_inode->get_cur_lane() != nullptr) {
                ped_inode->set_flag(0x20, true);
                ped_inode->set_flag(0x10, true);

                auto *the_core = this->get_core();
                auto *v11 = (ped_avoidance_inode *)the_core->get_info_node(ped_avoidance_inode::default_id, true);
                v11->set_respect_obbs(true);
            }
        }

        auto *v12 = this->get_core();
        auto *v13 = (std_fear_inode *)v12->get_info_node(std_fear_inode::default_id, false);
        if (v13 != nullptr) {
            v13->set_cowering_enabled(false);
        }
    }
}

}  // namespace ai
