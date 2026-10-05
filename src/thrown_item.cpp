#include "thrown_item.h"

#include "common.h"
#include "entity_mash.h"
#include "grenade.h"
#include "guidance_sys.h"
#include "memory.h"
#include "oldmath_po.h"
#include "parse_generic_mash.h"
#include "physical_interface.h"
#include "vtbl.h"
#include "wds.h"
#include "resource_manager.h"
#include "sound_and_pfx_interface.h"
#include "script.h"
#include "advanced_entity_ptrs.h"
#include "chuck/vm/script_object.h"
#include "chuck/vm/vm_thread.h"
#include "base_ai_core.h"
#include "ai_std_combat_target.h"
#include "ai_team.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include "config.h"
#include <new>
#include "motion_effect_struct.h"

namespace {

constexpr float random_scale = 1.0f / 32767.0f;

template <typename R, typename... Args>
R invoke(entity_base *self, unsigned offset, Args... args)
{
    auto fn = reinterpret_cast<R (__fastcall *)(entity_base *, void *, Args...)>(
        get_vfunc(self->m_vtbl, offset));
    return fn(self, nullptr, args...);
}

void scatter_direction(vector3d &direction, float spread)
{
    if (spread <= 0.0f)
        return;
    po facing = po_identity_matrix;
    facing.set_facing(direction);
    po rotation;
    rotation.set_rot(facing.get_y_facing(), Float{(std::rand() * random_scale * 2.0f - 1.0f) * spread * 0.5f});
    direction = rotation.non_affine_slow_xform(direction);
    rotation.set_rot(facing.get_z_facing(), Float{6.283185307179586f * (std::rand() * random_scale)});
    direction = rotation.non_affine_slow_xform(direction);
    direction.normalize();
}
}

namespace {
void *__fastcall thrown_delete(thrown_item *self, void *, unsigned flags)
{
    self->~thrown_item();
    if (flags & 1)
        mem_dealloc(self, sizeof(thrown_item));
    return self;
}
void __fastcall thrown_release(thrown_item *self, void *) { self->release_mem(); }
bool __fastcall thrown_chunk(thrown_item *, void *, void *, void *) { return false; }
bool __fastcall thrown_query(thrown_item *, void *) { return true; }
void __fastcall thrown_unmash(thrown_item *self, void *, generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    self->un_mash(header, object, data);
}
void __fastcall thrown_advance(thrown_item *self, void *, Float elapsed) { self->frame_advance(elapsed); }
void __fastcall thrown_apply(thrown_item *self, void *, actor *owner) { self->internal_apply_effects(owner, nullptr); }

void __fastcall thrown_defaults(thrown_item *, void *) {}
void __fastcall thrown_owner(thrown_item *self, void *, actor *owner) { self->set_owner(owner); }
vector3d *__fastcall thrown_fire_position(thrown_item *self, void *, vhandle_type<entity> source,
                                         const vector3d &position, ai::combat_inode::incoming_move *move)
{
    return self->fire_at_target_internal(source, {}, position, move);
}
vector3d *__fastcall thrown_fire_entity(thrown_item *self, void *, vhandle_type<entity> source,
                                       vhandle_type<entity> target, ai::combat_inode::incoming_move *move)
{
    entity_base *reference = self->field_104.get_volatile_ptr();
    if (!reference)
        reference = self;
    const auto position = thrown_item::calc_target_pos(reference->get_abs_position(), target,
        self->launch_speed, thrown_item::calc_target_pos_delta(self->target_spread), self->prediction);
    return self->fire_at_target_internal(source, target, position, move);
}
vector3d *__fastcall thrown_detonation_position(thrown_item *self, void *, vector3d *out)
{
    *out = self->detonate_position;
    return out;
}
void __fastcall thrown_spawn(thrown_item *self, void *, vector3d direction, float speed,
                             bool explicit_position, const vector3d &position,
                             ai::combat_inode::incoming_move *move)
{
    self->spawn_grenade(direction, speed, explicit_position, position, move);
}
}

void *thrown_item::native_vtable(void **handheld_table)
{
    static std::array<void *, 192> table;
    std::copy_n(handheld_table, 190, table.begin());
    table[0] = reinterpret_cast<void *>(&thrown_delete);
    table[0x10 / 4] = reinterpret_cast<void *>(&thrown_release);
    table[0x20 / 4] = reinterpret_cast<void *>(&thrown_chunk);
    table[0xE4 / 4] = reinterpret_cast<void *>(&thrown_query);
    table[0x164 / 4] = reinterpret_cast<void *>(&thrown_unmash);
    table[0x1A4 / 4] = reinterpret_cast<void *>(&thrown_advance);
    table[0x2B8 / 4] = reinterpret_cast<void *>(&thrown_apply);
    table[0x2D4 / 4] = reinterpret_cast<void *>(&thrown_defaults);
    table[0x2E0 / 4] = reinterpret_cast<void *>(&thrown_owner);
    table[0x2F0 / 4] = reinterpret_cast<void *>(&thrown_fire_position);
    table[0x2F4 / 4] = reinterpret_cast<void *>(&thrown_fire_entity);
    table[0x2F8 / 4] = reinterpret_cast<void *>(&thrown_detonation_position);
    table[0x2FC / 4] = reinterpret_cast<void *>(&thrown_spawn);

    return table.data();
}



thrown_item::~thrown_item() = default;

bool thrown_item::is_trip_mine() const
{
    return (field_10C & 0x400000) && (field_10C & 0x80000) && (field_10C & 0x200000);
}

grenade *grenade_cache::push_new_grenade(thrown_item *owner)
{
    auto *value = ::new (mem_alloc(sizeof(grenade))) grenade(make_unique_entity_id(), 24672);
    if (owner->field_10C & 0x8000) {
        value->sub_4D6B10(reinterpret_cast<int>(owner->get_mesh()));
    } else if (owner->projectile_template.get_volatile_ptr() && owner->projectile_resource.m_hash.source_hash_code) {
        resource_manager::push_resource_context(owner->m_resource_context);
        auto *visual = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
            owner->projectile_resource.m_hash, make_unique_entity_id(), po_identity_matrix,
            mString{""}, 0x2000, nullptr);
        resource_manager::pop_resource_context();
        if (visual->has_physical_ifc())
            visual->physical_ifc()->enable(false);
        visual->set_collisions_active(false, true);
        value->visual.field_0 = visual->my_handle.field_0;
    }
    auto *physical = value->physical_ifc();
    auto *source = owner->has_physical_ifc() ? owner->physical_ifc() : nullptr;
    const float mass = source ? source->field_10 : 0.0f;
    const float secondary_mass = source ? bit_cast<float>(source->field_14) : 0.0f;
    physical->field_10 = mass > 0.0f ? mass : 1.0f;
    physical->field_14 = bit_cast<int>(secondary_mass > 0.0f ? secondary_mass : 1.0f);
    value->render_scale = *reinterpret_cast<float *>(owner->field_100 + 0x24);
    value->set_collisions_active(false, true);
    if (owner->has_sound_and_pfx_ifc()) {
        if (!value->has_sound_and_pfx_ifc())
            value->create_sound_and_pfx_ifc();
        value->sound_and_pfx_ifc()->copy(*owner->sound_and_pfx_ifc());
    }
    value->weapon.field_0 = owner->my_handle.field_0;
    g_world_ptr->ent_mgr.add_dynamic_instanced_entity(value);
    entries->push_back(vhandle_type<entity>{value->get_my_vhandle()});
    return value;
}

vector3d thrown_item::calc_target_pos_delta(float radius)
{
    vector3d delta{0.0f, 0.0f, 0.0f};
    if (radius > 0.0f) {
        delta.x = std::rand() * random_scale * 2.0f - 1.0f;
        delta.y = std::rand() * random_scale * 2.0f - 1.0f;
        delta.z = std::rand() * random_scale * 2.0f - 1.0f;
        float length = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
        if (length > 0.01f)
            delta *= std::rand() * random_scale * radius / length;
    }
    return delta;
}

vector3d thrown_item::calc_target_pos(const vector3d &origin, vhandle_type<entity> target,
                                    float speed, const vector3d &offset, float prediction)
{
    vector3d result = offset;
    if (auto *value = target.get_volatile_ptr()) {
        const vector3d position = value->get_abs_position();
        vector3d predicted = position;
        if (prediction > 0.0f) {
            const vector3d difference = position - origin;
            const float time = std::min(0.75f, std::sqrt(difference.length2()) / speed);
            vector3d velocity;
            invoke<vector3d *>(value, 0x30, &velocity);
            predicted += velocity * time;
            if (value->has_physical_ifc()) {
                auto *physical = value->physical_ifc();
                predicted.y -= 9.81f * physical->m_gravity_multiplier * time * time * 0.5f;
                float floor = std::min(position.y, physical->field_F8);
                if (floor <= -10000.0f && floor >= -10000.0f)
                    floor = position.y - 2.0f;
                predicted.y = physical->get_floor_offset() + std::max(predicted.y, floor);
            }
            predicted = position + (predicted - position) * prediction;
        }
        result += predicted;
    }
    return result;
}

vector3d thrown_item::invert_launch_vec(const vector3d &direction)
{
    entity_base *reference = field_104.get_volatile_ptr();
    if (!reference)
        reference = field_108;
    return reference ? reference->get_abs_po().non_affine_inverse_xform(direction) : direction;
}

void thrown_item::frame_advance(Float elapsed)
{
    field_140.frame_advance(elapsed);
    field_180.frame_advance(elapsed);
    field_1C0.frame_advance(elapsed);
    field_200.frame_advance(elapsed);
    field_240.frame_advance(elapsed);
    handheld_item::frame_advance(elapsed);
}

void thrown_item::set_owner(actor *owner)
{
    handheld_item::set_owner(owner);
    if (mirv)
        invoke<void>(mirv, 0x2E0, owner);
}

void thrown_item::internal_apply_effects(actor *fallback_owner, ai::combat_inode::incoming_move *move)
{
    if (invoke<int>(this, 0x298)) {
        auto *owner = field_108 ? static_cast<actor *>(field_108) : fallback_owner;
        if (owner) {
            auto *visual = field_104.get_volatile_ptr();
            entity_base *reference = visual ? static_cast<entity_base *>(visual) : owner;
            vector3d direction = reference->get_abs_po().non_affine_slow_xform(launch_direction);
            direction.normalize();
            scatter_direction(direction, spread);
            const vector3d zero{0.0f, 0.0f, 0.0f};
            invoke<void>(this, 0x2FC, direction, launch_speed, false, &zero, move);
            item::apply_effects(owner);
            if (invoke<int>(this, 0x298) > 0)
                invoke<int>(this, 0x2A0);
        }
    }
    target_valid() = false;
}

vector3d *thrown_item::fire_at_target_internal(vhandle_type<entity> source, vhandle_type<entity> target,
                                             const vector3d &position, ai::combat_inode::incoming_move *move)
{
    const vector3d saved_direction = launch_direction;
    const float saved_speed = launch_speed;
    const vector3d zero{0.0f, 0.0f, 0.0f};
    if (launch_direction.x <= 0.0f && launch_direction.x >= 0.0f &&
        launch_direction.y <= 0.0f && launch_direction.y >= 0.0f &&
        launch_direction.z <= 0.0f && launch_direction.z >= 0.0f) {
        entity_base *reference = field_104.get_volatile_ptr();
        if (!reference)
            reference = this;
        reference->dirty_family(false);
        const vector3d origin = reference->get_abs_position();
        vector3d direction;
        if ((field_10C & 0x10000) || field_10C < 0 || gravity_multiplier <= 0.0f)
            direction = position - origin;
        else
            direction = physical_interface::calculate_force_vector_2(&origin, &position,
                Float{saved_speed}, Float{gravity_multiplier});
        direction.normalize();
        launch_direction = invert_launch_vec(direction);
    } else {
        vector3d direction = launch_direction;
        if (auto *owner = source.get_volatile_ptr())
            direction = owner->get_abs_po().non_affine_slow_xform(direction);
        launch_direction = invert_launch_vec(direction);
    }
    launch_direction.normalize();
    target_ent() = {};
    target_hit() = position;
    target_norm() = zero;
    target_valid() = true;
    internal_apply_effects(static_cast<actor *>(field_108), move);
    if (auto *owner = source.get_volatile_ptr())
        owner->get_abs_po();
    if ((field_10C & 0x10000) || field_10C < 0) {
        if (auto *guidance = last_grenade->physical_ifc()->field_E8) {
            guidance->set_target(target);
            guidance->field_24 = position;
        }
    }
    launch_direction = saved_direction;
    launch_direction.normalize();
    launch_speed = saved_speed;
    return &launch_direction;
}

void thrown_item::spawn_mirvs(int reason, const vector3d &position, const vector3d &normal,
                             vhandle_type<entity> target, const vector3d &hit, const vector3d &hit_normal,
                             ai::combat_inode::incoming_move *move)
{
    if (!mirv || mirv_count <= 0 || !(reason == 0 || (reason > 1 && reason <= 4)))
        return;
    po rotation = po_identity_matrix;
    vector3d lateral{1.0f, 0.0f, 0.0f};
    vector3d axis{0.0f, 1.0f, 0.0f};
    if (field_10C & 0x100) {
        rotation.set_facing(normal);
        lateral = rotation.get_x_facing();
        axis = rotation.get_z_facing();
    }
    vector3d direction = lateral * mirv_lateral + axis * mirv_vertical;
    direction.normalize();
    rotation.set_rot(axis, Float{6.283185307179586f / mirv_count});
    for (int i = 0; i < mirv_count; ++i) {
        target_ent() = target;
        target_hit() = hit;
        target_norm() = hit_normal;
        target_valid() = true;
        scatter_direction(direction, mirv->spread);
        invoke<void>(mirv, 0x2FC, direction, mirv->launch_speed, true, &position, move);
        direction = rotation.non_affine_slow_xform(direction);
    }
}

void grenade_cache::release_mem()
{
    if (entries) {
        while (!entries->empty()) {
            auto *value = entries->front().get_volatile_ptr();
            entries->erase(entries->begin());
            if (value)
                g_world_ptr->ent_mgr.destroy_entity(value);
        }
        delete entries;
        entries = nullptr;
    }
}

void grenade_cache::un_mash(thrown_item *owner, generic_mash_data_ptrs *data)
{
    if (!references)
        entries = new _std::vector<vhandle_type<entity>>;
    initial_count = *data->get_from_shared<int>();
    if (!references)
        for (int i = 0; i < initial_count; ++i)
            push_new_grenade(owner);
    ++references;
}

grenade *grenade_cache::get_grenade(thrown_item *owner)
{
    for (auto it = entries->begin(); it != entries->end();) {
        auto *value = static_cast<grenade *>(it->get_volatile_ptr());
        if (!value)
            it = entries->erase(it);
        else {
            if (value->inactive)
                return value;
            ++it;
        }
    }
    return push_new_grenade(owner);
}

void thrown_item::release_mem()
{
    if (--cache->references == 0)
        cache->release_mem();
    cache = nullptr;
    if (auto *value = projectile_template.get_volatile_ptr())
        g_world_ptr->ent_mgr.destroy_entity(value);
    projectile_template = {};
    while (!live_grenades->empty())
        invoke<void>(live_grenades->front(), 0x29C, false);
    delete live_grenades;
    live_grenades = nullptr;
    field_140.release_mem();
    field_180.release_mem();
    field_1C0.release_mem();
    field_200.release_mem();
    field_240.release_mem();
    field_340.~mString();
    handheld_item::release_mem();
}

void thrown_item::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    new (&field_340) mString;
    live_grenades = new _std::vector<grenade *>;
    handheld_item::un_mash(header, object, data);
    field_140.un_mash(header, &field_140, data);
    field_180.un_mash(header, &field_180, data);
    field_1C0.un_mash(header, &field_1C0, data);
    field_200.un_mash(header, &field_200, data);
    field_240.un_mash(header, &field_240, data);
    data->rebase_shared(4);
    const int string_size = *data->get_from_shared<int>();
    field_340 = data->get_from_shared<char>(string_size);
    resource_key key = *data->get_from_shared<resource_key>();
    projectile_template = {};
    if (key.m_hash.source_hash_code) {
        auto *value = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
            key.m_hash, make_unique_entity_id(), po_identity_matrix, mString{""}, 1, nullptr);
        if (value) {
            field_10C &= ~0x8000;
            value->set_active(false);
            value->set_visible(false, false);
            if (value->has_physical_ifc())
                value->physical_ifc()->enable(false);
            projectile_template.field_0 = value->my_handle.field_0;
        }
    }
    key = *data->get_from_shared<resource_key>();
    mirv = nullptr;
    if (key.m_hash.source_hash_code) {
        auto *value = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
            key.m_hash, make_unique_entity_id(), po_identity_matrix, mString{""}, 129, nullptr);
        if (invoke<bool>(value, 0xE4)) {
            mirv = static_cast<thrown_item *>(value);
            invoke<bool>(this, 0x288, value->my_handle.field_0, true);
        }
    }
    data->rebase_shared(4);
    cache = data->get_from_shared<grenade_cache>();
    cache->un_mash(this, data);
}

void thrown_item::remove_live_grenade(grenade *value)
{
    if (auto *owner = static_cast<thrown_item *>(value->weapon.get_volatile_ptr())) {
        if (owner->live_grenades) {
            auto &entries = *owner->live_grenades;
            auto it = std::find(entries.begin(), entries.end(), value);
            if (it != entries.end())
                entries.erase(it);
        }
    }
    auto &entries = all_grenades();
    auto it = std::find(entries.begin(), entries.end(), value);
    if (it != entries.end())
        entries.erase(it);
}

void thrown_item::clear_all()
{
    while (!all_grenades().empty())
        invoke<void>(all_grenades().front(), 0x29C, false);
}

grenade *thrown_item::get_new_grenade()
{
    auto *value = cache->get_grenade(this);
    auto *physical = value->physical_ifc();
    physical->set_gravity(!(field_10C & 0x4000));
    value->weapon.field_0 = my_handle.field_0;
    value->inactive = false;
    value->detonated = false;
    value->redirected = false;
    value->first_frame = true;
    value->stuck = false;
    value->incoming.field_10 = 0;
    physical->cancel_all_velocity();
    physical->enable(true);
    physical->suspend(false);
    physical->set_velocity(vector3d{0.0f, 0.0f, 0.0f}, false);
    physical->field_2C = vector3d{0.0f, 0.0f, 0.0f};
    physical->field_44 = vector3d{0.0f, 0.0f, 0.0f};
    physical->field_50 = vector3d{0.0f, 0.0f, 0.0f};
    physical->field_5C = vector3d{0.0f, 0.0f, 0.0f};
    value->stuck_time = 0.0f;
    value->change_list_status();
    return value;
}

bool thrown_item::can_damage(entity_base *target, bool redirected) const
{

    if (field_10C & 0x40000) {
        auto *target_core = target->get_ai_core();
        if (!target_core)
            return false;
        const auto owner_team = ai::team::manager::get_team_enum_by_hash(
            field_108->get_ai_core()->get_param_block()->get_pb_hash(ai::combat_target_inode::team_hash()));
        const auto target_team = ai::team::manager::get_team_enum_by_hash(
            target_core->get_param_block()->get_pb_hash(ai::combat_target_inode::team_hash()));
        if (static_cast<int>(target_team) != 15 && ai::team::manager::is_friend(owner_team, target_team))
            return false;
    }
    if ((target->field_4 & 0x20000) || ((field_10C & 0x2000000) &&
        (!redirected || target != field_108) && target == field_108))
        return false;
    if ((!redirected || target != field_108) &&
        ((field_10C & 0x20000000) || invoke<bool>(target, 0x4C)) &&
        (!invoke<bool>(target, 0x4C) || (field_10C & 0x4000000))) {
        if (!(field_10C & 0x8000000) || !(target->field_4 & 0x1000) ||
            ((field_10C & 0x4000000) && invoke<bool>(target, 0x4C)))
            return false;
    }
    return true;
}

void thrown_item::spawn_grenade(vector3d direction, float speed, bool explicit_position,
    const vector3d &position, ai::combat_inode::incoming_move *move)
{
    auto *visual = field_104.get_volatile_ptr();
    auto *value = get_new_grenade();
    last_grenade = value;
    if (explicit_position)
        value->set_abs_position(position);
    else if (visual)
        value->set_abs_po(visual->get_abs_po());
    else
        value->set_abs_position(field_108->get_abs_position());
    value->previous_position = value->get_abs_position();
    auto *physical = value->physical_ifc();
    if (field_10C & 0x4000) {
        if (!physical->field_E8)
            physical->create_guidance_sys(1);
        auto *guidance = physical->field_E8;
        if (field_10C & 0x10000) {
            guidance->field_18 = (guidance->field_18 & ~3) |
                (std::rand() * random_scale > guidance_probability ? 0 : 1);
        } else if (field_10C < 0) {
            guidance->field_18 = (guidance->field_18 & ~3) |
                (std::rand() * random_scale > guidance_probability ? 0 : 2);
        }
        guidance->set_target(target_ent());
        guidance->field_24 = target_hit();
        guidance->field_44 = prediction;
        guidance->field_30 = calc_target_pos_delta(target_spread);
        guidance->field_54 = field_2C8;
        guidance->field_A4 = this;
        guidance->field_64 = bit_cast<float>(field_2E0);
        guidance->field_68 = bit_cast<float>(field_2E4);
        guidance->field_6C = bit_cast<float>(field_2E8);
        guidance->field_70 = bit_cast<float>(field_2EC);
        guidance->field_74 = bit_cast<float>(field_2F0);
        guidance->field_60 = field_2DC;
        guidance->field_5C = field_2D8;
        guidance->field_58 = field_2D4;
        guidance->field_78 = field_300;
        po pose = value->get_abs_po();
        pose.set_facing(direction);
        *value->my_rel_po = pose;
        value->dirty_family(false);
        if (value->field_4 & (0x8000 | 4))
            value->dirty_model_po_family();
        value->po_changed();
    } else {
        const float z = (std::rand() * random_scale * 2.0f - 1.0f) * 3.141592653589793f;
        const float y = (std::rand() * random_scale * 2.0f - 1.0f) * 3.141592653589793f;
        const float x = (std::rand() * random_scale * 2.0f - 1.0f) * 3.141592653589793f;
        physical->field_2C = {x, y, z};
        physical->set_gravity(true);
        physical->m_gravity_multiplier = gravity_multiplier;
    }
    physical->field_C = (physical->field_C & ~0x20000000u) |
        ((field_10C & 0x80000) ? 0x20000000u : 0);
    physical->field_94 = field_310;
    physical->field_C = (physical->field_C & ~0x80000000u) | 0x10000000u;
    physical->field_90 = field_138;
    physical->field_8C = field_13C;
    if (move)
        value->incoming = *move;
    else
        value->incoming.field_10 = 0;
    const auto start_trail = [this](entity_base *source) {
        if (!field_31C)
            return;
        const auto *data = static_cast<const unsigned char *>(field_31C);
        if (!source->field_18)
            source->field_18 = new motion_effect_struct(source->my_handle, mString{""});
        source->field_18->activate_trail(source, *reinterpret_cast<const int *>(data + 48),
            *reinterpret_cast<const float *>(data + 44), color32{0xFFFF0000},
            *reinterpret_cast<const int *>(data + 36), *reinterpret_cast<const float *>(data + 56),
            *reinterpret_cast<const int *>(data + 40), data[25] != 0);
    };
    if (auto *projectile = value->visual.get_volatile_ptr()) {
        *projectile->my_rel_po = value->get_abs_po();
        projectile->dirty_family(false);
        if (projectile->field_4 & (0x8000 | 4))
            projectile->dirty_model_po_family();
        projectile->po_changed();
        projectile->set_active(true);
        projectile->set_visible(false, false);
        if (projectile->is_renderable())
            projectile->set_visible(true, false);
        if (projectile->field_4 & 4)
            invoke<void>(projectile, 0x2B4);
        projectile->field_4 |= 0x40;
        if (projectile->has_physical_ifc())
            projectile->physical_ifc()->enable(false);
        projectile->set_collisions_active(false, true);
        projectile->compute_sector(g_world_ptr->the_terrain, false, nullptr);
        start_trail(projectile);
        if (projectile->get_flavor() == 10)
            invoke<void>(projectile, 0x21C, 2, 1.0f);
    } else {
        start_trail(value);
    }
    value->set_collisions_active(false, true);
    value->field_4 |= 0x40;
    value->set_active(true);
    if (value->is_renderable())
        value->set_visible(true, false);
    value->fuse = (std::rand() * random_scale * 2.0f - 1.0f) * fuse_variation + fuse;
    value->arm_delay = arm_delay;
    if (!(field_10C & 0x4000) && value->arm_delay > 0.0f)
        value->cleared_owner = true;
    physical->enable(true);
    physical->set_velocity({}, false);
    physical->set_acceleration_factor({});
    physical->field_44 = {};
    physical->field_50 = {};
    if (physical->field_E8)
        physical->field_E8->launch(direction, speed);
    else {
        const auto force = direction * speed;
        reinterpret_cast<void (__fastcall *)(physical_interface *, void *, const vector3d &,
            physical_interface::force_type, const vector3d &, int)>(
            get_vfunc(physical->m_vtbl, 0x2C))(physical, nullptr, force,
                static_cast<physical_interface::force_type>(1), var<vector3d>(0x0091FF90), 0);
    }
    value->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    if (value->has_sound_and_pfx_ifc())
        value->flight_sound = value->sound_and_pfx_ifc()->play_sound_grp(
            var<string_hash>(0x0096018C), 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
    if (adv_ptrs && adv_ptrs->my_script && field_340 != mString{""}) {
        mString function_name = field_340 + mString{"(entity,entity)"};
        auto *instance = adv_ptrs->my_script;
        const int function = script::find_function(string_hash{function_name.c_str()}, instance->get_parent(), false);
        if (function >= 0) {
            if (auto *thread = script::new_thread(function, instance)) {
                auto *weapon = static_cast<thrown_item *>(value->weapon.get_volatile_ptr());
                script::push_arg(weapon ? static_cast<entity_base *>(weapon->field_104.get_volatile_ptr()) : value);
                auto *projectile = value->visual.get_volatile_ptr();
                script::push_arg(projectile ? projectile : value);
                script::exec_thread(false);
                value->script_thread = thread;
                value->script_thread_id = thread->field_1E4;
            }
        }
    }
    live_grenades->push_back(value);
    all_grenades().push_back(value);
    auto *projectile = value->visual.get_volatile_ptr();
    const auto pose = value->get_abs_po();
    field_1C0.spawn(false, value->get_abs_position(), pose.get_z_facing(), this,
        projectile ? projectile : value, visual, effect_offset, true, invoke<bool>(this, 0x2D8));
    target_valid() = false;
}

VALIDATE_SIZE(thrown_item, 0x350u);
