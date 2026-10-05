#include "traffic_inode.h"

#include "common.h"
#include "actor.h"
#include "damage_interface.h"
#include "traffic.h"
#include "aeps.h"
#include "ai_player_controller.h"
#include "conglom.h"
#include "decal_morphs.h"
#include "event.h"
#include "event_manager.h"
#include "sound_and_pfx_interface.h"
#include "wds.h"
#include "als_animation_logic_system.h"
#include "als_inode.h"
#include "base_ai_core.h"

#include <cmath>
#include <limits>

#include <cstdlib>

namespace ai {

VALIDATE_SIZE(traffic_inode, 0xCC);
VALIDATE_SIZE(traffic_inode::CarCombatInfo, 0x34);
VALIDATE_OFFSET(traffic_inode, hood, 0x1C);
VALIDATE_OFFSET(traffic_inode, roof, 0x50);
VALIDATE_OFFSET(traffic_inode, trunk, 0x84);
VALIDATE_OFFSET(traffic_inode, flags, 0xB8);
VALIDATE_OFFSET(traffic_inode, traffic_ptr, 0xC0);
VALIDATE_OFFSET(traffic_inode, field_C8, 0xC8);

namespace {
void __fastcall native_destruct(traffic_inode *self, void *)
{
    self->_destruct_mashed_class();
}
void __fastcall native_unmash(traffic_inode *self, void *, mash_info_struct *info, void *context)
{
    self->_unmash(info, context);
}
void *__fastcall native_delete(traffic_inode *self, void *, unsigned flags)
{
    self->~traffic_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(traffic_inode));
    return self;
}
unsigned __fastcall native_type(traffic_inode *, void *)
{
    return 422;
}
bool __fastcall native_subclass(traffic_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall native_is_or_subclass(traffic_inode *self, void *, unsigned type)
{
    return self->_is_or_is_subclass_of(static_cast<mash::virtual_types_enum>(type));
}
bool __fastcall native_needs_advance(traffic_inode *, void *)
{
    return true;
}
void __fastcall native_advance(traffic_inode *self, void *, Float time)
{
    self->_frame_advance(time);
}
void __fastcall native_activate(traffic_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}

void __fastcall native_deactivate(traffic_inode *, void *) {}
void __fastcall native_reset(traffic_inode *self, void *)
{
    self->_reset();
}
int __fastcall native_size(traffic_inode *, void *)
{
    return sizeof(traffic_inode);
}
}  // namespace

void *traffic_inode::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&native_destruct),
        reinterpret_cast<void *>(&native_unmash),
        reinterpret_cast<void *>(&native_delete),
        reinterpret_cast<void *>(&native_type),
        reinterpret_cast<void *>(&native_subclass),
        reinterpret_cast<void *>(&native_is_or_subclass),
        reinterpret_cast<void *>(&native_needs_advance),
        reinterpret_cast<void *>(&native_advance),
        reinterpret_cast<void *>(&native_activate),
        reinterpret_cast<void *>(&native_deactivate),
        reinterpret_cast<void *>(&native_reset),
        reinterpret_cast<void *>(&native_size),
    };
    return table;
}

traffic_inode::~traffic_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    traffic::destroy_traffic(traffic_ptr);
    traffic_ptr = nullptr;
}

void traffic_inode::_destruct_mashed_class()
{
    traffic::destroy_traffic(traffic_ptr);
    traffic_ptr = nullptr;
    info_node::_destruct_mashed_class();
}

void traffic_inode::attach_to_traffic()
{
    if (traffic_ptr || (flags & 1))
        return;
    traffic_ptr = traffic::create_traffic_from_entity(vhandle_type<entity>{field_C->my_handle});
    if (traffic_ptr) {
        traffic_ptr->field_21C = this;
        determine_vehicle_type_for_anims();
    }
    auto *als_node = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
    const bool was_active = (field_C->field_4 & 8) != 0;
    field_C->field_4 |= 8;
    const bool was_occupied = traffic_ptr && traffic_ptr->is_ai_car_occupied();
    if (traffic_ptr)
        traffic_ptr->field_202 = true;
    als_node->get_system()->force_update();
    if (was_active)
        field_C->field_4 |= 8;
    else
        field_C->field_4 &= ~8u;
    if (traffic_ptr)
        traffic_ptr->field_202 = was_occupied;
}

void traffic_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    attach_to_traffic();
    core->field_64->field_4 |= 8;
}

bool traffic_inode::remove_from_traffic_system(bool play_voice)
{
    bool replace = false;
    if (traffic_ptr) {
        if (play_voice)
            traffic_ptr->play_car_toss_voice();
        traffic_ptr->set_destroyable(true);
        if (traffic_ptr->field_158) {
            traffic::set_destroyed_elsewhere(traffic_ptr);
            replace = true;
        }
    }
    traffic::destroy_traffic(traffic_ptr);
    traffic_ptr = nullptr;
    flags |= 1;
    if (replace)
        traffic::create_new_traffic(-1);
    return true;
}

void traffic_inode::_frame_advance(Float time)
{
    auto *hero = g_world_ptr->get_hero_ptr(0);
    if (hero && car_combat_enabled() && hero->is_a_parent(field_C)) {
        auto *car = field_C->is_a_conglomerate() ? static_cast<conglomerate *>(field_C) : nullptr;
        entity_base *members[] = {
            car->get_member(string_hash("HOOD"), true),
            car->get_member(string_hash("ROOF"), true),
            car->get_member(string_hash("TRUNK"), true),
        };
        entity_base *closest = members[1];
        float closest_distance = std::numeric_limits<float>::max();
        const vector3d hero_position = hero->get_abs_position();

        for (int index = 0; index < 2; ++index) {
            if (members[index]) {
                const vector3d delta = hero_position - members[index]->get_abs_position();
                const float distance = delta.xz_length2();
                if (std::fabs(delta.y) < 6.0f && distance < closest_distance) {
                    closest_distance = distance;
                    closest = members[index];
                }
            }
        }
        if (closest == members[0])
            hood.frame_advance(field_C, time);
        else if (closest == members[1])
            roof.frame_advance(field_C, time);
        else
            trunk.frame_advance(field_C, time);
    }
    if (!traffic_ptr)
        attach_to_traffic();
    if (traffic_ptr)
        traffic_ptr->advance(time);
}

traffic_inode::CarCombatInfo::CarCombatInfo()
    : section(3), hit_points(200), fire_interval(1.0f), burst_size(0), attack_interval(3.0f), damage(0.0f),
      sense_lead_time(0.0f), counter_window(0.0f), field_30(false), dodged(false), attack_left(false),
      attack_announced(false)
{}

traffic_inode::CarCombatInfo::CarCombatInfo(from_mash_in_place_constructor *) {}

void traffic_inode::CarCombatInfo::reset_timers()
{
    shot_timer = 0.0f;
    const double random = std::rand() * (1.0 / 32768.0);
    shots_remaining = burst_size;
    attack_timer = (random + random - 1.0) * 0.5 + 0.5 + attack_interval;
    sense_timer = attack_timer - sense_lead_time;
    attack_left = std::rand() * (1.0 / 32768.0) < 0.45f;
    dodged = false;
    attack_announced = false;
}

void traffic_inode::CarCombatInfo::frame_advance(actor *owner, Float time)
{
    auto *hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0));
    if (!hero || burst_size <= 0 || hit_points <= 0 || !hero->is_a_parent(owner))
        return;

    sense_timer -= time;
    attack_timer -= time;
    if (attack_timer <= 0.0f)
        shot_timer -= time;
    if (!attack_announced && attack_timer <= 2.0f) {
        event_manager::raise_event(attack_left ? event::CAR_COMBAT_LEFT_ATTACK : event::CAR_COMBAT_RIGHT_ATTACK,
                                   owner->my_handle);
        attack_announced = true;
    }
    if (sense_timer <= 0.0f) {
        aeps::DoSpideySenseEffect(hero, 0.1f, 0);
        const float dodge_axis = hero->m_player_controller->field_2BC[1].field_10;
        if (attack_left ? dodge_axis > 0.8f : dodge_axis < -0.8f)
            dodged = true;
    }

    if (attack_timer <= 0.0f && shot_timer <= 0.0f && burst_size > 0) {
        const double random = std::rand() * (1.0 / 32768.0);
        shot_timer = (random * 2.0 - 1.0) * fire_interval * 0.25 + fire_interval * 0.5 + fire_interval;
        sense_timer = shot_timer - sense_lead_time;
        if (hero->has_damage_ifc()) {
            bool miss = dodged;
            if (!miss && section != 0 && burst_size * 0.5 < shots_remaining) {
                const float chance = 1.0f / burst_size + 1.0f / burst_size;
                miss = std::rand() * (1.0 / 32768.0) > chance;
            }
            attack_announced = false;
            auto *sound = owner->my_sound_and_pfx_interface;
            if (miss) {
                sound->play_sound_grp(string_hash("CAR_COMBAT_SHOOT_MISS"), 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
                event_manager::raise_event(event::CAR_COMBAT_DODGED, owner->my_handle);
            } else {
                const vector3d from = owner->get_abs_position();
                sound->play_sound_grp(string_hash("CAR_COMBAT_SHOOT"), 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
                const vector3d direction = (from - hero->get_abs_position()).normalized();
                hero->damage_ifc()->apply_damage(owner,
                                                 damage,
                                                 6,
                                                 from,
                                                 direction,
                                                 0,
                                                 string_hash("Car_Combat_Damage_Physics"),
                                                 string_hash{},
                                                 string_hash{},
                                                 false,
                                                 ZEROVEC,
                                                 17,
                                                 false);
                auto *roof_member = static_cast<conglomerate *>(owner)->get_member(string_hash("ROOF"), true);
                decal_morphs::create_decal(
                    string_hash("fx_dcl_bullethole"), roof_member->get_abs_position(), 30.0f, ZEROVEC, owner);
            }
        }
        if (--shots_remaining <= 0)
            reset_timers();
    }
}

traffic_inode::traffic_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[422]);
    _reset();
    initialize(false);
}

traffic_inode::traffic_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), hood(constructor), roof(constructor), trunk(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[422]);
    _reset();
    initialize(false);
}

void traffic_inode::initialize(bool enabled)
{
    my_param_block.set_pb_int(string_hash("car_combat_enabled"), enabled, true);
}

void traffic_inode::_reset()
{
    flags = 0;
    traffic_ptr = nullptr;
    animation_vehicle_type = 0;
    field_C4 = field_C5 = field_C6 = field_C7 = field_C8 = false;
}

bool traffic_inode::car_combat_enabled() const
{
    return my_param_block.get_optional_pb_int(string_hash("car_combat_enabled"), 0, nullptr) != 0;
}

void traffic_inode::determine_vehicle_type_for_anims()
{
    animation_vehicle_type = 0;
    if (traffic_ptr) {
        switch (traffic_ptr->field_C.bodytype) {
        case 1:
            animation_vehicle_type = 0;
            break;
        case 5:
            animation_vehicle_type = 2;
            break;
        case 6:
            animation_vehicle_type = 1;
            break;
        }
    }
}

void traffic_inode::init(int section, int hit_points, float fire_interval, int burst_size, float attack_interval,
                         float damage, float sense_lead_time, float counter_duration)
{
    auto *owner = get_actor();
    if (!owner->has_damage_ifc())
        owner->create_damage_ifc();

    CarCombatInfo *part = nullptr;
    int total_hit_points = 0;
    switch (section) {
    case 0:
        part = &hood;
        total_hit_points = hit_points + roof.hit_points + trunk.hit_points;
        break;
    case 1:
        part = &roof;
        total_hit_points = hit_points + hood.hit_points + trunk.hit_points;
        break;
    case 2:
        part = &trunk;
        total_hit_points = hit_points + hood.hit_points + roof.hit_points;
        break;
    }
    if (part) {
        auto &health = owner->damage_ifc()->field_1FC.field_0;
        health[0] = static_cast<float>(total_hit_points);
        if (health[0] > health[2])
            health[0] = health[2];
        if (health[0] < health[1])
            health[0] = health[1];
        part->section = section;
        part->hit_points = hit_points;
        part->fire_interval = fire_interval;
        part->burst_size = burst_size;
        part->attack_interval = attack_interval;
        part->damage = damage;
        part->sense_lead_time = sense_lead_time;
        part->counter_window = counter_duration + counter_duration;
        part->reset_timers();
    }
    if (owner->has_damage_ifc())
        owner->damage_ifc()->field_1F8 |= 0x8C0;
}
}  // namespace ai
