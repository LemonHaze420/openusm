#include "grenade.h"

#include "common.h"
#include "entity_mash.h"
#include "thrown_item.h"
#include "guidance_sys.h"
#include "physical_interface.h"
#include "beam.h"
#include "collide.h"
#include "local_collision.h"
#include "damage_interface.h"
#include "damage_inode.h"
#include "base_ai_core.h"
#include "combat_inode.h"
#include "combo_system_move.h"
#include "motion_effect_struct.h"
#include "event.h"
#include "event_manager.h"
#include "sound_and_pfx_interface.h"
#include "script.h"
#include "chuck/vm/script_object.h"
#include "chuck/vm/vm_thread.h"
#include "oldmath_po.h"
#include "region.h"
#include "terrain.h"
#include "subdivision_obb.h"
#include "memory.h"
#include "time_interface.h"
#include "variable.h"
#include "variables.h"
#include "vtbl.h"
#include "wds.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <new>
#include <vector>


namespace {
Var<grenade *> active_grenades{0x0095C76C};
Var<grenade *> inactive_grenades{0x0095C768};

template <typename R, typename... Args>
R invoke(entity_base *self, unsigned offset, Args... args)
{
    return reinterpret_cast<R (__fastcall *)(entity_base *, void *, Args...)>(
        get_vfunc(self->m_vtbl, offset))(self, nullptr, args...);
}

thrown_item *weapon_owner(grenade *value)
{
    return static_cast<thrown_item *>(value->weapon.get_volatile_ptr());
}

void stop_sound(sound_instance_id &id)
{
    if (auto *sound = id.get_sound_instance_ptr()) {
        sound->stop();
        id.field_0 = 0;
    }
}

void drain_trail(entity_base *value)
{
    if (value->field_18 && value->field_18->trail_active)
        value->field_18->draining_trail = true;
}

void *__fastcall grenade_delete(grenade *self, void *, unsigned flags)
{
    self->~grenade();
    if (flags & 1)
        mem_dealloc(self, sizeof(grenade));
    return self;
}
bool __fastcall grenade_query(grenade *, void *) { return true; }
bool __fastcall grenade_guided(grenade *self, void *)
{
    auto *weapon = weapon_owner(self);
    return weapon && (weapon->field_10C & 0x4000);
}
void __fastcall grenade_advance(grenade *self, void *, Float elapsed) { self->frame_advance(elapsed); }
void __fastcall grenade_render(grenade *self, void *, Float time)
{
    if (!self->visual.get_volatile_ptr())
        self->_render(time);
}
entity *__fastcall grenade_hit(grenade *self, void *, const vector3d &start, const vector3d &end,
                               vector3d &hit, int *kind)
{
    return self->check_hit(start, end, hit, kind);
}
bool __fastcall grenade_hit_entity(grenade *self, void *, entity *target, const vector3d &start,
                                    const vector3d &end, vector3d &hit, int *kind)
{
    return self->check_if_hit(target, start, end, hit, kind);
}
void __fastcall grenade_clear(grenade *self, void *, bool faded) { self->clear(faded); }
void __fastcall grenade_detonate(grenade *self, void *, int reason, entity *hit) { self->detonate(reason, hit); }
void __fastcall grenade_intercept(grenade *self, void *) { self->intercept(); }
void __fastcall grenade_redirect_position(grenade *self, void *, const vector3d &position, entity_base *source)
{
    self->redirect(position, source);
}
void __fastcall grenade_redirect_handle(grenade *self, void *, entity_base_vhandle target, entity_base *source)
{
    self->redirect(target, source);
}
void __fastcall grenade_redirect_direction(grenade *self, void *, const vector3d &direction, entity_base *source)
{
    self->redirect_dir(direction, source);
}

void collect_segment_regions(region_array &regions, region *origin, const vector3d &start, const vector3d &end)
{

    if (!origin)
        return;
    for (int i = 0; i < regions.count; ++i)
        if (regions.m_data[i] == origin)
            return;
    if (regions.count < 30)
        regions.m_data[regions.count++] = origin;
    for (auto neighbor : origin->neighbors) {
        auto *connected = g_world_ptr->the_terrain->regions[neighbor];
        if (connected->obb->line_segment_intersection(start, end))
            collect_segment_regions(regions, connected, start, end);
    }
}
}

void *grenade::native_vtable(void **actor_table)
{

    static std::array<void *, 173> table;
    std::copy_n(actor_table, 165, table.begin());
    table[0] = reinterpret_cast<void *>(&grenade_delete);
    table[0xF0 / 4] = reinterpret_cast<void *>(&grenade_query);
    table[0xF4 / 4] = reinterpret_cast<void *>(&grenade_guided);
    table[0x1A4 / 4] = reinterpret_cast<void *>(&grenade_advance);
    table[0x1AC / 4] = reinterpret_cast<void *>(&grenade_render);
    table[0x294 / 4] = reinterpret_cast<void *>(&grenade_hit);
    table[0x298 / 4] = reinterpret_cast<void *>(&grenade_hit_entity);
    table[0x29C / 4] = reinterpret_cast<void *>(&grenade_clear);
    table[0x2A0 / 4] = reinterpret_cast<void *>(&grenade_detonate);
    table[0x2A4 / 4] = reinterpret_cast<void *>(&grenade_intercept);
    table[0x2A8 / 4] = reinterpret_cast<void *>(&grenade_redirect_position);
    table[0x2AC / 4] = reinterpret_cast<void *>(&grenade_redirect_handle);
    table[0x2B0 / 4] = reinterpret_cast<void *>(&grenade_redirect_direction);
    return table.data();
}

grenade::grenade(const string_hash &id, unsigned flags) : actor(id, flags)
{
#if STANDALONE_SYSTEM
    construct_v_table_lookup();
    m_vtbl = ent_v_table_lookup[7];
#else
    m_vtbl = 0x889C08;
#endif
    inactive = armed = detonated = stuck = cleared_owner = first_frame = field_CE = redirected = false;
    weapon = {};
    flight_sound = {};
    armed_sound = {};
    visual = {};
    if (!g_generating_vtables) {
        next = previous = nullptr;
        fuse = arm_delay = beam_timer = stuck_time = 0.0f;
        render_scale = 1.0f;
        beam_entity = nullptr;
        beam_length = -1.0f;
        inactive = true;
        field_CE = true;
        script_thread = nullptr;
        script_thread_id = 0;
        if (!m_physical_interface)
            create_physical_ifc();
        change_list_status();
    }
}

grenade::~grenade()
{
    if (!g_generating_vtables) {
        thrown_item::remove_live_grenade(this);
        if (has_sound_and_pfx_ifc()) {
            stop_sound(flight_sound);
            stop_sound(armed_sound);
        }
        if (beam_entity)
            g_world_ptr->ent_mgr.destroy_entity(beam_entity);
        beam_entity = nullptr;
        if (auto *projectile = visual.get_volatile_ptr())
            g_world_ptr->ent_mgr.destroy_entity(projectile);
        visual = {};
        remove_from_list();
    }
}

void grenade::remove_from_list()
{
    if (previous)
        previous->next = next;
    else if (inactive_grenades() == this)
        inactive_grenades() = next;
    else if (active_grenades() == this)
        active_grenades() = next;
    if (next)
        next->previous = previous;
    next = previous = nullptr;
}

void grenade::change_list_status()
{
    remove_from_list();
    auto &head = (detonated || inactive) ? inactive_grenades() : active_grenades();
    next = head;
    head = this;
    if (next)
        next->previous = this;
}

void grenade::clear(bool)
{
    thrown_item::remove_live_grenade(this);
    set_visible(false, false);
    set_active(false);
    stuck_time = 0.0f;
    if (beam_entity) {
        beam_entity->set_active(false);
        beam_entity->set_visible(false, false);
    }
    if (my_sound_and_pfx_interface) {
        stop_sound(flight_sound);
        stop_sound(armed_sound);
    }
    auto *projectile = visual.get_volatile_ptr();
    if (projectile) {
        projectile->set_active(false);
        projectile->set_visible(false, false);
        drain_trail(projectile);
    }
    drain_trail(this);
    fuse = arm_delay = beam_timer = 0.0f;
    cleared_owner = armed = false;
    beam_length = -1.0f;
    clear_parent(true);
    inactive = true;
    first_frame = field_CE = true;
    redirected = false;
    if (script_thread) {
        auto *instance = script::get_gsoi();
        auto *thread = static_cast<vm_thread *>(script_thread);
        if (instance && instance->contains_thread(thread, script_thread_id))
            instance->kill_thread(thread->get_executable(), nullptr);
        script_thread = nullptr;
        script_thread_id = 0;
    }
    auto *physical = physical_ifc();
    physical->field_C &= ~0x80000080u;
    physical->cancel_all_velocity();
    physical->enable(false);
    physical->suspend(true);
    if (projectile)
        event_manager::raise_event(event::FADED_OUT, projectile->get_my_vhandle());
    change_list_status();
}

void grenade::frame_advance_all_grenades(Float elapsed)
{
    for (auto *current = active_grenades(); current;) {
        auto *next = current->next;
        const float scale = current->field_58 ? static_cast<float>(current->field_58->sub_4ADE50())
            : g_world_ptr->time_manager.field_0;
        invoke<void>(current, 0x1A4, Float{scale * elapsed.value});
        current = next;
    }
}

void grenade::sub_4D6B10(int mesh)
{
    field_90.set_mesh(reinterpret_cast<nglMesh *>(mesh));
    if (_get_mesh())
        field_4 |= 0x100;
    else
        field_4 &= ~0x100u;
}

bool grenade::check_if_hit(entity *target, const vector3d &start, const vector3d &end, vector3d &hit, int *kind)
{
    auto *weapon = weapon_owner(this);
    if (!weapon || !target)
        return false;
    if (stuck) {
        float radius = weapon->field_314;
        if (!(weapon->field_10C & 0x800000)) {
            radius = target->get_visual_radius();
            if (radius <= 0.0f)
                radius = 1.0f;
        }
        hit = get_abs_position();
        if ((get_abs_position() - target->get_abs_position()).length2() > radius * radius)
            return false;
        if (kind)
            *kind = 0;
        return true;
    }
    if (target->is_an_actor()) {
        vector3d normal;
        if (invoke<bool>(target, 0x250, &start, &end, &hit, &normal, 1.0f, true)) {
            if (kind)
                *kind = 1;
            return true;
        }
    }
    if (weapon->field_10C & 0x800000) {
        hit = get_abs_position();
        if ((hit - target->get_abs_position()).length2() <= weapon->field_314 * weapon->field_314) {
            if (kind)
                *kind = 0;
            return true;
        }
    }
    return false;
}

entity *grenade::check_hit(const vector3d &start, const vector3d &end, vector3d &hit, int *kind)
{
    auto *weapon = weapon_owner(this);
    int closest_kind = *kind;
    entity *closest = nullptr;
    if (weapon) {
        if (weapon->field_10C & 0x10000000) {
            for (int player = 0; player < g_world_ptr->num_players; ++player) {
                auto *candidate = g_world_ptr->get_hero_ptr(player);
                if (candidate && (candidate != weapon->field_108 || cleared_owner) &&
                    invoke<bool>(this, 0x298, candidate, &start, &end, &hit, kind))
                    return candidate;
            }
        } else {
            float radius = (weapon->field_10C & 0x800000) ? weapon->field_314 + 3.0f : 10.0f;
            if (radius < weapon->field_11C)
                radius = weapon->field_11C + 3.0f;
            if (damage_interface::find_damageable(get_abs_position(), radius, 3, true) > 0) {
                float distance_squared = radius * radius;
                for (auto *damage : *damage_interface::found_damageable) {
                    auto *candidate = damage->field_4;
                    if (!candidate || (candidate == weapon->field_108 && !cleared_owner) ||
                        !weapon->can_damage(candidate, redirected))
                        continue;
                    closest_kind = *kind;
                    if (invoke<bool>(this, 0x298, candidate, &start, &end, &hit, &closest_kind)) {
                        const float distance = (hit - start).length2();
                        if (distance < distance_squared) {
                            distance_squared = distance;
                            closest = candidate;
                        }
                    }
                }
            }
        }
    }
    *kind = closest_kind;
    return closest;
}

void grenade::intercept()
{
    if (auto *weapon = weapon_owner(this)) {
        auto *projectile = visual.get_volatile_ptr();
        const auto pose = get_abs_po();
        weapon->field_240.spawn(false, get_abs_position(), pose.get_z_facing(), weapon,
            projectile ? projectile : this, nullptr, vector3d{}, true, true);
    }
    invoke<void>(this, 0x29C, true);
    event_manager::raise_event(event::BEING_WEBBED, get_my_vhandle());
}

void grenade::redirect(const vector3d &position, entity_base *source)
{
    auto direction = position - get_abs_position();
    direction.normalize();
    invoke<void>(this, 0x2B0, &direction, source);
}

void grenade::reacquire_target(float radius, float cosine, float delay, entity_base *exclude)
{
    auto *weapon = weapon_owner(this);
    if (!(weapon->field_10C & 0x4000))
        return;
    auto *guidance = physical_ifc()->field_E8;
    if ((guidance->field_18 & 1) && weapon->field_338) {
        guidance->reacquire_target(radius, cosine, delay, exclude);
    } else {
        if (guidance->field_18 & 1)
            guidance->field_18 = (guidance->field_18 & ~3) | 2;
        const auto position = get_abs_position();
        const auto destination = position + get_abs_po().get_z_facing() * 100.0f;
        vector3d hit, normal;
        guidance->field_24 = find_intersection(position, destination, *local_collision::entfilter_blocks_beams,
            *local_collision::obbfilter_lineseg_test, &hit, &normal, nullptr, nullptr, nullptr, false)
            ? hit - normal * 10.0f : destination;
    }
}

void grenade::redirect(entity_base_vhandle target, entity_base *source)
{
    if (redirected)
        return;
    auto *weapon = weapon_owner(this);
    redirected = true;
    auto *entity = target.get_volatile_ptr();
    const auto old_velocity = physical_ifc()->get_velocity();
    auto direction = entity ? entity->get_abs_position() - get_abs_position() : old_velocity * -1.0f;
    direction.normalize();
    auto velocity = direction * old_velocity.length();
    physical_ifc()->set_velocity(velocity, false);
    auto up = get_abs_po().get_y_facing();
    auto normalized = velocity;
    normalized.normalize();
    if (std::fabs(normalized.x*up.x + normalized.y*up.y + normalized.z*up.z) > 0.99f || velocity.length() < 0.0001f)
        velocity = get_abs_po().get_z_facing() * -1.0f;
    po pose;
    pose.set_po(velocity, up, get_abs_position());
    set_abs_po(pose);
    if (weapon && (weapon->field_10C & 0x4000) && ((weapon->field_10C & 0x10000) || weapon->field_10C < 0)) {
        if (auto *guidance = physical_ifc()->field_E8) {
            if (weapon->field_10C & 0x10000) {
                if (entity)
                    guidance->set_target(vhandle_type<::entity>{entity->get_my_vhandle()});
                else
                    reacquire_target(35.0f, static_cast<float>(std::cos(0.5235987901687622)), 0.5f, source);
            } else {
                const auto position = get_abs_position();
                auto destination = position + direction * 1000.0f;
                vector3d hit, normal;
                if (find_intersection(position, destination, *local_collision::entfilter_blocks_beams,
                        *local_collision::obbfilter_lineseg_test, &hit, &normal, nullptr, nullptr, nullptr, false))
                    destination = hit - normal;
                guidance->field_24 = destination;
            }
        }
    }
}

void grenade::redirect_dir(const vector3d &input, entity_base *source)
{
    if (redirected)
        return;
    auto *weapon = weapon_owner(this);
    redirected = true;
    auto direction = input;
    direction.normalize();
    auto velocity = direction * physical_ifc()->get_velocity().length();
    physical_ifc()->set_velocity(velocity, false);
    const auto up = get_abs_po().get_y_facing();
    auto normalized = velocity;
    normalized.normalize();
    if (std::fabs(normalized.x*up.x + normalized.y*up.y + normalized.z*up.z) > 0.99f)
        velocity = get_abs_po().get_z_facing() * -1.0f;
    po pose;
    pose.set_po(velocity, up, get_abs_position());
    set_abs_po(pose);
    if (weapon && (weapon->field_10C & 0x4000) && ((weapon->field_10C & 0x10000) || weapon->field_10C < 0)) {
        if (auto *guidance = physical_ifc()->field_E8) {
            if (weapon->field_10C & 0x10000) {
                reacquire_target(35.0f, static_cast<float>(std::cos(0.5235987901687622)), 0.5f, source);
            } else {
                const auto position = get_abs_position();
                auto destination = position + direction * 1000.0f;
                vector3d hit, normal;
                if (find_intersection(position, destination, *local_collision::entfilter_blocks_beams,
                        *local_collision::obbfilter_lineseg_test, &hit, &normal, nullptr, nullptr, nullptr, false))
                    destination = hit - normal;
                guidance->field_24 = destination;
            }
        }
    }
}

void grenade::detonate(int reason, entity *hit)
{
    if (detonated)
        return;
    auto *owner = weapon_owner(this);
    detonated = true;
    const auto position = get_abs_position();
    event_manager::raise_event(event::EXPLODE, get_my_vhandle());
    if (owner) {
        if (owner->last_grenade == this)
            owner->last_grenade = nullptr;
        owner->detonate_position = position;
        owner->field_2B0 = this;
        event_manager::raise_event(event::DETONATE, owner->get_my_vhandle());
        owner->field_140.spawn(false, get_abs_position(), get_abs_po().get_z_facing(),
            owner, hit, nullptr, vector3d{}, true, true);
        if (auto *effect = owner->field_140.field_28; effect && effect->get_flavor() == 10)
            invoke<void>(effect, 0x21C, 2, 1.0f);
        if (owner->field_114) {
            std::vector<entity *> damaged;
            std::vector<grenade *> chain;
            if (hit && invoke<bool>(hit, 0x114) && owner->can_damage(hit, redirected))
                damaged.push_back(hit);
            if (owner->field_11C > 0.0f &&
                entity::find_entities(0x884 | ((owner->field_10C & 0x800) ? 0x40 : 0),
                                      this, owner->field_11C) > 0) {
                for (auto it = entity::found_entities->rbegin(); it != entity::found_entities->rend(); ++it) {
                    auto *candidate = *it;
                    if (candidate == this || candidate == hit ||
                        (invoke<bool>(candidate, 0x114) && !invoke<damage_interface *>(candidate, 0x118)->is_alive()))
                        continue;
                    if (invoke<bool>(candidate, 0xF0)) {
                        auto *grenade = static_cast<::grenade *>(candidate);
                        auto *other_owner = weapon_owner(grenade);
                        if (!grenade->detonated && (!other_owner || (other_owner->field_10C & 0x1000000)))
                            chain.push_back(grenade);
                    } else if (candidate->get_flavor() == 16) {
                        invoke<void>(candidate, 0x294, candidate);
                    } else if (invoke<bool>(candidate, 0x114) && owner->can_damage(candidate, redirected)) {
                        damaged.push_back(candidate);
                    }
                }
            }
            for (auto *target : damaged) {
                vector3d direction;
                if ((owner->field_10C & 0x4000) && !(owner->field_10C & 0x40000000)) {
                    direction = physical_ifc()->get_velocity();
                    if (direction.length() < 0.0001f)
                        direction = get_abs_po().get_z_facing();
                } else {
                    direction = target->get_abs_position() - position;
                }
                direction.normalize();
                const int amount = target == hit ? static_cast<int>(owner->field_114 * owner->field_118)
                                                 : owner->field_114;
                if (target->has_physical_ifc() && owner->field_128 > 0.0f)
                    target->physical_ifc()->set_gravity_delay_timer(0.5f);
                const bool explosion = (owner->field_10C & 0x40000000) != 0;
                const auto force = explosion && owner->field_128 > 0.0f
                    ? direction * owner->field_128 : vector3d{};
                const string_hash empty{0};
                const auto &attack = incoming.field_10 ? incoming.field_14.field_8 : empty;
                const auto &category = incoming.field_10 ? incoming.field_14.field_4 : empty;
                const auto &reaction = incoming.field_10 ? incoming.field_14.field_C : empty;
                invoke<damage_interface *>(target, 0x118)->apply_damage(owner, static_cast<float>(amount), explosion ? 6 : 2,
                    position, direction, 0, attack, category, reaction, false, force, 17, false);
                if (!explosion && target->has_physical_ifc() && owner->field_128 > 0.0f) {
                    auto *physical = target->physical_ifc();
                    auto impulse = direction * owner->field_128;
                    if (physical->field_184 && target->get_abs_po().get_y_facing().y > 0.8f)
                        impulse.y = 0.0f;
                    reinterpret_cast<void (__fastcall *)(physical_interface *, void *, const vector3d &,
                        physical_interface::force_type, const vector3d &, int)>(
                        get_vfunc(physical->m_vtbl, 0x2C))(physical, nullptr, impulse,
                            static_cast<physical_interface::force_type>(1), var<vector3d>(0x0091FF90), 0);
                    physical->set_gravity_delay_timer(1.0f);
                }
                if (target->is_an_actor()) {
                    if (auto *core = target->get_ai_core()) {
                        if (auto *damage = static_cast<ai::damage_inode *>(
                                core->get_info_node(ai::damage_inode::default_id, false))) {

                            if (!(damage->field_C->field_8 & 0x4000)) {
                                damage->field_1C = g_world_ptr->time_manager.field_C;
                                damage->field_20 = g_world_ptr->time_manager.field_8;
                            }
                        }
                    }
                }
            }
            for (auto *grenade : chain) {
                auto *other_owner = weapon_owner(grenade);
                if (!grenade->detonated && (!other_owner || (other_owner->field_10C & 0x1000000)))
                    invoke<void>(grenade, 0x2A0, 5, static_cast<entity *>(nullptr));
            }
        }
        auto direction = (owner->field_10C & 0x4000) || !has_physical_ifc()
            ? get_abs_po().get_z_facing() : physical_ifc()->get_velocity();
        if (has_physical_ifc() && !(owner->field_10C & 0x4000) && direction.length2() < 0.001f)
            direction = get_abs_po().get_z_facing();
        direction.normalize();
        if (owner->field_10C & 0x4000) {
            auto *guidance = physical_ifc()->field_E8;
            owner->spawn_mirvs(reason, position, direction, guidance->field_20,
                              guidance->field_24, direction, &incoming);
        } else {
            owner->spawn_mirvs(reason, position, direction, {}, vector3d{}, vector3d{}, &incoming);
        }
    }
    invoke<void>(this, 0x29C, false);
}

void grenade::frame_advance(Float elapsed)
{
    auto *owner = weapon_owner(this);
    if (detonated || inactive)
        return;
    const float dt = elapsed.value;
    auto *physical = physical_ifc();
    if (stuck) {
        if ((!owner || (owner->field_10C & 0x20000)) && flight_sound.get_sound_instance_ptr())
            stop_sound(flight_sound);
        if (stuck) {
            stuck_time += dt;
            if (m_parent) {
                vector3d hit, normal;
                if (is_in_limbo() || find_sphere_intersection(get_abs_position(), Float{0.0f},
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test,
                    &hit, &normal, nullptr, nullptr)) {
                    if (!owner || owner->field_11C <= 0.0f)
                        invoke<void>(this, 0x29C, true);
                    else
                        invoke<void>(this, 0x2A0, 7, static_cast<entity *>(nullptr));
                    return;
                }
            }
        }
    }
    if (!cleared_owner && owner) {
        float radius;
        if (owner->field_10C & 0x800000) {
            radius = owner->field_314 + 1.0f;
        } else {
            radius = invoke<float>(owner->field_108, 0x28);
            if (radius <= 0.0f)
                radius = 1.0f;
        }
        cleared_owner = (owner->field_108->get_abs_position() - get_abs_position()).length2() > radius * radius;
    }
    if (arm_delay > 0.0f)
        arm_delay = std::max(0.0f, arm_delay - dt);
    if (owner && (owner->field_30C > 0.0f || owner->field_304 >= 0.0f)) {
        auto position = get_abs_position();
        vector3d normal{0.0f, 1.0f, 0.0f};
        const float elevation = g_world_ptr->the_terrain->get_elevation(position, normal, this, nullptr, nullptr, -1.0f);
        if (elevation > -10000.0f) {
            if (owner->field_308 > -10.0f) {
                position.y = owner->field_308 + elevation;
                set_abs_position(position);
            } else {
                const float height = position.y - elevation;
                if (owner->field_304 >= 0.0f && height <= owner->field_304 && get_velocity().y < 0.0f) {
                    invoke<void>(this, 0x2A0, 6, static_cast<entity *>(nullptr));
                } else if (owner->field_30C > 0.0f && height < elevation + owner->field_30C) {
                    physical->set_velocity({}, false);
                    position.y = elevation + owner->field_30C;
                    set_abs_position(position);
                }
            }
        }
    }
    if (!armed && arm_delay <= 0.0f && (!owner || !owner->is_trip_mine() || stuck)) {
        armed = true;
        if (has_sound_and_pfx_ifc())
            armed_sound = sound_and_pfx_ifc()->play_sound_grp(
                var<string_hash>(0x0095FFA0), 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
        if (owner) {
            owner->field_2B4 = this;
            auto *projectile = visual.get_volatile_ptr();
            owner->field_200.spawn(false, get_abs_position(), get_abs_po().get_z_facing(),
                owner, projectile ? projectile : this, nullptr, vector3d{}, true, true);
            if (owner->is_trip_mine()) {
                if (!beam_entity) {
                    auto *beam = g_world_ptr->ent_mgr.create_and_add_beam(nullptr, make_unique_entity_id(), 0x2000);
                    beam_entity = beam;
                    beam->field_94 |= 0xC8;
                }
                beam_entity->set_visible(true, false);
                beam_entity->set_active(true);
                beam_length = -1.0f;
                beam_timer = 0.0f;
            }
            event_manager::raise_event(event::ARMED, owner->get_my_vhandle());
        }
    }
    if (armed) {
        if (owner && (owner->field_10C & 0x1000) && fuse > 0.0f)
            fuse = std::max(0.0f, fuse - dt);
        else
            fuse = 0.0f;
        if (beam_timer > 0.0f)
            beam_timer = std::max(0.0f, beam_timer - dt);
    }
    if (owner) {
        const uint32_t flags = owner->field_10C;
        const bool sticky = (flags & 0x80000) != 0;
        const bool alive = owner->is_trip_mine() ||
            (((owner->field_330 >= 0.0f && sticky && stuck) || !(flags & 0x1000) || fuse > 0.0f) &&
             (owner->field_330 < 0.0f || !sticky || !stuck || stuck_time < owner->field_330));
        if (alive) {
            const vector3d start = first_frame && owner->field_108
                ? owner->field_108->get_abs_position() : previous_position;
            auto end = start;
            if (stuck) {
                if (owner->is_trip_mine() && armed) {
                    const float old_length = beam_length;
                    if (beam_timer <= 0.0f) {
                        const auto position = get_abs_position();
                        const auto destination = position + get_abs_po().get_z_facing() * 100.0f;
                        vector3d hit, normal;
                        entity *occluder = nullptr;
                        find_intersection(position, destination, *local_collision::entfilter_blocks_beams,
                            *local_collision::obbfilter_lineseg_test, &hit, &normal, nullptr, &occluder, nullptr, false);
                        beam_length = (position - hit).length();
                        static_cast<beam *>(beam_entity)->set_point_to_point(position, hit);
                        beam_entity->compute_sector(g_world_ptr->the_terrain, false, nullptr);
                        beam_timer = m_parent ? 0.0f : 0.25f;
                    }
                    if ((old_length <= 0.0f || std::fabs(beam_length - old_length) < 0.1f) && beam_length > 0.1f) {
                        const auto position = get_abs_position();
                        const auto destination = position + get_abs_po().get_z_facing() * beam_length;
                        region_array regions{};
                        collect_segment_regions(regions, get_primary_region(), position, destination);
                        for (int i = 0; i < regions.count; ++i) {
                            auto &entities = *static_cast<_std::list<entity *> *>(regions.m_data[i]->region_entities);
                            for (auto *candidate : entities) {
                                if (detonated)
                                    break;
                                if (!candidate || !candidate->is_an_actor() || (candidate->field_4 & 0x20000) ||
                                    !owner->can_damage(candidate, redirected))
                                    continue;
                                vector3d hit, normal;
                                if (invoke<bool>(candidate, 0x250, &position, &destination, &hit, &normal, 1.0f, false))
                                    invoke<void>(this, 0x2A0, 3, static_cast<entity *>(nullptr));
                            }
                        }
                    } else {
                        invoke<void>(this, 0x2A0, 3, static_cast<entity *>(nullptr));
                    }
                }
            } else {
                end = get_abs_position();
                if (render_scale > 1.01f || render_scale < 0.99f) {
                    po scale = po_identity_matrix;
                    scale.set_scale({render_scale, render_scale, render_scale});
                    po pose;
                    po::compose(pose, get_abs_po(), scale);
                    set_abs_po(pose);
                }
                if (first_frame && (physical->field_C & (0x10000000 | 0x20000000)) && !(physical->field_C & 0x80)) {
                    vector3d hit, normal;
                    entity *contact = nullptr;
                    if (find_intersection(start, end, *local_collision::entfilter_blocks_beams,
                        *local_collision::obbfilter_lineseg_test, &hit, &normal, nullptr, &contact, nullptr, false)) {
                        normal.normalize();
                        physical->bounce(elapsed, hit, normal, contact);
                        end = get_abs_position();
                    }
                }
                first_frame = false;
                if ((physical->field_C & 0x80) && (cleared_owner ||
                    reinterpret_cast<entity_base *>(physical->field_C0) != owner->field_108)) {
                    if (!owner->is_trip_mine() && (flags & 0x2000) && !detonated && armed) {
                        int kind = 0;
                        entity *contact = reinterpret_cast<entity *>(physical->field_C0);
                        auto hit = physical->field_A8;
                        if (!contact || !owner->can_damage(contact, redirected))
                            contact = invoke<entity *>(this, 0x294, &start, &end, &hit, &kind);
                        if (contact) {
                            if (m_parent)
                                hit = m_parent->get_abs_po().inverse()->slow_xform(hit);
                            set_abs_position(hit);
                            invoke<void>(this, 0x2A0, kind, (flags & 0x800000) ? nullptr : contact);
                        }
                    }
                    if (!detonated) {
                        auto *contact = reinterpret_cast<entity *>(physical->field_C0);
                        const bool impact_only = owner->is_trip_mine() || !armed ||
                            ((flags & (0x400 | 0x80000)) &&
                             (!(flags & 0x2000) || !contact || !invoke<bool>(contact, 0x114))) ||
                            (contact && ((contact->field_4 & 0x20000) || !owner->can_damage(contact, redirected)));
                        if (impact_only) {
                            if ((flags & (0x80000 | 0x400)) && (physical->get_velocity().length2() >= 2.0f || stuck)) {
                                auto *projectile = visual.get_volatile_ptr();
                                owner->field_180.spawn(false, physical->field_A8, physical->field_B4, owner,
                                    projectile ? projectile : this, nullptr, vector3d{}, true, true);
                            }
                        } else {
                            invoke<void>(this, 0x2A0, 1, contact && invoke<bool>(contact, 0x114) ? contact : nullptr);
                        }
                    }
                }
            }
            if (!owner->is_trip_mine() && (flags & 0x2000) && !detonated && armed &&
                (!stuck || owner->field_11C > 0.0f)) {
                int kind = 0;
                vector3d hit;
                if (auto *contact = invoke<entity *>(this, 0x294, &start, &end, &hit, &kind)) {
                    if (m_parent)
                        hit = m_parent->get_abs_po().inverse()->slow_xform(hit);
                    set_abs_position(hit);
                    invoke<void>(this, 0x2A0, kind, (flags & 0x800000) ? nullptr : contact);
                }
            }
            if (std::fabs(owner->spin.z) > 0.0001f)
                physical->field_2C = get_abs_po().get_z_facing() * (360.0f * owner->spin.z);
            else if (std::fabs(owner->spin.y) > 0.0001f)
                physical->field_2C = get_abs_po().get_y_facing() * (360.0f * owner->spin.y);
            else if (std::fabs(owner->spin.x) > 0.0001f)
                physical->field_2C = get_abs_po().get_x_facing() * (360.0f * owner->spin.x);
            if (auto *projectile = visual.get_volatile_ptr()) {
                projectile->set_abs_po(get_abs_po());
                projectile->force_regions(this);
                projectile->update_proximity_maps();
            }
        } else if (owner->field_11C <= 0.0f || (sticky && stuck && !(flags & 0x200))) {
            invoke<void>(this, 0x29C, true);
        } else {
            invoke<void>(this, 0x2A0, 2, static_cast<entity *>(nullptr));
        }
    }
    previous_position = get_abs_position();
    stuck = (physical->field_C & 0x80000000) != 0;
}

VALIDATE_SIZE(grenade, 0x1A4);
VALIDATE_OFFSET(grenade, next, 0xC0);
VALIDATE_OFFSET(grenade, weapon, 0xD0);
VALIDATE_OFFSET(grenade, incoming, 0xF0);
VALIDATE_OFFSET(grenade, visual, 0x184);
