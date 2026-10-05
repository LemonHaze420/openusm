#include "prop_system.h"

#include "actor.h"
#include "advanced_entity_ptrs.h"
#include "collision_event.h"
#include "colmesh.h"
#include "common.h"
#include "conglom.h"
#include "damage_interface.h"
#include "event_manager.h"
#include "moved_entities.h"
#include "nuge.h"
#include "physical_interface.h"
#include "physics_system.h"
#include "phys_vector3d.h"
#include "rigid_body.h"
#include "script.h"
#include "script_object.h"
#include "sound_and_pfx_interface.h"
#include "subdivision_obb.h"
#include "terrain_types_manager.h"
#include "utility.h"
#include "wds.h"
#include "local_collision.h"
#include "rbc_def_contact.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>

namespace prop_system {
namespace {
constexpr int max_props = 32;
struct prop_pool {
    prop_physics_body slots[max_props];
    prop_physics_body *allocated[max_props];
    int indices[max_props];
    int capacity;
    int count;

    prop_pool() : capacity(max_props), count(0)
    {
        for (int i = 0; i != max_props; ++i) {
            allocated[i] = &slots[i];
            indices[i] = i;
        }
    }

    prop_physics_body *allocate()
    {
        return count < capacity ? allocated[count++] : nullptr;
    }

    void release(prop_physics_body *record)
    {
        const int slot = static_cast<int>(record - slots);
        const int index = indices[slot];
        assert(index >= 0 && index < count);
        auto *last = allocated[--count];
        allocated[index] = last;
        allocated[count] = record;
        indices[last - slots] = index;
        indices[slot] = count;
    }
};

struct prop_control {
    bool active;
    int priority;
    float remaining;
    int body_count;
    entity_base_vhandle owner;
};
VALIDATE_SIZE(prop_physics_body, 0x4C);
VALIDATE_SIZE(prop_pool, 0xA88);
VALIDATE_SIZE(prop_control, 0x14);
static Var<prop_control[max_props]> controls{0x0095A780};
static Var<int> used_slots{0x00923814};
static Var<bool> slots_dirty{0x00965944};

prop_pool &pool()
{
#if STANDALONE_SYSTEM
    static prop_pool storage;
    return storage;
#else
    return *var<prop_pool *>(0x00922D90);
#endif
}

bool can_be_a_prop(actor *owner)
{
    return owner != nullptr && owner->has_physical_ifc() && owner->colgeom != nullptr &&
        owner->colgeom->get_type() == collision_geometry::MESH;
}

int count_props(actor *owner)
{
    if ((owner->physical_ifc()->field_C & 0x400000) != 0)
        return 0;
    int count = 1;
    if ((owner->field_4 & 4) != 0) {
        auto &members = static_cast<conglomerate *>(owner)->members;
        for (uint16_t i = 0; i < members.m_size; ++i) {
            auto *member = members.at(i);
            if (member->is_an_actor() && can_be_a_prop(static_cast<actor *>(member)))
                ++count;
        }
    }
    return count;
}

void calc_bounds(const cg_mesh &mesh, vector3d &dimensions, vector3d &center)
{
    vector3d minimum{10000000.0f, 10000000.0f, 10000000.0f};
    vector3d maximum{-10000000.0f, -10000000.0f, -10000000.0f};
    for (int i = 0; i < mesh.data->field_C; ++i) {
        const auto &obb = mesh.data->field_10[i];
        if (!obb.field_34)
            continue;
        const auto &triangles = obb.triangles();
        for (uint16_t vertex = 0; vertex < triangles.vertex_count; ++vertex) {
            for (int axis = 0; axis != 3; ++axis) {
                minimum[axis] = std::min(minimum[axis], triangles.vertices[vertex][axis]);
                maximum[axis] = std::max(maximum[axis], triangles.vertices[vertex][axis]);
            }
        }
    }
    center = (maximum + minimum) * 0.5f;
    dimensions = maximum - minimum;
}

void create_prop(actor *owner, const vector3d &velocity, float randomness)
{
    if (owner->colgeom == nullptr)
        return;
    owner->colgeom->field_C &= ~0x12u;
    auto *physics = owner->physical_ifc();
    physics->set_allow_manage_standing(false);
    physics->set_gravity(true);
    owner->set_collisions_active(false, true);
    physics->enable(false);
    vector3d dimensions, center;
    calc_bounds(*static_cast<cg_mesh *>(owner->colgeom), dimensions, center);
    phys_vector3d box_dimensions{dimensions};
    phys_vector3d inertia;
    float volume;
    nuge::calc_box_inertia(box_dimensions, inertia, volume);
    const float mass = std::max(volume, 1.0f);
    inertia = (mass / volume) * inertia;
    po transform = owner->get_abs_po();
    transform.set_position(transform.slow_xform(center));
    po offset{identity_matrix};
    offset.set_position(-center);
    const float scale = randomness * (1.0f / 16384.0f);

    const float velocity_z = static_cast<float>(std::rand() - 0x4000) * scale;
    const float velocity_y = static_cast<float>(std::rand() - 0x4000) * scale;
    phys_vector3d randomized_velocity;
    randomized_velocity[0] = static_cast<float>(std::rand() - 0x4000) * scale + velocity.x;
    randomized_velocity[1] = velocity_y + velocity.y;
    randomized_velocity[2] = velocity_z + velocity.z;
    const float angular_z = static_cast<float>(std::rand() - 0x4000) * scale;
    const float angular_y = static_cast<float>(std::rand() - 0x4000) * scale;
    phys_vector3d angular_velocity;
    angular_velocity[0] = static_cast<float>(std::rand() - 0x4000) * scale;
    angular_velocity[1] = angular_y;
    angular_velocity[2] = angular_z;
    auto *body = phys_sys::create_rigid_body();
    body->set(mass, inertia, transform.m, randomized_velocity, angular_velocity, 2.0f, 3);
    body->field_138 = 20.0f;
    auto *record = pool().allocate();
    record->mesh_offset = offset;
    record->body = body;
    record->owner = owner;
    record->contact = false;
    physics->field_174 = record;
}

void add_prop(actor *owner, const vector3d &velocity, float randomness)
{
    slots_dirty() = true;
    if ((owner->physical_ifc()->field_C & 0x400000) != 0)
        return;
    create_prop(owner, velocity, randomness);
    if ((owner->field_4 & 4) != 0) {
        auto &members = static_cast<conglomerate *>(owner)->members;
        for (uint16_t i = 0; i < members.m_size; ++i) {
            auto *member = members.at(i);
            if (member->is_an_actor() && can_be_a_prop(static_cast<actor *>(member)))
                create_prop(static_cast<actor *>(member), velocity, randomness);
        }
    }
}

void destroy_prop(actor *owner)
{
    owner->colgeom->field_C |= 2u;
    auto *physics = owner->physical_ifc();
    auto *record = physics->field_174;
    phys_sys::destroy(record->body);
    pool().release(record);
    physics->field_174 = nullptr;
}

void set_relative_pose(entity_base *owner, const po &parent)
{
    const auto &p = parent.m;
    const float determinant =
        p[0][0] * (p[1][1] * p[2][2] - p[1][2] * p[2][1]) -
        p[0][1] * (p[1][0] * p[2][2] - p[1][2] * p[2][0]) +
        p[0][2] * (p[1][0] * p[2][1] - p[1][1] * p[2][0]);
    const float inverse[3][3] = {
        {(p[1][1] * p[2][2] - p[1][2] * p[2][1]) / determinant,
         (p[0][2] * p[2][1] - p[0][1] * p[2][2]) / determinant,
         (p[0][1] * p[1][2] - p[0][2] * p[1][1]) / determinant},
        {(p[1][2] * p[2][0] - p[1][0] * p[2][2]) / determinant,
         (p[0][0] * p[2][2] - p[0][2] * p[2][0]) / determinant,
         (p[0][2] * p[1][0] - p[0][0] * p[1][2]) / determinant},
        {(p[1][0] * p[2][1] - p[1][1] * p[2][0]) / determinant,
         (p[0][1] * p[2][0] - p[0][0] * p[2][1]) / determinant,
         (p[0][0] * p[1][1] - p[0][1] * p[1][0]) / determinant},
    };
    const auto &absolute = owner->my_abs_po->m;
    auto &relative = owner->my_rel_po->m;
    for (int row = 0; row != 4; ++row) {
        for (int column = 0; column != 3; ++column) {
            const float x = absolute[row][0] - (row == 3 ? p[3][0] : 0.0f);
            const float y = absolute[row][1] - (row == 3 ? p[3][1] : 0.0f);
            const float z = absolute[row][2] - (row == 3 ? p[3][2] : 0.0f);
            relative[row][column] = x * inverse[0][column] + y * inverse[1][column] + z * inverse[2][column];
        }
        relative[row][3] = row == 3 ? 1.0f : 0.0f;
    }
}

void advance_prop(entity_base *owner, int &count)
{
    if ((owner->field_4 & 0x8000) == 0 && count != 0) {
        owner->field_8 |= 0x10000000;
    } else {
        if ((owner->field_8 & 0x8000000) != 0)
            owner->compute_rel_po_from_model();
        auto *parent = owner->m_parent;
        if (owner->has_physical_ifc() && owner->is_an_actor() &&
            static_cast<actor *>(owner)->colgeom != nullptr &&
            static_cast<actor *>(owner)->colgeom->get_type() == collision_geometry::MESH) {
            auto *physics = owner->physical_ifc();
            if (auto *record = physics->field_174) {
                owner->my_abs_po->m = record->mesh_offset.m * record->body->field_0;
                record->body->field_C0 = physics->field_74.x;
                record->body->field_C4 = physics->field_74.y;
                record->body->field_C8 = physics->field_74.z;
                record->body->field_CC = 0.0f;
                record->body->field_134 = (physics->field_C & 4) != 0 && physics->field_A4 <= 0.0f
                    ? (physics->field_D8 <= 0.0f ? physics->m_gravity_multiplier : physics->field_D4)
                    : 0.0f;
                if (parent != nullptr && (record->body->field_144 & 8) != 0)
                    destroy_prop(static_cast<actor *>(owner));
                else
                    ++count;
            }
            if (parent == nullptr)
                *owner->my_rel_po = *owner->my_abs_po;
            else
                set_relative_pose(owner, parent->get_abs_po());
        } else if (parent != nullptr) {
            owner->my_abs_po->m = owner->my_rel_po->m * parent->get_abs_po().m;
        }
        owner->field_8 &= ~0x10000000u;
    }
    for (auto *child = owner->m_child; child != nullptr; child = child->field_28)
        advance_prop(child, count);
}

bool __fastcall prop_environment_filter(const local_collision::entfilter_base *, void *,
    actor *candidate, dynamic_conglomerate_clone *, const local_collision::query_args_t *args)
{
    return candidate != args->field_2C && candidate->has_entity_collision();
}

void collide_prop_environment(actor *owner, int &contact_count)
{
    auto *record = owner->physical_ifc()->field_174;
    auto *body = record->body;
    auto *mesh = static_cast<cg_mesh *>(owner->colgeom);
    po predicted = po_identity_matrix;
    po::compose(predicted, reinterpret_cast<const po &>(body->field_40), record->mesh_offset);
    const vector3d center = predicted.slow_xform(mesh->get_local_space_bounding_sphere_center());
    const float radius = mesh->get_bounding_sphere_radius();
    static const local_collision::entfilter_base::native_vtable filter_table{prop_environment_filter};
    static const local_collision::entfilter_base filter{reinterpret_cast<std::intptr_t>(&filter_table)};
    local_collision::query_args_t args{};
    args.set_entity(owner);
    auto *primitives = local_collision::query_sphere(center, radius, filter,
        *local_collision::obbfilter_sphere_test, args);
    auto *intersections = local_collision::get_all_sphere_intersections(primitives, center, radius);
    entity *notified[40];
    int notified_count = 0;
    int point_budget = 0;
    for (auto *intersection = intersections; intersection != nullptr;
         intersection = reinterpret_cast<local_collision::intersection_list_t *>(intersection->field_0)) {
        auto *other = intersection->is_ent ? static_cast<entity *>(intersection->intersection_node) : nullptr;
        if (other != nullptr && other->is_an_actor()) {
            auto *other_actor = static_cast<actor *>(other);
            if (!owner->allow_collision(other_actor->get_my_vhandle()) ||
                !other_actor->allow_collision(owner->get_my_vhandle()))
                continue;
        }
        const vector3d normal = -intersection->normal;
        vector3d local_normal;
        for (int axis = 0; axis < 3; ++axis)
            local_normal[axis] = predicted[axis][0] * normal.x +
                predicted[axis][1] * normal.y + predicted[axis][2] * normal.z;
        const vector3d local_point = predicted.inverse_xform(intersection->point);
        auto *environment = phys_sys::get_environment_rigid_body();
        rigid_body_constraint_contact *contact = nullptr;
        for (int leaf = 0; leaf < mesh->data->field_C; ++leaf) {
            const auto &obb = mesh->data->field_10[leaf];
            if (!obb.field_34)
                continue;
            const auto &vertices = obb.triangles();
            for (uint16_t vertex = 0; vertex < vertices.vertex_count; ++vertex) {
                if (contact_count >= 256 || point_budget >= 127)
                    continue;
                const auto &point = vertices.vertices[vertex];
                const float separation = dot(point - local_point, local_normal);
                if (separation < -0.05f)
                    continue;
                if (contact == nullptr)
                    contact = phys_sys::create_no_error_rbc_contact(body, environment);
                if (contact == nullptr)
                    continue;
                const phys_vector3d body_point{record->mesh_offset.slow_xform(point)};
                const phys_vector3d environment_point{predicted.slow_xform(point - separation * local_normal)};
                const phys_vector3d contact_normal{normal};
                contact->add_point(body, environment, body_point, environment_point,
                    contact_normal, 0.60000002f, 0.30000001f, 100.0f, true);
                ++contact_count;
                point_budget += 2;
            }
        }
        bool already_notified = false;
        for (int index = 0; index < notified_count; ++index)
            already_notified |= notified[index] == other;
        if (!already_notified) {
            if (other != nullptr) {
                collision_event owner_event{other->get_my_vhandle(), nullptr,
                    intersection->point, intersection->normal};
                event_manager::raise_event(&owner_event, owner->get_my_vhandle());
                collision_event other_event{owner->get_my_vhandle(), nullptr,
                    intersection->point, -intersection->normal};
                event_manager::raise_event(&other_event, other->get_my_vhandle());
            } else if (intersection->intersection_node != nullptr) {
                collision_event terrain_event{{},
                    static_cast<const subdivision_node *>(intersection->intersection_node),
                    intersection->point, intersection->normal};
                event_manager::raise_event(&terrain_event, owner->get_my_vhandle());
            }
            if (notified_count < 40)
                notified[notified_count++] = other;
        }
        if (slots_dirty())
            break;
    }
    local_collision::destroy_intersection_list(&intersections);
    local_collision::destroy_primitive_list(&primitives);
}

}

void environment_collision_callback(int &contact_count)
{
    for (int index = 0; index < pool().count; ++index)
        pool().allocated[index]->contact = false;
    int index = 0;
    while (index < pool().count) {
        auto *record = pool().allocated[index];
        slots_dirty() = false;
        if (!record->contact) {
            record->contact = true;
            collide_prop_environment(record->owner, contact_count);
            if (slots_dirty()) {
                index = 0;
                continue;
            }
        }
        ++index;
    }
}

void collision_callback(event *base_event, entity_base_vhandle handle, void *)
{
    auto *owner = static_cast<actor *>(handle.get_volatile_ptr());
    if (owner == nullptr)
        return;
    const auto &collision = *static_cast<collision_event *>(base_event);
    auto *other = static_cast<entity *>(collision.other.get_volatile_ptr());
    auto *physics = owner->physical_ifc();
    float mass = physics->field_174 != nullptr ? 1.0f / physics->field_174->body->field_130 : physics->field_8C;
    if ((physics->field_C & 0x800000) != 0 && mass < 7.5f)
        mass = 7.5f;
    float magnitude = 0.0f;
    string_hash terrain{static_cast<int>(to_hash("DEFAULT"))};
    if (other != owner) {
        vector3d other_velocity = ZEROVEC;
        if (other != nullptr && other->has_physical_ifc())
            other_velocity = other->physical_ifc()->get_velocity();
        const vector3d relative_velocity = physics->get_velocity() - other_velocity;
        magnitude = std::fabs(dot(relative_velocity, collision.normal)) * collision.normal.length();
        terrain = other != nullptr
            ? terrain_types_manager::get_terrain_type_by_index(static_cast<unsigned char>(other->field_41))
            : physical_interface::calc_obb_face_terrain_type(owner->get_abs_position(),
                static_cast<subdivision_node_obb_base *>(const_cast<subdivision_node *>(collision.obb)));
    }
    if (owner->has_sound_and_pfx_ifc()) {
        const float volume = std::clamp((magnitude - 5.0f) / 15.0f, 0.0f, 1.0f);
        auto *sound = owner->sound_and_pfx_ifc();
        if (volume > 0.0f && sound->field_24 <= 0.0f) {
            sound->field_24 = 0.3f;
            sound->play_terrain_sound(TERRAIN_SOUND_PHYSICS_IMPACT, terrain, volume);
        }
    }
    if (mass >= 7.5f) {
        const int damage = static_cast<int>((magnitude - 5.0f) * 2.0f);
        if (owner->has_damage_ifc() && damage > 0) {
            const string_hash empty{0};
            owner->damage_ifc()->apply_damage(other != nullptr && other->is_an_actor() ? other : nullptr,
                static_cast<float>(damage), 1, collision.position, collision.normal, 0,
                empty, empty, empty, false, ZEROVEC, 17, false);
        }
        if (other != nullptr && other->is_an_actor()) {
            auto *actor = static_cast<::actor *>(other);
            if (actor->adv_ptrs != nullptr && actor->adv_ptrs->my_script != nullptr) {
                auto *instance = actor->adv_ptrs->my_script;
                const string_hash function{"prop_physics_collision(entity,entity,vector3d,vector3d)"};
                const int index = script::find_function(function, instance->parent, true);
                if (index >= 0) {
                    script::new_thread(index, instance);
                    script::push_arg(owner);
                    script::push_arg(other);
                    script::push_arg(collision.position);
                    script::push_arg(collision.normal);
                    script::exec_thread(true);
                }
            }
        }
    }
}

bool start(actor *owner, const vector3d &velocity, float randomness, float lifetime, int priority)
{
    auto *physics = owner->physical_ifc();
    if (physics->is_prop_physics_running())
        return true;
    const int needed = count_props(owner);
    int used = used_slots();
    int free_control = -1;
    for (int i = 0; i != max_props; ++i)
        if (!controls()[i].active)
            free_control = i;
    while (free_control == -1 || needed + used > max_props) {
        int victim = -1;
        for (int i = 0; i != max_props; ++i) {
            const auto &candidate = controls()[i];
            if (candidate.active && (victim == -1 ||
                candidate.priority < controls()[victim].priority ||
                (candidate.priority == controls()[victim].priority &&
                 candidate.remaining < controls()[victim].remaining)))
                victim = i;
        }
        if (victim == -1 || controls()[victim].priority == 3)
            return false;
        used -= controls()[victim].body_count;
        if (auto *previous = static_cast<actor *>(controls()[victim].owner.get_volatile_ptr())) {
            if (previous->has_physical_ifc())
                previous->physical_ifc()->stop_prop_physics(false);
        }
        if (free_control == -1)
            free_control = victim;
    }
    physics->enable(true);
    add_prop(owner, velocity, randomness);
    auto &control = controls()[free_control];
    control.active = true;
    control.owner = owner->get_my_vhandle();
    control.priority = priority;
    control.body_count = needed;
    control.remaining = lifetime < 0.0f ? 20.0f : lifetime;
    event_manager::raise_event(event::PROP_PHYSICS_START, owner->get_my_vhandle());
    physics->field_80 = event_manager::add_callback(event::COLLISION_EVENT,
        owner->get_my_vhandle(), collision_callback, nullptr, false);
    return true;
}

void stop(actor *owner, bool destroying)
{
    auto *physics = owner->physical_ifc();
    if (!physics->is_prop_physics_running())
        return;
    slots_dirty() = true;
    moved_entities::add_moved(vhandle_type<entity>{owner->get_my_vhandle()});
    destroy_prop(owner);
    if ((owner->field_4 & 4) != 0) {
        auto &members = static_cast<conglomerate *>(owner)->members;
        for (uint16_t i = 0; i < members.m_size; ++i) {
            auto *member = members.at(i);
            if (member->is_an_actor() && member->has_physical_ifc() &&
                member->physical_ifc()->is_prop_physics_running())
                destroy_prop(static_cast<actor *>(member));
        }
    }
    if (!destroying)
        owner->invalidate_frame_delta();
    for (auto &control : controls()) {
        if (control.active && control.owner.get_volatile_ptr() == owner) {
            control.active = false;
            control.owner = entity_base_vhandle{0};
            control.priority = 0;
            control.remaining = -1.0f;
            break;
        }
    }
    event_manager::remove_callback(physics->field_80, event::COLLISION_EVENT, owner->get_my_vhandle());
    physics->field_80 = 0;
    physics->enable(false);
    event_manager::raise_event(event::PROP_PHYSICS_STOP, owner->get_my_vhandle());
}

void frame_advance(Float elapsed)
{
    for (auto &control : controls()) {
        if (!control.active)
            continue;
        auto *owner = static_cast<actor *>(control.owner.get_volatile_ptr());
        if (owner == nullptr || !owner->has_physical_ifc()) {
            control.active = false;
            continue;
        }
        int count = 0;
        advance_prop(owner, count);
        if ((owner->field_4 & 4) != 0) {
            owner->field_8 |= 0x40;
            static_cast<conglomerate *>(owner)->field_110 |= 1;
        }
        control.body_count = count;
        control.remaining -= elapsed.value;
        if (control.remaining < 0.0f)
            owner->physical_ifc()->stop_prop_physics(false);
    }
    if (slots_dirty()) {
        used_slots() = pool().count;
        slots_dirty() = false;
    }
}
}
