#include "gun.h"

#include "ai_team.h"
#include "base_ai_core.h"
#include "common.h"
#include "damage_interface.h"
#include "entity_mash.h"
#include "event.h"
#include "memory.h"
#include "ngl.h"
#include "oldmath_po.h"
#include "param_block.h"
#include "parse_generic_mash.h"
#include "us_pcuv_shader.h"
#include "vtbl.h"
#include "wds.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <new>

#include "collision_geometry.h"
#include "local_collision.h"
#include "terrain.h"
#include <cstdlib>
#include "damage_inode.h"
#include "physical_interface.h"
#include "script.h"
#include "sound_and_pfx_interface.h"
#include "terrain_types_manager.h"
#include "moved_entities.h"
#include <vector.hpp>
VALIDATE_SIZE(gun_hit, 0x28);
VALIDATE_SIZE(gun_beam, 0x34);
VALIDATE_SIZE(gun, 0x374);
VALIDATE_OFFSET(gun, projectile, 0x2F8);
VALIDATE_OFFSET(gun, laser, 0x32C);

namespace {
bool has_damage_interface(entity *value)
{
    return reinterpret_cast<bool(__fastcall *)(entity *, void *)>(get_vfunc(value->m_vtbl, 0x114))(value, nullptr);
}
damage_interface *get_damage_interface(entity *value)
{
    return reinterpret_cast<damage_interface *(__fastcall *)(entity *, void *)>(get_vfunc(value->m_vtbl, 0x118))(
        value, nullptr);
}
bool entity_is_alive(entity *value)
{
    return reinterpret_cast<bool(__fastcall *)(entity *, void *)>(get_vfunc(value->m_vtbl, 0x50))(value, nullptr);
}
beam *create_gun_beam()
{
    auto *result = g_world_ptr->ent_mgr.create_and_add_beam(nullptr, make_unique_entity_id(), 0x2000);
    result->field_94 |= 0xC8;
    return result;
}
void remove_beam(vhandle_type<beam> &handle)
{
    if (auto *value = handle.get_volatile_ptr())
        g_world_ptr->ent_mgr.destroy_entity(value);
    handle.field_0 = INVALID_HANDLE;
}
void stop_beam_effects(beam *value)
{
    auto **last = reinterpret_cast<void **>(value->field_8C);
    for (auto **it = reinterpret_cast<void **>(value->field_88); it != last; ++it) {
        if (*it)
            reinterpret_cast<void(__fastcall *)(void *, void *, bool)>(
                get_vfunc(*reinterpret_cast<std::intptr_t *>(*it), 0x28))(*it, nullptr, true);
    }
}
string_hash move_hash(void *move)
{
    return move ? *reinterpret_cast<string_hash *>(static_cast<char *>(move) + 0x1C) : string_hash{};
}
}  // namespace

struct beam_collision_state {
    entity *target;
    bool enabled;
};
struct blaster_beam {
    _std::list<beam_collision_state> *ignored;
    vhandle_type<beam> body;
    vhandle_type<beam> trail;
    vector3d trail_start;
    float speed;
    vector3d direction;
    float remaining;
    int startup_frames;
    gun *owner;
    int actor_hits;
    int penetrations;
    string_hash attack;
    bool dynamic;
    bool active;

    void initialize(gun *source, bool allocated)
    {
        ignored = new _std::list<beam_collision_state>;
        body.field_0 = create_gun_beam()->my_handle;
        trail.field_0 = create_gun_beam()->my_handle;
        body.get_volatile_ptr()->set_visible(false, false);
        trail.get_volatile_ptr()->set_visible(false, false);
        owner = source;
        dynamic = allocated;
        active = false;
    }
    void release()
    {
        remove_beam(body);
        remove_beam(trail);
        delete ignored;
        ignored = nullptr;
    }
    void ignore(entity *target)
    {
        if (!target)
            return;
        for (const auto &entry : *ignored)
            if (entry.target == target)
                return;
        ignored->push_back({target, (target->field_4 & 0x4000) != 0});
    }
    void disable_ignored()
    {
        for (auto &entry : *ignored)
            if (entry.target) {
                entry.enabled = (entry.target->field_4 & 0x4000) != 0;
                entry.target->set_collisions_active(false, false);
            }
    }
    void restore_ignored()
    {
        for (auto &entry : *ignored)
            if (entry.target)
                entry.target->set_collisions_active(entry.enabled, false);
    }
};
VALIDATE_SIZE(blaster_beam, 0x44);

struct cached_gun_beam {
    vhandle_type<beam> handle;
    float lifetime;
    bool dynamic;
};
struct gun_beam_cache {
    int references;
    int advance_index;
    _std::list<cached_gun_beam *> *free_beams;
    _std::list<cached_gun_beam *> *used_beams;
    _std::list<blaster_beam *> *free_blasters;
    _std::list<blaster_beam *> *used_blasters;

    void un_mash(generic_mash_header *, gun *, generic_mash_data_ptrs *);
    void release_mem()
    {
        if (--references != 0)
            return;
        for (auto *list : {free_beams, used_beams}) {
            for (auto *entry : *list) {
                remove_beam(entry->handle);
                if (entry->dynamic)
                    delete entry;
            }
            delete list;
        }
        free_beams = used_beams = nullptr;
        for (auto *list : {free_blasters, used_blasters}) {
            for (auto *entry : *list) {
                entry->release();
                if (entry->dynamic)
                    delete entry;
            }
            delete list;
        }
        free_blasters = used_blasters = nullptr;
    }
    void frame_advance(float dt)
    {
        if (advance_index <= 0) {
            for (auto it = used_beams->begin(); it != used_beams->end();) {
                auto *entry = *it;
                if (entry) {
                    entry->lifetime = std::max(entry->lifetime - dt, 0.0f);
                    if (entry->lifetime <= 0.0f) {
                        if (auto *value = entry->handle.get_volatile_ptr()) {
                            stop_beam_effects(value);
                            value->set_visible(false, false);
                        }
                        free_beams->push_back(entry);
                        it = used_beams->erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }
        if (++advance_index >= references)
            advance_index = 0;
    }
    vhandle_type<beam> acquire(float lifetime)
    {
        cached_gun_beam *entry;
        if (free_beams->empty()) {
            entry = new cached_gun_beam;
            entry->handle.field_0 = create_gun_beam()->my_handle;
            entry->dynamic = true;
        } else {
            entry = free_beams->front();
            free_beams->pop_front();
        }
        entry->lifetime = lifetime;
        used_beams->push_front(entry);
        return entry->handle;
    }
    blaster_beam *acquire_blaster()
    {
        blaster_beam *entry;
        if (free_blasters->empty()) {
            entry = new blaster_beam{};
            entry->speed = 2.0f;
            entry->direction = ZVEC;
            entry->initialize(nullptr, true);
        } else {
            entry = free_blasters->front();
            free_blasters->pop_front();
        }
        entry->active = true;
        used_blasters->push_front(entry);
        return entry;
    }
    void release_blaster(blaster_beam *entry)
    {
        if (!entry)
            return;
        for (auto it = used_blasters->begin(); it != used_blasters->end(); ++it) {
            if (*it == entry) {
                entry->active = false;
                free_blasters->push_back(entry);
                used_blasters->erase(it);
                return;
            }
        }
    }
};
VALIDATE_SIZE(gun_beam_cache, 0x18);

void gun_beam_cache::un_mash(generic_mash_header *, gun *owner, generic_mash_data_ptrs *data)
{
    const auto unique_bytes = *data->get_from_shared<uint32_t>();
    const auto shared_bytes = *data->get_from_shared<uint32_t>();
    if (references) {
        data->field_0 += unique_bytes;
        data->field_4 += shared_bytes;
        ++references;
        return;
    }
    free_beams = new _std::list<cached_gun_beam *>;
    used_beams = new _std::list<cached_gun_beam *>;
    free_blasters = new _std::list<blaster_beam *>;
    used_blasters = new _std::list<blaster_beam *>;
    int count = *data->get_from_shared<int>();
    data->rebase_shared(4);
    for (int i = 0; i < count; ++i) {
        data->rebase_shared(4);
        auto *entry = data->get_from_shared<cached_gun_beam>();
        entry->dynamic = false;
        auto *value = create_gun_beam();
        value->set_visible(false, false);
        entry->handle.field_0 = value->my_handle;
        entry->lifetime = 0.0f;
        free_beams->push_back(entry);
    }
    count = *data->get_from_shared<int>();
    data->rebase_shared(4);
    for (int i = 0; i < count; ++i) {
        data->rebase_shared(4);
        auto *entry = data->get_from_shared<blaster_beam>();
        entry->initialize(owner, false);
        free_blasters->push_back(entry);
    }
    ++references;
}

void gun_beam::un_mash(generic_mash_header *header, gun *owner, generic_mash_data_ptrs *data)
{
    const auto name = *data->get_from_shared<tlFixedString>();
    texture = name.c_str()[0] ? nglLoadTexture(name) : nullptr;
    if (cached) {
        data->rebase_shared(4);
        cache = data->get_from_shared<gun_beam_cache>();
        cache->un_mash(header, owner, data);
    } else
        cache = nullptr;
    handle.field_0 = INVALID_HANDLE;
}

namespace {
void *__fastcall gun_delete(gun *self, void *, unsigned flags)
{
    self->~gun();
    if (flags & 1)
        mem_dealloc(self, sizeof(gun));
    return self;
}
void __fastcall gun_release(gun *self, void *)
{
    self->release_mem();
}
bool __fastcall gun_chunk(gun *, void *, void *, void *)
{
    return false;
}
bool __fastcall gun_query(gun *, void *)
{
    return true;
}
void __fastcall gun_unmash(gun *self, void *, generic_mash_header *h, void *o, generic_mash_data_ptrs *p)
{
    self->un_mash(h, o, p);
}
void __fastcall gun_advance(gun *self, void *, Float dt)
{
    self->frame_advance(dt);
}
void __fastcall gun_apply(gun *self, void *, actor *target)
{
    self->apply_effects(target);
}
void __fastcall gun_holster(gun *self, void *, bool visible)
{
    self->holster(visible);
}
void __fastcall gun_draw(gun *self, void *, bool visible)
{
    self->draw(visible);
}
void __fastcall gun_hide(gun *self, void *)
{
    self->hide();
}
void __fastcall gun_show(gun *self, void *)
{
    self->show();
}
void __fastcall gun_idle(gun *, void *) {}
void __fastcall gun_visible(gun *self, void *, bool visible)
{
    self->set_visibility(visible);
}
void __fastcall gun_position(gun *self, void *, vhandle_type<entity> source, const vector3d *pos, void *move)
{
    self->fire_at_position(source, *pos, move);
}
void __fastcall gun_target(gun *self, void *, vhandle_type<entity> source, vhandle_type<entity> target, void *move)
{
    self->fire_at_target(source, target, move);
}
void __fastcall gun_effects(gun *self, void *, entity *source, entity *target, const vector3d *pos,
                            const vector3d *face, string_hash attack)
{
    self->do_effects(source, target, *pos, *face, attack);
}
void __fastcall gun_update(gun *self, void *)
{
    self->update_continuous_fire();
}
void __fastcall gun_stop(gun *self, void *)
{
    self->deactivate_continuous_fire();
}
}  // namespace

void *gun::native_vtable(void **handheld_table)
{
    static std::array<void *, 0x304 / 4> table;
    std::copy_n(handheld_table, 0x2F8 / 4, table.begin());
    table[0] = reinterpret_cast<void *>(&gun_delete);
    table[0x10 / 4] = reinterpret_cast<void *>(&gun_release);
    table[0x20 / 4] = reinterpret_cast<void *>(&gun_chunk);
    table[0xE0 / 4] = reinterpret_cast<void *>(&gun_query);
    table[0x164 / 4] = reinterpret_cast<void *>(&gun_unmash);
    table[0x1A4 / 4] = reinterpret_cast<void *>(&gun_advance);
    table[0x2B8 / 4] = reinterpret_cast<void *>(&gun_apply);
    table[0x2C4 / 4] = reinterpret_cast<void *>(&gun_holster);
    table[0x2C8 / 4] = reinterpret_cast<void *>(&gun_draw);
    table[0x2CC / 4] = reinterpret_cast<void *>(&gun_hide);
    table[0x2D0 / 4] = reinterpret_cast<void *>(&gun_show);
    table[0x2D4 / 4] = reinterpret_cast<void *>(&gun_idle);
    table[0x2E8 / 4] = reinterpret_cast<void *>(&gun_visible);
    table[0x2F0 / 4] = reinterpret_cast<void *>(&gun_position);
    table[0x2F4 / 4] = reinterpret_cast<void *>(&gun_target);
    table[0x2F8 / 4] = reinterpret_cast<void *>(&gun_effects);
    table[0x2FC / 4] = reinterpret_cast<void *>(&gun_update);
    table[0x300 / 4] = reinterpret_cast<void *>(&gun_stop);
    return table.data();
}

gun::~gun() = default;

void gun::release_mem()
{
    fire_script.finalize(mash::allocation_scope{});
    flight_script.finalize(mash::allocation_scope{});
    release_blasters(true);
    delete blasters;
    blasters = nullptr;
    delete hits;
    hits = nullptr;
    for (auto *configuration : {&projectile, &laser}) {
        if (configuration->texture)
            nglReleaseTexture(configuration->texture);
        configuration->texture = nullptr;
        if (configuration->cache)
            configuration->cache->release_mem();
        configuration->cache = nullptr;
    }
    muzzle_effect.release_mem();
    handheld_item::release_mem();
}

void gun::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    blasters = new _std::list<blaster_beam *>;
    hits = new _std::list<gun_hit>;
    ::new (&fire_script) mString;
    ::new (&flight_script) mString;
    handheld_item::un_mash(header, object, data);
    for (auto *text : {&fire_script, &flight_script}) {
        data->rebase_shared(4);
        const int bytes = *data->get_from_shared<int>();
        *text = reinterpret_cast<const char *>(data->field_4);
        data->field_4 += bytes;
    }
    for (auto *effect : {&charge_effect, &muzzle_effect, &hit_effect, &world_effect, &blocked_effect})
        effect->un_mash(header, effect, data);
    projectile.un_mash(header, this, data);
    laser.un_mash(header, this, data);
}

void gun::deactivate_continuous_fire()
{
    field_10C &= ~0x2000;
    charge_effect.field_36 = muzzle_effect.field_36 = hit_effect.field_36 = blocked_effect.field_36 = 0;
    remove_beam(projectile.handle);
    muzzle_effect.kill(this, true);
}
void gun::remove_laser()
{
    remove_beam(laser.handle);
    field_10C &= ~0x400;
}
void gun::holster(bool visible)
{
    handheld_item::holster(visible);
    remove_laser();
    deactivate_continuous_fire();
}
void gun::draw(bool visible)
{
    if (!(field_10C & 1)) {
        shot_cooldown = 0.0f;
        deactivate_continuous_fire();
    }
    handheld_item::draw(visible);
}
void gun::hide()
{
    handheld_item::hide();
    if (field_10C & 1)
        remove_laser();
    deactivate_continuous_fire();
}
void gun::show()
{
    handheld_item::show();
    if (field_10C & 1)
        activate_laser();
    deactivate_continuous_fire();
}
void gun::set_visibility(bool visible)
{
    handheld_item::set_visibility(visible);
    if (field_10C & 1) {
        if (visible && field_110)
            activate_laser();
        else
            remove_laser();
    }
    deactivate_continuous_fire();
}
void gun::activate_laser()
{
    if (field_108 && (field_10C & 0x100))
        laser.spawn(true, calculate_fire_pos(), target_position, this, nullptr, nullptr);
    field_10C |= 0x400;
}
vector3d gun::calculate_fire_pos()
{
    auto *visual = field_104.get_volatile_ptr();
    entity_base *dirty = visual ? static_cast<entity_base *>(visual) : this;
    dirty->field_8 |= 0x10000040;
    for (auto *child = dirty->m_child; child; child = child->field_28)
        if (!(child->field_8 & 0x10000000))
            child->dirty_family(false);
    vector3d offset = projectile.offset;
    const auto position = visual ? visual->get_abs_position() : field_108->get_abs_position();
    if (visual && !(offset.length2() <= 0.0f))
        offset = visual->get_abs_po().non_affine_slow_xform(offset);
    return position + offset;
}
vector3d gun::calculate_laser_pos()
{
    auto *visual = field_104.get_volatile_ptr();
    vector3d offset = laser.offset;
    const auto position = visual ? visual->get_abs_position() : field_108->get_abs_position();
    if (visual && !(offset.length2() <= 0.0f))
        offset = visual->get_abs_po().non_affine_slow_xform(offset);
    return position + offset;
}
vector3d gun::fire_direction()
{
    if (auto *visual = field_104.get_volatile_ptr())
        return visual->get_abs_po().get_z_facing();
    return (field_108 ? field_108 : this)->get_abs_po().get_z_facing();
}
void gun::update_targeting()
{
    const auto start = calculate_laser_pos();
    if (auto *value = laser.handle.get_volatile_ptr()) {
        value->set_point_to_point(start, target_position);
        value->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    }
}
void gun::update_continuous_fire()
{
    if ((field_10C & 0x3000) != 0x3000)
        return;
    const auto start = calculate_fire_pos();
    if (auto *value = projectile.handle.get_volatile_ptr()) {
        value->set_point_to_point(start, target_position);
        value->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    }
    if (muzzle_effect.field_28 && field_108) {
        muzzle_effect.field_28->unforce_regions();
        muzzle_effect.field_28->force_regions(static_cast<entity *>(field_108));
    }
}
void gun::accumulate_hit(entity *value, const vector3d &position, const vector3d &direction)
{
    for (auto &hit : *hits)
        if (hit.target == value) {
            ++hit.count;
            hit.damage += damage;
            hit.position = position;
            hit.direction = direction;
            return;
        }
    hits->push_back({value, damage, position, direction, 1, 0.0f});
}
void gun::setup_fire_gun(entity *source, void *move)
{
    if (field_10C & 0x10000)
        return;
    fire_countdown = fire_delay;
    pending_source = source;
    pending_move = move;
    field_10C |= 0x10000;
    check_fire_gun();
}
void gun::check_fire_gun()
{
    if ((field_10C & 0x10000) && fire_countdown <= 0.0f) {
        field_10C &= ~0x10000;
        fire_gun(pending_source, pending_move);
    }
}
void gun::fire_at_position(vhandle_type<entity>, const vector3d &position, void *move)
{
    target = nullptr;
    tracked_target.field_0 = INVALID_HANDLE;
    target_position = position;
    hit_target = true;
    target_direction = position - get_abs_position();
    target_direction.normalize();
    setup_fire_gun(static_cast<entity *>(field_108), move);
}
void gun::fire_at_target(vhandle_type<entity>, vhandle_type<entity> handle, void *move)
{
    if (auto *value = handle.get_volatile_ptr()) {
        target = value;
        tracked_target = handle;
        target_position = value->get_abs_position();
        hit_target = true;
        target_direction = target_position - get_abs_position();
        target_direction.normalize();
        setup_fire_gun(static_cast<entity *>(field_108), move);
    }
}
void gun::apply_effects(actor *source)
{
    if ((field_10C & 0x3000) == 0x3000)
        deactivate_continuous_fire();
    else {
        if (field_108)
            source = static_cast<actor *>(field_108);
        fire_gun(source, nullptr);
        item::apply_effects(source);
    }
}
void gun::update_charge_effect()
{
    if (!charge_effect_dirty)
        return;
    charge_effect.kill(this, true);
    if (charge_effect_active) {
        auto *visual = field_104.get_volatile_ptr();
        auto direction = field_108->get_abs_po().get_z_facing();
        if (visual)
            direction = fire_direction();
        charge_effect.spawn(true, calculate_fire_pos(), direction, this, visual, visual, effect_offset, true, false);
    }
    charge_effect_dirty = false;
}

blaster_beam *gun_beam::spawn(bool persistent, const vector3d &start, const vector3d &end, gun *owner,
                              _std::list<blaster_beam *> *blasters, void *move)
{
    if (persistent)
        handle.field_0 = create_gun_beam()->my_handle;
    blaster_beam *entry = nullptr;
    beam *body;
    beam *trail = nullptr;
    if (blasters) {
        entry = cache->acquire_blaster();
        handle = entry->body;
        entry->startup_frames = 2;
        entry->actor_hits = entry->penetrations = 0;
        entry->owner = owner;
        body = handle.get_volatile_ptr();
        trail = entry->trail.get_volatile_ptr();
        body->field_74 = std::max(length, 0.0f);
        body->field_78 = 0.0f;
        entry->direction = end - start;
        entry->remaining = std::sqrt(entry->direction.length2());
        entry->direction *= 1.0f / entry->remaining;
        entry->remaining = owner->range > 0.0f ? owner->range : 2000.0f;
        entry->speed = speed;
        entry->trail_start = start;
        entry->attack = move_hash(move);
        const auto tip = start + entry->direction * body->field_74;
        body->set_point_to_point(start, tip);
        trail->set_point_to_point(start, tip);
        blasters->push_back(entry);
    } else {
        if (persistent)
            return nullptr;
        handle = cache->acquire(lifetime);
        body = handle.get_volatile_ptr();
        body->set_point_to_point(start, end);
    }
    if (!body)
        return nullptr;
    auto configure_texture = [this](beam *value) {
        if (!texture)
            return;
        if (!value->my_material)
            value->my_material = new PCUV_ShaderMaterial;
        else if (value->my_material->m_texture)
            nglReleaseTexture(value->my_material->m_texture);
        value->my_material->m_texture = texture;
        nglAddTextureRef(texture);
    };
    body->field_94 |= 0xC8;
    body->field_E2 = additive;
    body->field_7C = body->field_80 = color;
    body->field_70 = width;
    body->field_DC = texture_scale;
    configure_texture(body);
    body->unforce_regions();
    body->set_visible(true, false);
    body->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    if (trail) {
        trail->field_94 |= 0xC8;
        trail->field_E2 = additive;
        trail->field_7C = color;
        trail->field_7C[3] = 0;
        trail->field_80 = color;
        trail->field_80[3] = static_cast<uint8_t>(color[3] * 0.5f + 0.5f);
        trail->field_70 = width * 0.5f;
        trail->field_DC = texture_scale;
        configure_texture(trail);
        trail->unforce_regions();
        trail->set_visible(owner->trail_length > 0.0001f, false);
        trail->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    }
    if (!persistent || blasters)
        handle.field_0 = INVALID_HANDLE;
    return entry;
}

bool gun::can_hit_this_target(entity *value)
{
    if (!value)
        return false;
    auto owner_fn = reinterpret_cast<entity *(__fastcall *)(gun *, void *)>(get_vfunc(m_vtbl, 0x2DC));
    auto *owner = owner_fn(this, nullptr);
    if (!owner)
        return true;
    if (owner == value)
        return false;
    if (!owner->is_an_actor() || !value->is_an_actor())
        return true;
    auto *owner_core = owner->get_ai_core();
    auto *target_core = value->get_ai_core();
    if (!owner_core || !target_core || !(field_10C & 0x8000))
        return true;
    const string_hash team_key{int(to_hash("team"))};
    const auto first = ai::team::manager::get_team_enum_by_hash(owner_core->field_50.get_pb_hash(team_key));
    const auto second = ai::team::manager::get_team_enum_by_hash(target_core->field_50.get_pb_hash(team_key));
    return first != 15 && second != 15 && !ai::team::manager::is_friend(first, second);
}

void gun::release_blasters(bool recycle)
{
    for (auto it = blasters->begin(); it != blasters->end();) {
        auto *entry = *it;
        auto *body = entry->body.get_volatile_ptr();
        auto *trail = entry->trail.get_volatile_ptr();
        body->set_visible(false, false);
        body->raise_event(event::DESTROYED);
        trail->set_visible(false, false);
        if (recycle) {
            projectile.cache->release_blaster(entry);
            it = blasters->erase(it);
        } else
            ++it;
    }
}

void gun::frame_advance(Float elapsed)
{
    const float dt = elapsed.value;
    if (field_10C & 0x10000) {
        fire_countdown -= dt;
        check_fire_gun();
    }
    charge_effect.frame_advance(elapsed);
    muzzle_effect.frame_advance(elapsed);
    hit_effect.frame_advance(elapsed);
    blocked_effect.frame_advance(elapsed);
    world_effect.frame_advance(elapsed);
    if (projectile.cache)
        projectile.cache->frame_advance(dt);
    if (shot_cooldown > 0.0f)
        shot_cooldown = std::max(shot_cooldown - dt, 0.0f);
    if (!field_108)
        return;
    if ((field_10C & 0x4400) == 0x4400) {
        calculate_target(0.0f, false, ZEROVEC, false);
        update_targeting();
    }
    update_charge_effect();
    if ((field_10C & 0x3000) == 0x3000) {
        const bool ready = shot_cooldown <= 0.0f;
        if (!ready && fire_delay <= 0.0f) {
            if (auto *tracked = tracked_target.get_volatile_ptr()) {
                target_position = tracked->get_abs_position();
                hit_target = true;
                target_direction = target_position - get_abs_position();
                target_direction.normalize();
                update_targeting();
            }
        }
        if ((ready && fire_delay > 0.0f) || (!ready && fire_delay <= 0.0f)) {
            vector3d direction = target_position - calculate_fire_pos();
            direction.normalize();
            const auto saved_position = target_position;
            const auto saved_direction = target_direction;
            auto *saved_target = target;
            calculate_target(spread, true, direction, false);
            reinterpret_cast<void(__fastcall *)(gun *, void *)>(get_vfunc(m_vtbl, 0x2FC))(this, nullptr);
            target_position = saved_position;
            target_direction = saved_direction;
            target = saved_target;
        }
        if (ready)
            setup_fire_gun(static_cast<entity *>(field_108), nullptr);
    }
    if (!blasters->empty())
        update_blasters(dt);
    handheld_item::frame_advance(elapsed);
}

namespace {
bool __fastcall gun_capsule_filter(const local_collision::entfilter_base *, void *, actor *value,
                                   dynamic_conglomerate_clone *clone, const local_collision::query_args_t *args)
{
    return value->colgeom->get_type() == collision_geometry::CAPSULE && (value->field_4 & 0x80000) &&
           !(value->field_4 & 0x20000) && entity_is_alive(value) &&
           local_collision::entity_line_segment_test(value, clone, *args);
}
const local_collision::entfilter_base &capsule_filter()
{
    static const local_collision::entfilter_base::native_vtable table{gun_capsule_filter};
    static const local_collision::entfilter_base filter{reinterpret_cast<std::intptr_t>(&table)};
    return filter;
}
}  // namespace

void gun::calculate_target(float dispersion, bool firing, const vector3d &desired_direction, bool penetrate)
{
    if (!field_108)
        return;
    vector3d normal = ZEROVEC;
    entity *hit = nullptr;
    auto *visual = field_104.get_volatile_ptr();
    vector3d start, direction, up;
    if (visual && visual->m_parent) {
        start = firing ? calculate_fire_pos() : calculate_laser_pos();
        direction = desired_direction;
        if (field_10C & 0x4000) {
            direction = fire_direction();
            if (aim_cone > 0.0f && std::cos(aim_cone) + 1.0f < dot(direction, desired_direction) + 1.0f)
                direction = desired_direction;
        }
        const auto &pose = visual->get_abs_po();
        up = pose.get_y_facing();
        if (is_colinear(direction, up, Float{0.01f}))
            up = pose.get_x_facing();
        po orientation;
        orientation.set_po(up, direction, ZEROVEC);
        direction = orientation.get_y_facing();
        up = orientation.get_z_facing();
        if (g_world_ptr->the_terrain->find_region(start, nullptr) && firing) {
            auto behind = start - direction * 0.5f;
            vector3d point;
            if (find_intersection(start,
                                  behind,
                                  *local_collision::entfilter_entity_no_capsules,
                                  *local_collision::obbfilter_lineseg_test,
                                  &point,
                                  &normal,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  false))
                behind = point + normal * 0.1f;
            g_world_ptr->the_terrain->find_region(behind, nullptr);
        }
    } else {
        const auto &pose = field_108->get_abs_po();
        start = pose.get_position();
        if (desired_direction.length2() > 0.01f) {
            direction = desired_direction;
            up = pose.get_y_facing();
            if (is_colinear(direction, up, Float{0.01f}))
                up = pose.get_x_facing();
            po orientation;
            orientation.set_po(up, direction, ZEROVEC);
            direction = orientation.get_y_facing();
            up = orientation.get_z_facing();
        } else {
            direction = pose.get_z_facing();
            up = pose.get_y_facing();
        }
    }
    if (dispersion > 0.0f) {
        const auto original = direction;
        po rotation = po_identity_matrix;
        const float random = static_cast<float>(std::rand()) / 32767.0f;
        rotation.set_rot(up, Float{(random + random - 1.0f) * dispersion * 0.5f});
        direction = rotation.non_affine_slow_xform(direction);
        rotation.set_rot(original, Float{static_cast<float>(std::rand()) / 32767.0f * 6.2831855f});
        direction = rotation.non_affine_slow_xform(direction);
    }
    auto *owner = static_cast<entity *>(field_108);
    const bool owner_collisions = (owner->field_4 & 0x4000) != 0;
    owner->set_collisions_active(false, false);
    entity *root = nullptr;
    bool root_collisions = false;
    if (owner->field_4 & 0x8000) {
        root = static_cast<entity *>(owner->get_conglom_owner());
        if (root == owner)
            root = nullptr;
        if (root) {
            root_collisions = (root->field_4 & 0x4000) != 0;
            root->set_collisions_active(false, false);
        }
    }
    auto end = start + direction * (range > 0.0f ? range : 1000.0f);
    auto segment = end - start;
    if (segment.length2() > 2500.0f) {
        segment.normalize();
        end = start + segment * 50.0f;
    }
    vector3d world_point, world_normal, point;
    const bool world_hit = find_intersection(start,
                                             end,
                                             *local_collision::entfilter_entity_no_capsules,
                                             *local_collision::obbfilter_lineseg_test,
                                             &world_point,
                                             &world_normal,
                                             nullptr,
                                             &hit,
                                             nullptr,
                                             false);
    if (world_hit)
        end = world_point;
    hit_target = find_intersection(start,
                                   end,
                                   *local_collision::entfilter_entity_collision,
                                   *local_collision::obbfilter_lineseg_test,
                                   &point,
                                   &normal,
                                   nullptr,
                                   &hit,
                                   nullptr,
                                   false);
    if (!(field_10C & 0x800)) {
        for (int i = 0; i < penetrations; ++i) {
            if (!penetrate || damage_radius > 0.0f || !hit_target || !hit || !has_damage_interface(hit))
                break;
            auto *damage_ifc = get_damage_interface(hit);
            if (!(damage_ifc->field_1F8 & 1) || !hit->colgeom ||
                hit->colgeom->get_type() == collision_geometry::CAPSULE || (hit->field_8 & 0x4000))
                break;
            accumulate_hit(hit, point, normal * -1.0f);
            start = point;
            g_world_ptr->the_terrain->find_region(start, nullptr);
            if (damage_ifc->field_1FC.field_0[0] > damage)
                damage_ifc->field_184.spawn(
                    false, point, normal, nullptr, damage_ifc->field_4, damage_ifc->field_4, ZEROVEC, false, true);
            hit->set_collisions_active(false, false);
            hit = nullptr;
            const auto &filter =
                max_targets > 1 && damage_radius <= 0.0f
                    ? static_cast<const local_collision::entfilter_base &>(*local_collision::entfilter_blocks_beams)
                    : capsule_filter();
            hit_target = find_intersection(start,
                                           end,
                                           filter,
                                           *local_collision::obbfilter_reject_all,
                                           &point,
                                           &normal,
                                           nullptr,
                                           &hit,
                                           nullptr,
                                           false);
        }
    }
    for (auto &entry : *hits)
        if (entry.target)
            entry.target->set_collisions_active(true, false);
    owner->set_collisions_active(owner_collisions, false);
    if (root)
        root->set_collisions_active(root_collisions, false);
    if (hit_target) {
        normal.normalize();
        target = hit;
        target_direction = normal;
        target_direction.normalize();
        target_position = point + target_direction * 0.01f;
    } else if (world_hit) {
        hit_target = true;
        target = nullptr;
        target_direction = world_normal;
        target_direction.normalize();
        target_position = world_point + target_direction * 0.01f;
    } else {
        target_position = end;
        target_direction = start - end;
        target_direction.normalize();
        target = nullptr;
    }
}

namespace {
void gun_impulse(entity *target, const vector3d &direction, float force)
{
    if (!target->has_physical_ifc() || !(force > 0.0f))
        return;
    auto *physical = target->physical_ifc();
    auto delta = direction * force;
    if (physical->field_184 && target->get_abs_po().get_y_facing().y > 0.8f)
        delta.y = 0.0f;
    const vector3d center_of_mass{1.0e11f, 1.0e11f, 1.0e11f};
    physical->apply_force_increment(delta, static_cast<physical_interface::force_type>(1), center_of_mass, 0);
    physical->field_A4 = 1.0f;
}
void run_gun_script(gun *owner, const mString &name, entity *target, const vector3d &position,
                    const vector3d &direction)
{
    if (find_func_and_spawn_new_thread(owner, string_hash{name.c_str()})) {
        script::push_arg(owner);
        script::push_arg(target);
        script::push_arg(position);
        script::push_arg(direction);
        script::exec_thread(false);
    }
}
void apply_gun_damage(damage_interface *ifc, entity *source, float amount, int type, const vector3d &position,
                      const vector3d &direction, string_hash attack, const vector3d &impulse)
{
    const string_hash empty{};
    ifc->apply_damage(source, amount, type, position, direction, 0, attack, empty, empty, false, impulse, 17, false);
}
}  // namespace

void gun::do_effects(entity *source, entity *victim, const vector3d &position, const vector3d &face, string_hash attack)
{
    if (source) {
        if (damage_radius > 0.0f) {
            hit_effect.spawn(false, position, face, this, victim, nullptr, ZEROVEC, true, true);
            if (hit_target || (field_10C & 0x800))
                world_effect.spawn(false, position, face, this, victim, victim, ZEROVEC, false, true);
            _std::list<entity *> candidates;
            if (damage_interface::find_damageable(position, damage_radius, 3, true) > 0) {
                for (auto *damage_ifc : *damage_interface::found_damageable) {
                    auto *candidate = damage_ifc->field_4;
                    if (candidate && (candidate->field_4 & 0x200) && !(candidate->field_4 & 0x20000) &&
                        has_damage_interface(candidate) && entity_is_alive(candidate))
                        candidates.push_back(candidate);
                }
            }
            for (auto *candidate : candidates) {
                auto direction = candidate->get_abs_position() - position;
                const float squared = direction.length2();
                if (squared > 1.0f)
                    direction *= 1.0f / std::sqrt(squared);
                else
                    direction = YVEC;
                if (!can_hit_this_target(candidate))
                    continue;
                vector3d force = ZEROVEC;
                if (impulse > 0.0f) {
                    force = direction * impulse;
                    if (candidate->has_physical_ifc())
                        candidate->physical_ifc()->field_A4 = 1.0f;
                }
                const float amount = candidate == victim ? damage * direct_damage_multiplier : damage;
                apply_gun_damage(get_damage_interface(candidate),
                                 this,
                                 amount,
                                 6,
                                 candidate->get_abs_position(),
                                 direction,
                                 attack,
                                 force);
            }
        } else if (victim && has_damage_interface(victim)) {
            auto direction = victim->get_abs_position() - source->get_abs_position();
            direction.normalize();
            bool successful = (victim->field_8 & 0x4000) != 0;
            if ((field_10C & 0x800) ||
                (((field_10C & 0x1000) || pellets == 1) && (max_targets <= 1 || damage_radius > 0.0f))) {
                if (can_hit_this_target(victim)) {
                    if (has_damage_interface(victim)) {
                        entity *emitter = field_104.get_volatile_ptr();
                        if (!emitter)
                            emitter = this;
                        apply_gun_damage(
                            get_damage_interface(victim), emitter, damage, 5, position, direction, attack, ZEROVEC);
                        if (victim->is_an_actor()) {
                            if (auto *core = victim->get_ai_core()) {
                                auto *node = static_cast<ai::damage_inode *>(
                                    core->get_info_node(ai::damage_inode::default_id, false));
                                if (node && !(node->field_C->field_8 & 0x4000)) {
                                    node->field_1C = g_world_ptr->time_manager.field_C;
                                    node->field_20 = g_world_ptr->time_manager.field_8;
                                }
                            }
                        }
                        if (get_damage_interface(victim)->field_104.field_3E)
                            successful = true;
                    }
                    gun_impulse(victim, direction, impulse);
                }
            } else
                accumulate_hit(victim, position, direction);
            if (has_sound_and_pfx_ifc()) {
                const auto material =
                    terrain_types_manager::get_terrain_type_by_index(static_cast<uint8_t>(victim->field_41));
                const auto &where = victim->get_abs_position();
                my_sound_and_pfx_interface->play_terrain_sound(
                    static_cast<eTerrainSoundType>(successful ? 11 : 10), material, 1.0f, &where);
            }
            auto &effect = successful ? hit_effect : blocked_effect;
            effect.spawn(false, position, face, this, victim, nullptr, ZEROVEC, true, true);
        } else if (hit_target || (field_10C & 0x800)) {
            hit_effect.spawn(false, position, face, this, victim, nullptr, ZEROVEC, true, true);
            world_effect.spawn(false, position, face, this, victim, victim, ZEROVEC, false, true);
        }
    }
    hit_target = false;
}

void gun::fire_gun(entity *source, void *move)
{
    auto *visual = field_104.get_volatile_ptr();
    if (charge_effect_active)
        charge_effect.kill(this, true);
    charge_effect_dirty = charge_effect_active;
    charge_effect_active = false;
    hits->clear();
    if (magazine_size > 0 && ammunition <= 0) {
        ammunition = magazine_size;
        reloaded = true;
        field_365 = false;
        if (field_108 && has_sound_and_pfx_ifc()) {
            const auto &position = (visual ? static_cast<entity_base *>(visual) : field_108)->get_abs_position();
            my_sound_and_pfx_interface->play_sound_grp_at(string_hash{"RELOAD"}, &position, 1, 1, 1, -1, -1);
        }
        shot_cooldown = shot_interval;
        return;
    }
    const auto start = calculate_fire_pos();
    if ((field_10C & 0x3000) != 0x3000) {
        const auto facing = visual ? fire_direction() : field_108->get_abs_po().get_z_facing();
        auto availability = reinterpret_cast<bool(__fastcall *)(gun *, void *)>(get_vfunc(m_vtbl, 0x2D8));
        muzzle_effect.spawn((field_10C & 0x1000) != 0,
                            start,
                            facing,
                            this,
                            visual,
                            visual,
                            effect_offset,
                            true,
                            availability(this, nullptr));
    }
    auto direction = target_position - calculate_fire_pos();
    direction.normalize();
    const auto saved_position = target_position;
    const auto saved_direction = target_direction;
    auto *saved_target = target;
    if (field_10C & 0x1000)
        field_10C |= 0x2000;
    else
        field_10C &= ~0x2000;
    using effects_fn =
        void(__fastcall *)(gun *, void *, entity *, entity *, const vector3d *, const vector3d *, string_hash);
    const auto effects = reinterpret_cast<effects_fn>(get_vfunc(m_vtbl, 0x2F8));
    for (int pellet = 0; pellet < pellets; ++pellet) {
        calculate_target(spread, true, direction, true);
        if (pellet == 0)
            reinterpret_cast<void(__fastcall *)(gun *, void *)>(get_vfunc(m_vtbl, 0x2FC))(this, nullptr);
        if (field_10C & 0x200)
            projectile.spawn(false, start, target_position, this, (field_10C & 0x800) ? blasters : nullptr, move);
        if (!fire_script.empty())
            run_gun_script(this, fire_script, target, start, target_direction);
        if ((field_10C & 0x800) || max_targets <= 1 || damage_radius > 0.0f) {
            if (!(field_10C & 0x800))
                effects(this, nullptr, source, target, &target_position, &target_direction, move_hash(move));
        } else {
            auto ray = target_position - start;
            const float length = std::sqrt(ray.length2());
            if (length > 0.0001f) {
                ray *= 1.0f / length;
                effects(this, nullptr, source, target, &target_position, &target_direction, move_hash(move));
                if (damage_interface::find_damageable((start + target_position) * 0.5f, length * 0.5f, 3, true) > 0) {
                    _std::vector<gun_hit> candidates;
                    for (auto *ifc : *damage_interface::found_damageable) {
                        auto *candidate = ifc->field_4;
                        if (!can_hit_this_target(candidate))
                            continue;
                        auto point = candidate->get_abs_position();
                        auto normal = ray * -1.0f;
                        using segment_fn = bool(__fastcall *)(
                            entity *, void *, const vector3d *, const vector3d *, vector3d *, vector3d *, float, bool);
                        if (reinterpret_cast<segment_fn>(get_vfunc(candidate->m_vtbl, 0x250))(
                                candidate, nullptr, &start, &target_position, &point, &normal, 1.0f, true))
                            candidates.push_back({candidate, damage, point, normal, 1, (point - start).length2()});
                    }
                    std::sort(candidates.begin(), candidates.end(), [](const gun_hit &a, const gun_hit &b) {
                        return a.distance_squared < b.distance_squared;
                    });
                    int count = 0;
                    for (auto &candidate : candidates) {
                        if (count++ >= max_targets)
                            break;
                        effects(this,
                                nullptr,
                                source,
                                candidate.target,
                                &candidate.position,
                                &candidate.direction,
                                move_hash(move));
                    }
                }
            }
        }
    }
    target_position = saved_position;
    target_direction = saved_direction;
    target = saved_target;
    for (auto &hit : *hits) {
        if (!can_hit_this_target(hit.target))
            continue;
        if (has_damage_interface(hit.target)) {
            vector3d force = ZEROVEC;
            int type = 5;
            if (explosive_hit_count > 0 && hit.count >= explosive_hit_count) {
                type = 6;
                if (impulse > 0.0f) {
                    if (hit.target->has_physical_ifc())
                        hit.target->physical_ifc()->field_A4 = 1.0f;
                    force = hit.direction * impulse;
                }
            } else
                gun_impulse(hit.target, hit.direction, impulse);
            apply_gun_damage(get_damage_interface(hit.target),
                             this,
                             hit.damage,
                             type,
                             hit.position,
                             hit.direction,
                             move_hash(move),
                             force);
        } else
            gun_impulse(hit.target, hit.direction, impulse);
    }
    hits->clear();
    ++shots_fired;
    if (ammunition > 0)
        --ammunition;
    shot_cooldown = shot_interval;
}

void gun::update_blasters(float dt)
{
    for (auto it = blasters->begin(); it != blasters->end();) {
        auto *entry = *it;
        if (entry->startup_frames) {
            entry->startup_frames = std::max(entry->startup_frames - 1, 0);
            ++it;
            continue;
        }
        entry->remaining -= dt * entry->speed;
        auto *body = entry->body.get_volatile_ptr();
        auto *trail = entry->trail.get_volatile_ptr();
        const auto start = body->get_abs_position();
        const auto movement = entry->direction * (dt * entry->speed);
        const auto position = start + movement;
        auto direction = movement;
        direction.normalize();
        const auto end = position + direction * body->field_74;
        if (flight_script.c_str()[0])
            run_gun_script(this, flight_script, target, position, movement);
        auto *owner = static_cast<entity *>(field_108);
        const bool owner_not_excluded = owner && !(owner->field_4 & 0x20000);
        const bool owner_collisions = owner && (owner->field_4 & 0x4000);
        if (owner) {
            owner->field_4 |= 0x20000;
            owner->set_collisions_active(false, false);
        }
        entry->disable_ignored();
        entity *victim = nullptr;
        vector3d point, normal;
        const bool collided = find_intersection(start,
                                                end,
                                                *local_collision::entfilter_entity_collision,
                                                *local_collision::obbfilter_lineseg_test,
                                                &point,
                                                &normal,
                                                nullptr,
                                                &victim,
                                                nullptr,
                                                false);
        entry->restore_ignored();
        if (owner) {
            if (owner_not_excluded)
                owner->field_4 &= ~0x20000;
            else
                owner->field_4 |= 0x20000;
            owner->set_collisions_active(owner_collisions, false);
        }
        body->set_abs_position(position);
        if (trail_length <= 0.0001f)
            trail->set_visible(false, false);
        else {
            auto displacement = position - entry->trail_start;
            const float length = std::sqrt(displacement.length2());
            if (length > trail_length)
                entry->trail_start = position - displacement * (trail_length / length);
            trail->set_point_to_point(entry->trail_start, position);
            trail->set_visible(true, false);
        }
        moved_entities::add_moved(vhandle_type<entity>{entry->body.field_0});
        if (!collided && entry->remaining > 0.0f) {
            ++it;
            continue;
        }
        if (collided) {
            entity *source = owner;
            if (!source)
                source = field_104.get_volatile_ptr();
            if (!source)
                source = this;
            using effects_fn =
                void(__fastcall *)(gun *, void *, entity *, entity *, const vector3d *, const vector3d *, string_hash);
            reinterpret_cast<effects_fn>(get_vfunc(m_vtbl, 0x2F8))(
                this, nullptr, source, victim, &point, &normal, entry->attack);
        }
        bool keep = false;
        if (entry->remaining > 0.0f && collided && victim && victim->colgeom) {
            if (victim->colgeom->get_type() == collision_geometry::CAPSULE) {
                keep = ++entry->actor_hits < max_targets;
                if (keep)
                    entry->ignore(victim);
            } else if (entry->penetrations < penetrations && damage_radius <= 0.0f && has_damage_interface(victim) &&
                       (get_damage_interface(victim)->field_1F8 & 1) && !(victim->field_8 & 0x4000)) {
                entry->ignore(victim);
                ++entry->penetrations;
                auto *ifc = get_damage_interface(victim);
                if (ifc->field_1FC.field_0[0] > damage)
                    ifc->field_184.spawn(
                        false, point, normal, nullptr, ifc->field_4, ifc->field_4, ZEROVEC, false, true);
                keep = true;
            }
        }
        if (keep) {
            ++it;
            continue;
        }
        body->set_visible(false, false);
        body->raise_event(event::DESTROYED);
        trail->set_visible(false, false);
        projectile.cache->release_blaster(entry);
        it = blasters->erase(it);
    }
}
