#include "ai_std_avoidance.h"

#include "actor.h"
#include "ai_pedestrian.h"
#include "ai_team.h"
#include "base_ai_core.h"
#include "camera.h"
#include "collision_capsule.h"
#include "colmesh.h"
#include "common.h"
#include "core_ai_resource.h"
#include "game.h"
#include "local_collision.h"
#include "native_info_node_table.h"
#include "oldmath_po.h"
#include "ped_spawner.h"
#include "physical_interface.h"
#include "subdivision_obb.h"
#include "wds.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>

namespace ai {

VALIDATE_SIZE(avoidance_obstacle, 0x48);
VALIDATE_SIZE(avoidance_inode, 0x48);
VALIDATE_SIZE(ped_avoidance_inode, 0x54);
static_assert(offsetof(avoidance_obstacle, axes) == 8);
static_assert(offsetof(avoidance_obstacle, center) == 0x38);
static_assert(offsetof(avoidance_obstacle, top) == 0x44);

namespace {
const string_hash respect_obbs_hash{int(to_hash("respect_obbs"))};
const string_hash ignore_teammates_hash{int(to_hash("ignore_teammates"))};
const string_hash team_hash{int(to_hash("team"))};
int next_update_offset = 1;
constexpr int update_interval = 10;

float length_squared(const vector3d &v)
{
    return dot(v, v);
}
vector3d perpendicular(const vector3d &v)
{
    return {v.z, 0.0f, -v.x};
}


bool __fastcall avoidance_entity_filter(const local_collision::entfilter_base *, void *, actor *ent,
                                        dynamic_conglomerate_clone *, const local_collision::query_args_t *args)
{
    if (!ent->has_entity_collision())
        return false;
    if (ent->field_4 & 4) {
        const float radius = ent->get_colgeom_radius() + args->field_28;
        return length_squared(ent->get_colgeom_center() - args->field_10) < radius * radius;
    }
    const vector3d center = ent->get_abs_po().inverse_xform(args->field_10);
    auto *geometry = ent->colgeom;
    if (geometry->get_type() == collision_geometry::MESH) {
        const auto &box = static_cast<cg_mesh *>(geometry)->data->field_10[0];
        const vector3d offset = center - box.field_0;
        const vector3d *axes[] = {&box.field_10, &box.axis_y, &box.axis_z};
        float distance2 = 0.0f;
        for (const auto *axis : axes) {
            const float extent = axis->length();
            const float outside = std::max(0.0f, std::fabs(dot(*axis, offset) / extent) - extent);
            distance2 += outside * outside;
        }
        return distance2 <= args->field_28 * args->field_28;
    }
    const float radius = geometry->get_bounding_sphere_radius() + args->field_28;
    return length_squared(center - geometry->get_local_space_bounding_sphere_center()) < radius * radius;
}

const local_collision::entfilter_base::native_vtable avoidance_filter_table{avoidance_entity_filter};
const local_collision::entfilter_base avoidance_filter{reinterpret_cast<std::intptr_t>(&avoidance_filter_table)};


void horizontal_axes(const vector3d *axes, vector3d &a, vector3d &b)
{
    for (int i = 0; i < 3; ++i) {
        const int j = (i + 1) % 3;
        const int k = (i + 2) % 3;
        if (std::fabs(axes[i].y) > std::fabs(axes[j].y) && std::fabs(axes[i].y) > std::fabs(axes[k].y)) {
            a = axes[j];
            b = axes[k];
            return;
        }
    }
}

void box_obstacle(avoidance_obstacle &out, const subdivision_node_obb_base &box)
{
    out.handle = INVALID_HANDLE;
    out.is_entity = false;
    out.has_box = true;
    out.center = box.center;
    vector3d axes[3];
    box.unpack_axii(axes);
    out.top = box.center.y + std::max(0.0f, std::max(axes[0].y, std::max(axes[1].y, axes[2].y)));
    horizontal_axes(axes, out.axes[0], out.axes[2]);
    out.axes[1] = -out.axes[0];
    out.axes[3] = -out.axes[2];
}


float box_radius(const avoidance_obstacle &obstacle, const vector3d &direction)
{
    if (std::fabs(length_squared(direction) - 1.0f) >= EPSILON)
        return 0.0f;
    float radii[2];
    for (int i = 0; i < 2; ++i) {
        const int axis = i * 2;
        const float projection = dot(direction, obstacle.axes[axis]);
        if (projection <= 0.0f && projection >= 0.0f)
            radii[i] = 999999.0f;
        else {
            const auto &v = obstacle.axes[axis + (projection < 0.0f)];
            radii[i] = length_squared(v) / dot(direction, v);
        }
    }
    return std::min(radii[0], radii[1]);
}

float entity_radius(entity *ent)
{
    if (ent->is_an_actor() && ent->colgeom && ent->colgeom->get_type() == collision_geometry::CAPSULE)
        return static_cast<collision_capsule *>(ent->colgeom)->get_core_radius();
    return 0.5f;
}


float blocking_strength(const avoidance_obstacle &obstacle, const vector3d &position, const vector3d &center,
                        const vector3d &direction, const vector3d &force)
{
    const float magnitude2 = length_squared(force);
    if (magnitude2 <= 0.0f)
        return 0.0f;
    if (!obstacle.has_box) {
        const float magnitude = std::sqrt(magnitude2);
        return dot(force / magnitude, direction) < -0.866f ? magnitude : 0.0f;
    }
    if (dot(direction, force) >= 0.0f)
        return 0.0f;
    const vector3d side = perpendicular(direction);
    bool left = false, right = false;
    for (int i = 0; i < 4; ++i) {
        vector3d corner = obstacle.axes[i / 2] + obstacle.axes[2 + i % 2] + center - position;
        corner.normalize();
        if (dot(corner, side) > 0.0f)
            left = true;
        else
            right = true;
        if (left && right)
            return std::sqrt(magnitude2 >= 0.25f ? magnitude2 : 1.0f - magnitude2);
    }
    return 0.0f;
}


vector3d passing_force(float strength, bool both_sides, const vector3d &velocity, const vector3d &direction,
                       const vector3d &force)
{
    if (strength <= 0.0f)
        return {};
    vector3d side = perpendicular(direction);
    const float speed2 = length_squared(velocity);
    if (!(both_sides && speed2 <= 0.1f)) {
        if (speed2 > 0.25f ? dot(side, velocity) > 0.0f : dot(side, force) < 0.0f)
            side = -side;
    }
    return side * strength;
}

template <class T, unsigned Type, unsigned Parent>
struct avoidance_table : native_inode::table<T, Type, Parent, 18> {
    static void __fastcall destruct(T *self, void *)
    {
        self->destruct_mashed_class();
    }
    static bool __fastcall needs(T *self, void *)
    {
        return self->_does_need_advance();
    }
    static void __fastcall advance(T *self, void *, Float dt)
    {
        self->_frame_advance(dt);
    }
    static void __fastcall activate(T *self, void *, ai_core *core)
    {
        self->_activate(core);
    }
    static void __fastcall collect(T *self, void *)
    {
        self->collect_obstacles();
    }
    static void __fastcall steer(T *self, void *, const vector3d &direction, const vector3d &target, bool stop,
                                 float margin, float dt)
    {
        self->steer(direction, target, stop, margin, dt);
    }
    static bool __fastcall box(T *self, void *, vector3d *axes, vector3d &center, float &top, entity *ent)
    {
        return self->get_entity_box(axes, center, top, ent);
    }
    static float __fastcall radius(T *self, void *, const avoidance_obstacle &obstacle, const vector3d &direction)
    {
        return self->obstacle_radius(obstacle, direction);
    }
    static bool __fastcall accept_entity(T *self, void *, entity *ent)
    {
        return self->accepts_entity(ent);
    }
    static bool __fastcall accept_height(T *self, void *, float top)
    {
        return self->accepts_height(top);
    }
    avoidance_table()
    {
        (*this)[0] = reinterpret_cast<void *>(&destruct);
        (*this)[6] = reinterpret_cast<void *>(&needs);
        (*this)[7] = reinterpret_cast<void *>(&advance);
        (*this)[8] = reinterpret_cast<void *>(&activate);
        (*this)[12] = reinterpret_cast<void *>(&collect);
        (*this)[13] = reinterpret_cast<void *>(&steer);
        (*this)[14] = reinterpret_cast<void *>(&box);
        (*this)[15] = reinterpret_cast<void *>(&radius);
        (*this)[16] = reinterpret_cast<void *>(&accept_entity);
        (*this)[17] = reinterpret_cast<void *>(&accept_height);
    }
};
}  // namespace

void *avoidance_inode::native_vtable()
{
    static avoidance_table<avoidance_inode, 336, 537> table;
    return table.data();
}

void *ped_avoidance_inode::native_vtable()
{
    static avoidance_table<ped_avoidance_inode, 157, 336> table;
    return table.data();
}

avoidance_inode::avoidance_inode() : field_1C(false), field_20(nullptr), field_44(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

avoidance_inode::avoidance_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), field_1C(false), field_20(new _std::vector<avoidance_obstacle>),
      desired_direction(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

avoidance_inode::~avoidance_inode() = default;

void avoidance_inode::destruct_mashed_class()
{
    delete field_20;
    field_20 = nullptr;
    info_node::_destruct_mashed_class();
}

ped_avoidance_inode::ped_avoidance_inode() : field_48(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

ped_avoidance_inode::ped_avoidance_inode(from_mash_in_place_constructor *constructor) : avoidance_inode(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

void avoidance_inode::clear_obstacles()
{
    _std::vector<avoidance_obstacle> empty;
    field_20->swap(empty);
}

void avoidance_inode::refresh_parameters()
{
    field_1C = my_param_block.get_optional_pb_int(respect_obbs_hash, 1, nullptr) != 0;
}

void avoidance_inode::set_respect_obbs(bool value)
{
    my_param_block.set_pb_int(respect_obbs_hash, value, true);
    field_1C = value;
}

void avoidance_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    clear_obstacles();
    field_30 = 0;
    refresh_parameters();
    field_34 = -1;
    field_38 = 0.0f;
    field_3C = 1.0f;
    field_40 = 15;
    if (core->field_50.does_parameter_exist(team_hash))
        field_40 = team::manager::get_team_enum_by_hash(core->field_50.get_pb_hash(team_hash));
    field_44 = my_param_block.get_optional_pb_int(ignore_teammates_hash, 1, nullptr) != 0;
}

bool avoidance_inode::_does_need_advance() const
{
    const int ticks = g_world_ptr->time_manager.field_C;
    return field_34 == -1 || ticks >= field_34 || field_34 - ticks > update_interval;
}

void avoidance_inode::_frame_advance(Float)
{
    if ((field_30 & 1) || field_C->m_player_controller)
        return;
    const int ticks = g_world_ptr->time_manager.field_C;
    if (my_param_block.field_0 == ticks - 1)
        refresh_parameters();
    dispatch_collect();
    if (field_34 == -1) {
        field_34 = ticks + next_update_offset;
        if (++next_update_offset > update_interval)
            next_update_offset = 1;
    } else {
        field_34 = ticks + update_interval;
    }
}

void avoidance_inode::dispatch_collect()
{
    reinterpret_cast<void(__fastcall *)(avoidance_inode *, void *)>(reinterpret_cast<void **>(m_vtbl)[12])(this,
                                                                                                           nullptr);
}
bool avoidance_inode::dispatch_box(vector3d *axes, vector3d &center, float &top, entity *ent)
{
    return reinterpret_cast<bool(__fastcall *)(avoidance_inode *, void *, vector3d *, vector3d &, float &, entity *)>(
        reinterpret_cast<void **>(m_vtbl)[14])(this, nullptr, axes, center, top, ent);
}
float avoidance_inode::dispatch_radius(const avoidance_obstacle &obstacle, const vector3d &direction)
{
    return reinterpret_cast<float(__fastcall *)(
        avoidance_inode *, void *, const avoidance_obstacle &, const vector3d &)>(
        reinterpret_cast<void **>(m_vtbl)[15])(this, nullptr, obstacle, direction);
}
bool avoidance_inode::dispatch_accepts_entity(entity *ent)
{
    return reinterpret_cast<bool(__fastcall *)(avoidance_inode *, void *, entity *)>(
        reinterpret_cast<void **>(m_vtbl)[16])(this, nullptr, ent);
}
bool avoidance_inode::dispatch_accepts_height(float top)
{
    return reinterpret_cast<bool(__fastcall *)(avoidance_inode *, void *, float)>(
        reinterpret_cast<void **>(m_vtbl)[17])(this, nullptr, top);
}

void avoidance_inode::add_entity_obstacle(avoidance_obstacle &out, entity *ent)
{
    out.handle = ent->my_handle;
    out.is_entity = true;
    out.has_box = dispatch_box(out.axes, out.center, out.top, ent);
}

void avoidance_inode::collect_obstacles()
{
    clear_obstacles();
    vector3d velocity = field_C->get_velocity();
    if (length_squared(velocity) < EPSILON)
        return;
    velocity.normalize();
    const vector3d center = field_C->get_abs_position() + velocity;
    auto *list = local_collision::query_sphere(
        center,
        3.0f,
        avoidance_filter,
        *(field_1C ? local_collision::obbfilter_sphere_test : local_collision::obbfilter_reject_all),
        {});
    for (auto *item = list; item; item = item->field_0) {
        avoidance_obstacle obstacle{};
        if (item->is_ent) {
            auto *ent = item->field_4.ent;
            if (!ent || ent == field_C || ent->is_a_parent(field_C) ||
                (ent->has_physical_ifc() && ent->physical_ifc()->field_174))
                continue;
            add_entity_obstacle(obstacle, ent);
        } else {
            auto *box = item->field_4.obb;
            if (!box || box->is_flagged(1))
                continue;
            box_obstacle(obstacle, *box);
        }
        field_20->push_back(obstacle);
    }
    local_collision::destroy_primitive_list(&list);
}

bool avoidance_inode::get_entity_box(vector3d *axes, vector3d &center, float &top, entity *ent)
{
    center = ent->get_abs_position();
    top = center.y;
    if (!ent->is_an_actor() || !ent->colgeom || ent->colgeom->get_type() != collision_geometry::MESH)
        return false;
    auto *mesh = static_cast<cg_mesh *>(ent->colgeom)->data;
    if (mesh->field_C <= 0)
        return false;
    auto &pose = ent->get_abs_po();
    auto &box = mesh->field_10[0];
    const vector3d offset = pose.non_affine_slow_xform(box.field_0);
    vector3d transformed[3] = {pose.non_affine_slow_xform(box.field_10),
                               pose.non_affine_slow_xform(box.axis_y),
                               pose.non_affine_slow_xform(box.axis_z)};
    center += offset;
    top = center.y + std::max(0.0f, std::max(transformed[0].y, std::max(transformed[1].y, transformed[2].y)));
    vector3d a, b;
    horizontal_axes(transformed, a, b);
    axes[0] = offset + a;
    axes[1] = offset - a;
    axes[2] = offset + b;
    axes[3] = offset - b;
    for (int i = 0; i < 4; ++i)
        axes[i].y = 0.0f;
    return true;
}

float avoidance_inode::obstacle_radius(const avoidance_obstacle &obstacle, const vector3d &direction)
{
    float radius = 0.0f;
    if (obstacle.has_box) {
        radius = box_radius(obstacle, direction);
        radius += obstacle.is_entity ? 0.5f : std::clamp((radius - 1.0f) / 0.5f, 0.0f, 1.0f) * 0.5f;
    } else if (auto *ent = static_cast<entity *>(obstacle.handle.get_volatile_ptr())) {
        radius = entity_radius(ent);
    }
    const auto *geometry = field_C->colgeom;
    return radius + (geometry && field_C->colgeom->get_type() == collision_geometry::CAPSULE
                         ? static_cast<collision_capsule *>(field_C->colgeom)->get_core_radius()
                         : 0.5f);
}

bool avoidance_inode::accepts_entity(entity *ent)
{
    if (!ent || (ent->has_physical_ifc() && ent->physical_ifc()->field_174))
        return false;
    if (!field_44)
        return true;
    auto *core = ent->get_ai_core();
    return !core || !core->field_50.does_parameter_exist(team_hash) ||
           team::manager::get_team_enum_by_hash(core->field_50.get_pb_hash(team_hash)) != field_40;
}

bool avoidance_inode::accepts_height(float top)
{
    const float floor = static_cast<entity_base *>(field_C)->get_floor_offset();
    return 0.4f * floor < top - (field_C->get_abs_position().y - floor);
}

vector3d avoidance_inode::avoidance_force(const avoidance_obstacle &obstacle, const vector3d &center,
                                          const vector3d &direction)
{
    vector3d away = field_C->get_abs_position() - center;
    float distance = away.length();
    if (distance < EPSILON)
        return {};
    away /= distance;
    distance = std::max(float(EPSILON), distance - dispatch_radius(obstacle, away));
    if (distance > 5.0f)
        return {};
    if (dot(-away, direction) > 0.9848f)
        distance *= 0.5f;
    if (auto *ent = obstacle.handle.get_volatile_ptr()) {
        vector3d velocity = ent->get_abs_po().get_z_facing();
        if (ent->is_an_actor())
            velocity = static_cast<actor *>(ent)->get_velocity();
        if (length_squared(velocity) > 0.25f && dot(velocity, away) > 0.9848f)
            distance *= 0.5f;
    }
    return away * std::min(0.8f, (1.0f / distance - 1.0f / 5.0f) * 0.22f);
}

float avoidance_inode::target_overlap(const avoidance_obstacle &obstacle, const vector3d &target, float margin)
{
    vector3d direction = target - obstacle.center;
    direction.y = 0.0f;
    const float distance = direction.length();
    vector3d radial = direction;
    if (distance > EPSILON)
        direction = radial = direction / distance;
    else {
        radial = field_C->get_abs_position() - obstacle.center;
        radial.y = 0.0f;
        radial.normalize();
    }
    const float overlap = dispatch_radius(obstacle, radial) - distance;
    if (overlap > 0.0f)
        return overlap;
    if (overlap <= -margin)
        return 0.0f;
    vector3d from_target = field_C->get_abs_position() - target;
    from_target.normalize();
    return dot(from_target, direction) > 0.34f ? overlap + margin : 0.0f;
}

void avoidance_inode::steer(const vector3d &direction, const vector3d &target, bool stop_at_target, float margin,
                            float dt)
{
    desired_direction = direction;
    if ((field_30 & 1) || field_C->m_player_controller ||
        (direction.x <= 0.0f && direction.x >= 0.0f && direction.y <= 0.0f && direction.y >= 0.0f &&
         direction.z <= 0.0f && direction.z >= 0.0f))
        return;
    field_30 = 0;
    const vector3d position = field_C->get_abs_position();
    vector3d correction;
    float strength = 0.0f;
    vector3d obstacle_velocity;
    bool left = false, right = false;
    auto accumulate = [&](const vector3d &origin, const vector3d &travel, float prediction, bool first) {
        for (auto &obstacle : *field_20) {
            auto *ent = static_cast<entity *>(obstacle.handle.get_volatile_ptr());
            if ((obstacle.is_entity && !dispatch_accepts_entity(ent)) || !dispatch_accepts_height(obstacle.top))
                continue;
            vector3d velocity;
            vector3d center = obstacle.center;
            if (!first) {
                if (ent && ent->is_an_actor())
                    velocity = static_cast<actor *>(ent)->get_velocity();
                center += velocity * prediction;
            }
            center.y = origin.y;
            vector3d force = avoidance_force(obstacle, center, travel);
            float overlap = length_squared(force) > 0.0f ? target_overlap(obstacle, target, margin) : 0.0f;
            if (overlap > 0.0f) {
                if (first)
                    field_30 |= 4;
                if (stop_at_target) {
                    force = {};
                    if (first)
                        field_30 |= 8;
                }
            }
            if (length_squared(force) > EPSILON) {
                if (dot(center - origin, perpendicular(direction)) > 0.0f)
                    left = true;
                else
                    right = true;
                if (overlap < EPSILON) {
                    const float block = blocking_strength(obstacle, origin, center, travel, force);
                    if (block > 0.0f) {
                        strength = std::max(strength, block);
                        if (first && ent && ent->is_an_actor())
                            velocity = static_cast<actor *>(ent)->get_velocity();
                        obstacle_velocity += velocity;
                    }
                }
            }
            correction += force;
        }
    };
    accumulate(position, direction, 0.0f, true);
    if (length_squared(correction) <= EPSILON)
        return;
    correction += passing_force(strength, left && right, obstacle_velocity, direction, correction);
    vector3d adjusted = direction + correction;
    adjusted.normalize();
    const float prediction = std::clamp(dt, 0.033333335f, 0.06666667f);
    const vector3d future = position + adjusted * (field_C->get_velocity().length() * prediction);
    vector3d travel = target - future;
    if (length_squared(travel) >= EPSILON)
        travel.normalize();
    else
        travel = field_C->get_abs_po().get_z_facing();
    correction = {};
    strength = 0.0f;
    obstacle_velocity = {};
    left = right = false;
    accumulate(future, travel, prediction, false);
    correction += passing_force(strength, left && right, obstacle_velocity, direction, correction);
    if (length_squared(correction) > EPSILON) {
        vector3d next = correction + travel;
        if (length_squared(next) > LARGE_EPSILON)
            next /= next.length();
        if (dot(next, adjusted) < 0.4f) {
            field_38 += dt;
            if (field_38 > 0.8f) {
                field_30 |= 2;
                if (field_38 > 1.6f) {
                    field_38 -= 0.8f;
                    field_3C *= -1.0f;
                }
                desired_direction = vector3d::cross(direction, UP) * field_3C;
            }
            return;
        }
    }
    field_30 |= 2;
    desired_direction = adjusted;
    field_38 = 0.0f;
}

void ped_avoidance_inode::_activate(ai_core *core)
{
    field_4C = static_cast<pedestrian_inode *>(core->get_info_node(pedestrian_inode::default_id, true));
    field_48 = !field_4C->is_flagged(1);
    avoidance_inode::_activate(core);
    if (field_4C->is_flagged(1))
        set_respect_obbs(false);
    field_50 = FLT_MAX;
}

bool ped_avoidance_inode::_does_need_advance() const
{
    return !field_48 || avoidance_inode::_does_need_advance();
}

void ped_avoidance_inode::_frame_advance(Float dt)
{
    if (field_48) {
        avoidance_inode::_frame_advance(dt);
    } else if (!(field_30 & 1)) {
        const auto &camera_position = g_game_ptr->get_current_view_camera(0)->get_abs_position();
        if (length_squared(camera_position - field_C->get_abs_position()) < 35.0f * 35.0f) {
            if (field_8->field_6C->field_44)
                dispatch_collect();
            else
                avoidance_inode::_frame_advance(dt);
        }
    }
}

void ped_avoidance_inode::collect_boxes()
{
    vector3d velocity = field_C->get_velocity();
    if (length_squared(velocity) <= EPSILON)
        return;
    velocity.normalize();
    const vector3d center = field_C->get_abs_position() + velocity;
    auto *list = local_collision::query_sphere(
        center, 3.0f, *local_collision::entfilter_reject_all, *local_collision::obbfilter_sphere_test, {});
    for (auto *item = list; item; item = item->field_0) {
        auto *box = item->field_4.obb;
        if (!box || box->is_flagged(1))
            continue;
        avoidance_obstacle obstacle{};
        box_obstacle(obstacle, *box);
        if (dispatch_accepts_height(obstacle.top))
            field_20->push_back(obstacle);
    }
    local_collision::destroy_primitive_list(&list);
}

bool ped_avoidance_inode::add_pedestrian(entity *ent)
{
    if (ent == field_C || !(ent->field_4 & 0x200))
        return false;
    const vector3d offset = ent->get_abs_position() - field_C->get_abs_position();
    const float distance2 = length_squared(offset);
    if (distance2 >= 0.5625f && (distance2 >= 12.25f || dot(offset, field_C->get_abs_po().get_z_facing()) <= 0.3f))
        return false;
    avoidance_obstacle obstacle{};
    add_entity_obstacle(obstacle, ent);
    field_20->push_back(obstacle);
    field_50 = std::min(field_50, distance2);
    return true;
}

void ped_avoidance_inode::collect_pedestrians()
{
    for (auto *spawner : ped_spawner::ped_spawner_list)
        if (spawner)
            if (auto *ent = spawner->get_my_actor())
                add_pedestrian(ent);
}

void ped_avoidance_inode::collect_obstacles()
{
    if (field_48) {
        avoidance_inode::collect_obstacles();
        if (field_4C->m_ped_spawner) {
            field_50 = FLT_MAX;
            collect_pedestrians();
        }
    } else if (length_squared(field_C->get_velocity()) >= EPSILON || (field_30 & 0x10)) {
        clear_obstacles();
        if (field_1C)
            collect_boxes();
        field_50 = FLT_MAX;
        collect_pedestrians();
        if (pedestrian_inode::non_ped_list)
            for (const auto &handle : *pedestrian_inode::non_ped_list)
                if (auto *ent = handle.get_volatile_ptr())
                    add_pedestrian(ent);
    }
}

float ped_avoidance_inode::obstacle_radius(const avoidance_obstacle &obstacle, const vector3d &direction)
{
    if (field_48 || obstacle.has_box)
        return avoidance_inode::obstacle_radius(obstacle, direction);
    float projection = 0.0f;
    if (auto *ent = obstacle.handle.get_volatile_ptr())
        projection = std::fabs(dot(ent->get_abs_po().get_z_facing(), direction)) * 0.3f;
    return projection + 0.05f;
}

bool ped_avoidance_inode::blocks_translation(const vector3d &translation)
{
    const auto &position = field_C->get_abs_position();
    const float target_x = position.x + translation.x;
    const float target_z = position.z + translation.z;
    for (const auto &obstacle : *field_20) {
        auto *ent = static_cast<entity *>(obstacle.handle.get_volatile_ptr());
        if (obstacle.is_entity && !dispatch_accepts_entity(ent))
            continue;
        if (!dispatch_accepts_height(obstacle.top))
            continue;
        auto *player = g_world_ptr->get_hero_ptr(0);
        if (player != nullptr && obstacle.is_entity && obstacle.handle.field_0 == player->my_handle.field_0)
            continue;
        vector3d radial{target_x - obstacle.center.x, 0.0f, target_z - obstacle.center.z};
        if (!(dot(radial, translation) < 0.0f))
            continue;
        const float distance = radial.length();
        if (distance < EPSILON)
            return true;
        radial /= distance;
        const float clearance = obstacle.has_box ? distance - box_radius(obstacle, radial)
                                : ent != nullptr ? distance - entity_radius(ent)
                                                 : FLT_MAX;
        if (clearance < 0.7f)
            return true;
    }
    return false;
}

void ped_avoidance_inode::steer(const vector3d &direction, const vector3d &target, bool stop_at_target, float margin,
                                float dt)
{
    avoidance_inode::steer(direction, target, stop_at_target, margin, dt);
    if (field_48 || field_4C->is_flagged(0x10) || !field_4C->is_flagged(2))
        return;
    for (auto &obstacle : *field_20) {
        if (!obstacle.is_entity)
            continue;
        auto *ent = obstacle.handle.get_volatile_ptr();
        if (!ent)
            continue;
        vector3d offset = ent->get_abs_position() - field_C->get_abs_position();
        if (length_squared(offset) >= 4.0f)
            continue;
        vector3d velocity = static_cast<entity_base *>(field_C)->get_velocity();
        vector3d other_velocity = ent->get_velocity();
        if (length_squared(velocity) > 1.0f && length_squared(other_velocity) > 1.0f) {
            velocity.normalize();
            offset.normalize();
            if (dot(velocity, offset) > 0.5f) {
                other_velocity.normalize();
                if (dot(other_velocity, velocity) > 0.8f)
                    field_30 |= 0x10;
            }
        }
    }
    if (!(field_30 & 0x10) && (field_30 & 2)) {
        float left = 0.55f, right = 0.55f;
        if (field_4C->is_on_right_side_of_road())
            right = 0.2f;
        else
            left = 0.2f;
        const vector3d lane_direction = field_4C->get_desired_dir();
        const float fraction = field_4C->get_lane_fraction(lane_direction);
        const float side = lane_direction.x * desired_direction.z - lane_direction.z * desired_direction.x;
        if ((fraction < right && side > 0.0f) || (fraction > 2.0f - left && side <= 0.0f))
            field_30 |= 0x10;
    }
}

}  // namespace ai
