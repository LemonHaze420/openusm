#include "damage_interface.h"

#include "actor.h"
#include "common.h"
#include "damage_morphs.h"
#include "fe_health_widget.h"
#include "femanager.h"
#include "igofrontend.h"
#include "variables.h"
#include "func_wrapper.h"
#include "parse_generic_mash.h"
#include "resource_key.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "base_ai_core.h"
#include "combat_inode.h"
#include "std_fear_inode.h"
#include "event.h"
#include "event_manager.h"
#include "event_type.h"
#include "input_mgr.h"
#include "rumble_manager.h"
#include "spider_monkey.h"
#include "wds.h"
#include "ai_pedestrian.h"
#include "ai_action_processor_inode.h"
#include "ped_spawner.h"
#include "physical_interface.h"
#include "collision_geometry.h"
#include "conglom.h"
#include "script.h"
#include "script_manager.h"
#include "script_object.h"
#include "vm_executable.h"
#include "sound_and_pfx_interface.h"
#include "resource_manager.h"
#include "region.h"
#include "terrain.h"
#include "memory.h"
#include "oldmath_po.h"
#include <functional>

#include <cmath>

VALIDATE_SIZE(damage_info, 0x40u);

VALIDATE_OFFSET(damage_interface, field_FC, 0xFCu);
VALIDATE_SIZE(damage_interface, 0x23Cu);
VALIDATE_OFFSET(damage_interface, prop_velocity, 0xDCu);
VALIDATE_OFFSET(damage_interface, prop_lifetime, 0xECu);

int damage_interface::find_damageable(const vector3d &position, float radius, unsigned flags, bool restrict_regions)
{
    if (all_damage_interfaces && !all_damage_interfaces->empty()) {
        region_array nearby{};
        region *origin = nullptr;
        if (restrict_regions) {
            origin = g_world_ptr->the_terrain->find_region(position, nullptr);
            build_region_list_radius(&nearby, origin, position, radius, true);
        }
        if (!found_damageable)
            found_damageable = new _std::list<damage_interface *>;
        found_damageable->clear();
        const float radius_squared = radius * radius;
        for (auto *damage : *all_damage_interfaces) {
            auto *owner = damage->field_4;
            if ((flags & 1) && !(damage->field_1FC.field_0[0] > 0.0f))
                continue;
            if ((flags & 2) && !(owner->field_4 & 0x200))
                continue;
            if (!((owner->get_abs_position() - position).length2() <= radius_squared))
                continue;
            if (origin && !owner->is_in_region(origin)) {
                bool connected = false;
                for (int i = 0; i < nearby.count; ++i) {
                    if (owner->is_in_region(nearby.m_data[i])) {
                        connected = true;
                        break;
                    }
                }
                if (!connected)
                    continue;
            }
            found_damageable->push_back(damage);
        }
    }
    return found_damageable ? found_damageable->size() : 0;
}

namespace {

bool damage_nearby_pedestrians(const vector3d &position, float radius, float amount)
{
    const float radius_squared = radius * radius;
    if (std::fabs(amount) > 1.0f)
        amount = 1.0f;
    if (amount > 0.0f)
        amount = -amount;
    const string_hash reaction{"Wounded_Upper"};
    bool affected = false;
    for (auto *spawner : ped_spawner::ped_spawner_list) {
        actor *ped = spawner->get_my_actor();
        if (ped == nullptr)
            continue;
        auto *core = ped->get_ai_core();
        if (spawner->field_5 || !ai::pedestrian_inode::is_a_pedestrian(core))
            continue;
        auto *inode = static_cast<ai::pedestrian_inode *>(core->get_info_node(ai::pedestrian_inode::default_id, true));
        if (inode->field_D1)
            continue;
        const vector3d direction = ped->get_abs_position() - position;
        if (direction.length2() >= radius_squared)
            continue;
        affected = true;
        inode->m_hit_points += amount;
        if (inode->m_hit_points <= 0.0f) {
            inode->m_hit_points = 0.0f;
            inode->field_D1 = true;
        }
        if (auto *combat = core->get_info_node(ai::combat_inode::default_id, false)) {
            using reaction_fn = void(__fastcall *)(
                ai::info_node *, void *, string_hash, string_hash, string_hash, int, entity *, const vector3d &, bool);
            reinterpret_cast<reaction_fn>(get_vfunc(combat->m_vtbl, 0x124))(
                combat, nullptr, reaction, reaction, reaction, 10, nullptr, direction, false);
        }
    }
    return affected;
}


void apply_destruction_radius_damage(const vector3d &position, float inner_radius)
{
    auto *origin_region = g_world_ptr->the_terrain->find_region(position, nullptr);
    if (origin_region == nullptr)
        return;
    constexpr float radius = 10.0f;
    damage_nearby_pedestrians(position, radius, 0.0f);
    region_array regions{};
    build_region_list_radius(&regions, origin_region, position, radius, true);
    const string_hash pain{"PAIN"};
    const string_hash reaction{"Wounded_Upper_Big"};
    const string_hash empty{0};
    for (int index = 0; index < regions.count; ++index) {
        auto *entities = static_cast<_std::list<entity *> *>(regions.m_data[index]->region_entities);
        for (auto *node = entities->m_head->_Prev; node != entities->m_head;) {
            auto *ent = node->_Myval;
            node = node->_Prev;
            if (ent == nullptr || (ent->field_4 & 0x200) == 0)
                continue;
            vector3d direction = ent->get_abs_position() - position;
            const float distance_squared = direction.length2();
            if (distance_squared < inner_radius * inner_radius || distance_squared > radius * radius)
                continue;
            direction.normalize();
            using active_fn = bool(__fastcall *)(entity *);
            if (!ent->has_damage_ifc() || !reinterpret_cast<active_fn>(get_vfunc(ent->m_vtbl, 0x50))(ent))
                continue;
            if (ent->is_hero() && ent->has_sound_and_pfx_ifc())
                ent->sound_and_pfx_ifc()->play_sound_grp(pain, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
            ent->damage_ifc()->apply_damage(
                nullptr, 0.0f, 6, position, direction, 0, reaction, empty, empty, false, ZEROVEC, 17, false);
        }
    }
}

physical_interface *setup_prop_physical_interface(actor *owner)
{
    if (!owner->has_physical_ifc()) {
        owner->create_physical_ifc();
        owner->field_4 |= 0x40;
    }
    return owner->physical_ifc();
}


void setup_prop_physics(actor *owner)
{
    setup_prop_physical_interface(owner);
    if ((owner->field_4 & 4) == 0)
        return;
    auto &members = static_cast<conglomerate *>(owner)->members;
    for (uint16_t index = 0; index < members.m_size; ++index) {
        auto *member = members.at(index);
        if (!member->is_an_actor())
            continue;
        auto *part = static_cast<actor *>(member);
        if (part->colgeom == nullptr || part->colgeom->get_type() != collision_geometry::MESH)
            continue;
        auto *physics = setup_prop_physical_interface(part);
        physics->set_gravity(true);
        physics->m_gravity_multiplier = 2.5f;
        physics->enable(true);
        physics->set_control_parent(nullptr);
        physics->field_185 = false;
        physics->field_184 = false;
        physics->field_170 = 0.0f;
        physics->field_180 = 0.0f;
    }
}

void create_destruction_prop(damage_interface *damage, actor *source)
{
    resource_manager::push_resource_context(source->get_resource_context());
    auto *ent = g_world_ptr->ent_mgr.acquire_entity(damage->field_D8, 0x81);
    resource_manager::pop_resource_context();
    if (ent != nullptr && ent->is_an_actor()) {
        ent->set_abs_po(source->get_abs_po());
        auto *prop = static_cast<actor *>(ent);
        setup_prop_physics(prop);
        if (!prop->physical_ifc()->start_prop_physics(damage->prop_velocity,
                                                      damage->prop_velocity_randomness,
                                                      damage->prop_lifetime,
                                                      physical_interface::PROP_PRIORITY_LOW))
            g_world_ptr->ent_mgr.make_time_limited(prop, Float{0.01f});
    }
}

void run_smoking_script(const string_hash &function, actor *owner)
{
    if (auto *executable = script_manager::find_function_by_name(function)) {
        spawn_thread_for_func(function, executable->owner->instances->_first_element);
        script::push_arg(owner);
        script::exec_thread(true);
    }
}

struct start_prop_physics_action : ai::ai_action_nugget {
    damage_interface *damage;
    ai::ai_core *owner_core;
    float remaining;

    start_prop_physics_action(ai::ai_core *core, damage_interface *damage_owner)
        : ai_action_nugget(core, string_hash{"start_prop_physics_action"}), damage(damage_owner), owner_core(core),
          remaining(damage_owner->field_F8)
    {}

    int frame_advance(Float elapsed) override
    {
        remaining -= elapsed.value;
        if (remaining > 0.0f)
            return 0;
        actor *owner = owner_core->field_64;
        create_destruction_prop(damage, owner);
        run_smoking_script(string_hash{"stop_smoking(entity)"}, owner);
        damage->continue_post_destruction_actions();
        return 1;
    }

protected:
    ~start_prop_physics_action() = default;
};

VALIDATE_SIZE(start_prop_physics_action, 0x1Cu);
}  // namespace

void damage_interface::post_destruction_actions()
{
    damage_nearby_pedestrians(field_4->get_abs_position(), 5.0f, -1000.0f);
    event_manager::raise_event(event::DESTROYED, field_4->get_my_vhandle());
    if ((field_1F8 & 0x40000) != 0)
        return;
    if ((field_1F8 & 2) != 0) {
        po transform{identity_matrix};
        transform.set_position(field_4->get_abs_position());
        auto *effect = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
            field_FC.m_hash, make_unique_entity_id(), transform, mString{}, 0x81, nullptr);
        g_world_ptr->ent_mgr.make_time_limited(effect, Float{field_1C4});
    }
    if ((field_1F8 & 0x10000) != 0) {
        if (auto *core = field_4->get_ai_core()) {
            auto *processor = static_cast<ai::ai_action_processor_inode *>(
                core->get_info_node(ai::ai_action_processor_inode::default_id, false));
            if (processor != nullptr) {
                auto *action = new (mem_alloc(sizeof(start_prop_physics_action))) start_prop_physics_action{core, this};
                processor->add_action(action);
                run_smoking_script(string_hash{"begin_smoking(entity)"}, field_4);
                return;
            }
        }
        create_destruction_prop(this, field_4);
    }
    continue_post_destruction_actions();
}

void damage_interface::continue_post_destruction_actions()
{
    if ((field_1F8 & 4) != 0 && field_1CC != nullptr) {
        const mString function = mString{static_cast<const char *>(field_1CC)} + "(entity)";
        if (find_func_and_spawn_new_thread(field_4, string_hash{function.c_str()})) {
            script::push_arg(field_4);
            script::exec_thread(false);
        }
    }
    if ((field_1F8 & 0x100000) != 0)
        apply_destruction_radius_damage(field_4->get_abs_position(), explosion_inner_radius);
    if ((field_1F8 & 0x40) == 0) {
        field_4->set_visible(false, false);
        if ((field_1F8 & 0x800) == 0)
            field_4->set_collisions_active(false, true);
    }
    if ((field_1F8 & 0x400) != 0)
        field_4->set_collisions_active(false, true);
    if ((field_1F8 & 0x80) == 0)
        field_4->set_active(false);
    if (field_4->has_sound_and_pfx_ifc()) {
        const string_hash sound = field_1C8 == string_hash{} ? string_hash{"EXPLODE"} : field_1C8;
        field_4->sound_and_pfx_ifc()->play_sound_grp(sound, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
    }
    if ((field_1F8 & 0x20000) != 0) {
        if (field_4->has_physical_ifc() && field_4->physical_ifc()->is_prop_physics_running())
            field_4->physical_ifc()->stop_prop_physics(false);
        g_world_ptr->ent_mgr.make_time_limited(field_4, Float{0.01f});
    }
}

template <>
void bounded_variable<float>::sub_48BFB0(const float &a2)
{
    this->field_0[0] = a2;
    if (a2 > this->field_0[2]) {
        this->field_0[0] = this->field_0[2];
    }

    if (this->field_0[0] < this->field_0[1]) {
        this->field_0[0] = this->field_0[1];
    }
}

damage_interface::damage_interface(actor *a2)
{
    THISCALL(0x004DE8A0, this, a2);
}

damage_interface::~damage_interface()
{
    if (!g_generating_vtables) {
        remove_from_dmg_ifc_list();
        if (field_1CC != nullptr)
            ::operator delete[](field_1CC);
        if (field_1D0 != nullptr)
            ::operator delete[](field_1D0);
        field_1CC = nullptr;
        field_1D0 = nullptr;
    }
    field_4 = nullptr;
}

void damage_interface::remove_from_dmg_ifc_list()
{
    for (auto it = all_damage_interfaces->begin(); it != all_damage_interfaces->end(); ++it) {
        if ((*it) == this) {
            all_damage_interfaces->erase(it);
            break;
        }
    }

    if (all_damage_interfaces->empty()) {
        delete all_damage_interfaces;
        all_damage_interfaces = nullptr;
        if (found_damageable != nullptr) {
            delete found_damageable;
            found_damageable = nullptr;
        }
    }
}


bool damage_interface::get_ifc_num(const resource_key &att, float *a3, bool is_log)
{
    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

    if constexpr (!STANDALONE_SYSTEM)
        return (bool)THISCALL(0x004C8C60, this, &att, a3, is_log);
    if (att.get_type() != RESOURCE_KEY_TYPE_IFC_ATTRIBUTE)
        return false;
    const auto percent = [](const bounded_variable<float> &points) {
        const float range = points.field_0[2] - points.field_0[1];
        return range <= 0.0f ? 0.0f : (points.field_0[0] - points.field_0[1]) / range;
    };
    switch (att.m_hash.source_hash_code) {
    case to_hash("HIT_POINTS"):
        *a3 = field_1FC.field_0[0];
        break;
    case to_hash("SUBDUED_POINTS"):
        *a3 = field_21C.field_0[0];
        break;
    case to_hash("KNOCK_DOWN_POINTS"):
        *a3 = static_cast<float>(field_22C.field_0[0]);
        break;
    case to_hash("ARMOR_POINTS"):
        *a3 = field_20C.field_0[0];
        break;
    case to_hash("MAX_HIT_POINTS"):
    case to_hash("MAX_ARMOR_POINTS"):
        *a3 = field_1FC.field_0[2];
        break;
    case to_hash("HIT_POINT_PERCENT"):
        *a3 = percent(field_1FC);
        break;
    case to_hash("ARMOR_POINT_PERCENT"):
        *a3 = percent(field_20C);
        break;
    case to_hash("DAMAGE_MOD"):
        *a3 = field_1D4;
        break;
    case to_hash("TARGET_PRIORITY"):
        *a3 = field_1D8;
        break;
    case to_hash("ALLOW_ARMOR"):
        *a3 = (field_1F8 & 0x4000) == 0;
        break;
    case to_hash("ARMOR_ONLY"):
        *a3 = (field_1F8 & 0x8000) != 0;
        break;
    default:
        return false;
    }
    return true;
}

bool damage_interface::set_ifc_num(const resource_key &att, Float a3, bool is_log)
{
    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

    if constexpr (!STANDALONE_SYSTEM)
        return (bool)THISCALL(0x004CE940, this, &att, a3, is_log);
    if (att.get_type() != RESOURCE_KEY_TYPE_IFC_ATTRIBUTE)
        return false;

    const float value = a3.value;


    const auto set_maximum = [value](bounded_variable<float> &points) {
        points.field_0[2] = value;
        if (value < points.field_0[1])
            std::swap(points.field_0[1], points.field_0[2]);
        points.sub_48BFB0(points.field_0[0]);
    };
    switch (att.m_hash.source_hash_code) {
    case to_hash("HIT_POINTS"):
        field_1FC.sub_48BFB0(value);
        break;
    case to_hash("SUBDUED_POINTS"):
        field_21C.sub_48BFB0(value);
        break;
    case to_hash("KNOCK_DOWN_POINTS"): {
        const auto converted = value >= -9223372036854775808.0f && value < 9223372036854775808.0f
                                   ? static_cast<int64_t>(value)
                                   : int64_t{0};
        field_22C.field_0[0] = bit_cast<int>(static_cast<uint32_t>(converted));
        if (field_22C.field_0[0] > field_22C.field_0[2])
            field_22C.field_0[0] = field_22C.field_0[2];
        if (field_22C.field_0[0] < field_22C.field_0[1])
            field_22C.field_0[0] = field_22C.field_0[1];
        break;
    }
    case to_hash("ARMOR_POINTS"):
        field_20C.sub_48BFB0(value);
        break;
    case to_hash("MAX_HIT_POINTS"):
        set_maximum(field_1FC);
        break;
    case to_hash("MAX_ARMOR_POINTS"):
        set_maximum(field_20C);
        break;
    case to_hash("DAMAGE_MOD"):
        field_1D4 = value;
        break;
    case to_hash("TARGET_PRIORITY"):
        field_1D8 = value;
        break;
    case to_hash("ALLOW_ARMOR"):
        field_1F8 = !(value <= 0.0f && value >= 0.0f) ? field_1F8 & ~0x4000 : field_1F8 | 0x4000;
        break;
    case to_hash("ARMOR_ONLY"):
        field_1F8 = !(value <= 0.0f && value >= 0.0f) ? field_1F8 | 0x8000 : field_1F8 & ~0x8000;
        break;
    case to_hash("IGNORE_EXPLOSIVE_DAMAGE"):
        field_F4 = !(value <= 0.0f && value >= 0.0f);
        break;
    default:
        return false;
    }
    return true;
}

void damage_interface::frame_advance_all_damage_ifc(Float a1)
{
    TRACE("damage_interface::frame_advance_all_damage_ifc");

    if constexpr (1) {
        if (all_damage_interfaces != nullptr && !all_damage_interfaces->empty()) {
            for (auto &dam : (*all_damage_interfaces)) {
                if (dam != nullptr) {
                    if constexpr (STANDALONE_SYSTEM) {
                        dam->frame_advance(a1);
                    } else {
                        void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(dam->m_vtbl, 0x28));
                        func(dam, nullptr, a1);
                    }
                }
            }
        }
    } else {
        CDECL_CALL(0x004D1990, a1);
    }
}

void damage_interface::_un_mash(generic_mash_header *header, void *a3, void *a4, generic_mash_data_ptrs *a5)
{
    TRACE("damage_interface::un_mash");

    if constexpr (STANDALONE_SYSTEM) {
        this->field_4 = static_cast<actor *>(a3);
        this->dynamic = false;
        if (all_damage_interfaces == nullptr) {
            all_damage_interfaces = new _std::vector<damage_interface *>{};
        }
        all_damage_interfaces->push_back(this);
        this->field_1F4 = false;
        this->field_1F5 = false;
        auto read_data = [a5]() -> void * {
            a5->rebase_shared(4u);
            const auto size = *a5->get_from_shared<uint32_t>();
            return a5->get_from_shared<uint8_t>(size);
        };
        this->field_1CC = (this->field_1F8 & 4) != 0 ? read_data() : nullptr;
        this->field_1D0 = (this->field_1F8 & 0x10) != 0 ? read_data() : nullptr;
        this->field_184.un_mash(header, &this->field_184, a5);
        auto read_bounded = [a5](bounded_variable<float> &value) {
            value.field_0[2] = static_cast<float>(*a5->get_from_shared<int>());
            if (value.field_0[2] < value.field_0[1]) {
                std::swap(value.field_0[1], value.field_0[2]);
            }
            value.sub_48BFB0(value.field_0[0]);
            value.sub_48BFB0(static_cast<float>(*a5->get_from_shared<int>()));
        };
        read_bounded(this->field_1FC);
        read_bounded(this->field_20C);
        this->field_1F8 &= ~0xC000;
        this->field_1D4 = 1.0f;
        this->field_1D8 = 1.0f;
        auto read_string = [a5](mString &value) {
            new (&value) mString{};
            a5->rebase_shared(4u);
            const auto size = *a5->get_from_shared<uint32_t>();
            value = reinterpret_cast<const char *>(a5->get_from_shared<uint8_t>(size));
        };
        read_string(this->field_C);
        read_string(this->field_1C);
        read_string(this->field_2C);
        read_string(this->field_3C);
    } else {
        THISCALL(0x004D9E20, this, header, a3, a4, a5);
    }
}

void damage_interface::release_ifc()
{
    this->remove_from_dmg_ifc_list();
    if (this->field_1F4 && this->field_1CC != nullptr) {
        operator delete[](this->field_1CC);
    }

    this->field_1CC = nullptr;

    auto v2 = this->field_1F5;
    if (v2 && this->field_1D0 != nullptr) {
        operator delete[](this->field_1D0);
    }

    this->field_1D0 = nullptr;

    this->field_184.release_mem();
}

void damage_interface::frame_advance(Float a3)
{
    TRACE("damage_interface::frame_advance");

    if constexpr (STANDALONE_SYSTEM) {
        this->field_1F8 = (this->field_1F8 & 0x2000) != 0 ? this->field_1F8 | 0x1000 : this->field_1F8 & ~0x1000;
        this->field_1F8 &= ~0x2000;
        auto advance = [a3](bounded_variable<float> &value) {
            value.sub_48BFB0(a3.value * value.field_0[3] + value.field_0[0]);
        };
        advance(this->field_1FC);
        advance(this->field_20C);
        if (this->field_1FC.field_0[0] > EPSILON) {
            advance(this->field_21C);
        }
        this->field_22C.field_0[0] =
            static_cast<int>(static_cast<double>(this->field_22C.field_0[3]) * a3.value + this->field_22C.field_0[0]);
        if (this->field_22C.field_0[0] > this->field_22C.field_0[2]) {
            this->field_22C.field_0[0] = this->field_22C.field_0[2];
        }
        if (this->field_22C.field_0[0] < this->field_22C.field_0[1]) {
            this->field_22C.field_0[0] = this->field_22C.field_0[1];
        }
        this->field_184.frame_advance(a3);
        this->update_hp_change(a3);
        if ((this->field_1F8 & 0x1000) != 0) {
            this->field_144 = this->field_104;
        }
        damage_morphs::instance_frame_advance(this->field_4);
    } else {
        THISCALL(0x004EC4A0, this, a3);
    }
}

void damage_interface::update_hp_change(Float time_step)
{
    constexpr float hp_epsilon = 0.0001f;
    const float previous_change = field_1E8;

    fe_health_widget *widget =
        field_4->get_player_controller() != nullptr ? g_femanager.IGO->m_hero_health : g_femanager.IGO->m_boss_health;

    if (std::fabs(field_1E8) >= hp_epsilon) {
        const double interpolation = 1.0 - static_cast<double>(field_1E8) / field_1E4;
        const float progress = interpolation < 0.0 ? 0.0f : static_cast<float>(interpolation);
        const double rate = static_cast<double>(field_1EC) + (static_cast<double>(field_1F0) - field_1EC) * progress;
        const double change = time_step.value * rate;
        field_1E8 = static_cast<float>(field_1E8 - change);
        const float health = static_cast<float>(field_1FC.field_0[0] + change);

        if (rate < 0.0) {
            if (!(health <= 0.0f) && !(field_1E8 > -hp_epsilon)) {
                if (widget != nullptr)
                    widget->set_poison_bar_precent(health / field_1FC.field_0[2]);
            } else {
                field_1E8 = 0.0f;
            }
        } else if (health >= field_1FC.field_0[2] || field_1E8 < hp_epsilon) {
            field_1E8 = 0.0f;
        }

        field_1FC.sub_48BFB0(health);
    }

    if (std::fabs(previous_change) >= hp_epsilon && std::fabs(field_1E8) < hp_epsilon && widget != nullptr) {
        widget->set_regen_bar_shown(false);
        widget->set_poison_bar_shown(false);
        widget->set_health_bar_shown(true);
    }
}

void damage_interface_patch()
{
    REDIRECT(0x00558500, damage_interface::frame_advance_all_damage_ifc);

    {
        FUNC_ADDRESS(address, &damage_interface::_un_mash);
        set_vfunc(0x0088398C, address);
    }

    {
        FUNC_ADDRESS(address, &damage_interface::frame_advance);
        set_vfunc(0x00883998, address);
    }
}

void damage_interface::apply_subdue([[maybe_unused]] entity *source, float amount)
{
    field_21C.sub_48BFB0(field_21C.field_0[0] + amount);
}

void damage_interface::apply_damage(entity *source, float amount, int damage_type, const vector3d &position,
                                    const vector3d &direction, int flags, const string_hash &attack,
                                    const string_hash &category, const string_hash &reaction, bool force_reaction,
                                    const vector3d &target, int combo_type, bool skip_combat)
{
    if ((damage_type == 6 && field_F4) || (std::equal_to<float>{}(amount, 0.0f) && damage_type != 7) ||
        (field_4->is_hero() && amount > 0.0f && god_mode_cheat()) || (field_4->field_8 & 0x4000) != 0)
        return;
    ai::ai_core *core = field_4->get_ai_core();
    auto *combat = core != nullptr ? core->get_info_node(ai::combat_inode::default_id, false) : nullptr;
    if (combat != nullptr && !skip_combat) {
        vector3d incoming_direction = ZEROVEC;
        if (damage_type == 2 || damage_type == 6 || source == nullptr) {
            incoming_direction = direction;
            incoming_direction.normalize();
        }
        using incoming_fn = void(__fastcall *)(ai::info_node *,
                                               void *,
                                               string_hash,
                                               string_hash,
                                               string_hash,
                                               int,
                                               entity_base_vhandle,
                                               const vector3d &,
                                               bool);
        reinterpret_cast<incoming_fn>(get_vfunc(combat->m_vtbl, 0x124))(
            combat,
            nullptr,
            attack,
            category,
            reaction,
            combo_type,
            entity_base_vhandle{source != nullptr ? source->my_handle : 0},
            incoming_direction,
            force_reaction);
    }
    float health = field_1FC.field_0[0];
    float armor = field_20C.field_0[0];
    if (source != nullptr && source->is_a_handheld_item()) {
        using owner_fn = entity *(__fastcall *)(entity *, void *);
        entity *owner = reinterpret_cast<owner_fn>(get_vfunc(source->m_vtbl, 0x2DC))(source, nullptr);
        field_104.field_30 = owner->my_handle;
        field_104.field_34 = source->my_handle;
    } else {
        field_104.field_30 = source != nullptr ? source->my_handle : 0;
        field_104.field_34 = 0;
    }
    if (ultra_god_mode_cheat() || mega_god_mode_cheat()) {
        auto *attacker = field_104.field_30.get_volatile_ptr();
        if (attacker != nullptr && attacker->is_hero()) {
            amount = 100000.0f;
            damage_type = 6;
            apply_subdue(static_cast<entity *>(attacker), 100000.0f);
            field_104.field_30 = field_104.field_34 = 0;
        }
    }
    using ignore_fn = bool(__fastcall *)(ai::info_node *, void *, bool);
    field_104.field_3E =
        combat != nullptr && reinterpret_cast<ignore_fn>(get_vfunc(combat->m_vtbl, 0xE4))(combat, nullptr, false);
    field_104.amount = field_104.field_3E ? 0.0f : amount;
    field_104.field_8 = direction;
    field_104.field_C = target;
    field_104.field_4 = position;
    field_104.field_28 = damage_type;
    field_104.field_3C = damage_type != 1;
    field_104.field_3D = true;
    field_104.field_2C = flags;
    field_104.field_38 = attack.source_hash_code;
    if (auto *type = event_manager::get_event_type(event::DAMAGED))
        type->raise_event(entity_base_vhandle{field_4->my_handle}, nullptr);
    const double scaled_damage = static_cast<double>(field_1D4) * field_104.amount + 0.5;
    field_104.amount =
        field_104.amount > 0.0f && scaled_damage <= 1.0 && field_1D4 > 0.0f ? 1.0f : static_cast<float>(scaled_damage);
    if (field_104.amount < 0.0f)
        field_104.amount = 0.0f;
    float fear_fraction = -1.0f;
    auto *fear = core != nullptr && field_104.amount > 0.0f
                     ? static_cast<ai::std_fear_inode *>(core->get_info_node(ai::std_fear_inode::default_id, false))
                     : nullptr;
    if (fear != nullptr) {
        const double total = static_cast<double>(health) + ((field_1F8 & 0x4000) == 0 ? armor : 0.0f);
        fear_fraction = total <= EPSILON ? 0.0f : std::min(static_cast<float>(field_104.amount / total), 1.0f);
    }
    if (field_104.amount > 0.0f ||
        (std::equal_to<float>{}(field_104.amount, 0.0f) && (damage_type == 7 || field_104.field_3E))) {
        if ((field_1F8 & 0x4000) != 0) {
            health -= field_104.amount;
        } else {
            armor -= field_104.amount;
            if (armor < 0.0f) {
                if ((field_1F8 & 0x8000) == 0)
                    health += armor;
                armor = 0.0f;
            }
        }
        if (health <= 0.0f) {
            health = 0.0f;
            if (field_1FC.field_0[0] > 0.0f)
                post_destruction_actions();
        }
        field_1FC.sub_48BFB0(health);
        field_20C.sub_48BFB0(armor);
        if (field_4->is_hero()) {
            if (auto *rumble = input_mgr::instance->rumble_ptr) {
                const float amplitude = field_104.amount / 20.0f + 0.5f;
                const float duration = std::min(field_104.amount / 20.0f + 0.1f, 1.0f);
                if (!rumble->field_5C || rumble->field_1C < amplitude)
                    rumble->start_vibration(amplitude, duration, 0.0f, 0.0f, 1, 0.3f);
            }
        }
        field_1F8 |= 0x2000;
        field_1DC = bit_cast<int>(g_world_ptr->time_manager.field_8);
    }
    if (fear != nullptr && fear_fraction > 0.0f && is_alive() && !is_subdued())
        fear->post_event(0, fear_fraction);
    using notify_fn = void(__fastcall *)(ai::info_node *, void *, float);
    if (combat != nullptr && field_4->is_hero()) {
        reinterpret_cast<notify_fn>(get_vfunc(combat->m_vtbl, 0x74))(combat, nullptr, -field_104.amount);
    } else if (source != nullptr && source->is_hero()) {
        if (auto *attacker_core = source->get_ai_core()) {
            if (auto *attacker_combat = attacker_core->get_info_node(ai::combat_inode::default_id, false))
                reinterpret_cast<notify_fn>(get_vfunc(attacker_combat->m_vtbl, 0x74))(
                    attacker_combat, nullptr, field_104.amount);
        }
    }
}
