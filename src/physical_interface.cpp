#include "physical_interface.h"

#include "actor.h"
#include "biped_system.h"
#include "base_ai_core.h"
#include "sound_and_pfx_interface.h"
#include "common.h"
#include "conglom.h"
#include "entity_mash.h"
#include "func_wrapper.h"
#include "guidance_sys.h"
#include "log.h"
#include "oldmath_po.h"
#include "moved_entities.h"

#include "pendulum.h"
#include "phys_vector3d.h"
#include "rb_ragdoll_model.h"
#include "rigid_body.h"
#include "trace.h"
#include "time_interface.h"
#include "parse_generic_mash.h"
#include "resource_key.h"
#include "terrain_types_manager.h"
#include "subdivision_obb.h"
#include "terrain.h"

#include "utility.h"
#include "variables.h"
#include "vector3d.h"
#include "vtbl.h"
#include "wds.h"
#include "prop_system.h"
#include "event.h"
#include "event_manager.h"
#include "physics_system.h"
#include "local_collision.h"
#include "collision_geometry.h"
#include "event_type.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"
#include <algorithm>

#include <cassert>
#include <cmath>
#include <functional>

VALIDATE_OFFSET(physical_interface, field_164, 0x164);
VALIDATE_SIZE(physical_interface, 0x1B0);

std::reference_wrapper<int[512]> physical_interface::rotators = var<int[512]>(0x0095AF78);

int &physical_interface::rotators_num = var<int>(0x0095A6B8);


physical_interface::physical_interface(actor *a2) : field_188(), field_198()
{
#if STANDALONE_SYSTEM
    construct_v_table_lookup();
    m_vtbl = ifc_v_table_lookup[2];
#else
    m_vtbl = 0x00883A44;
#endif
    this->field_4 = a2;
    this->dynamic = true;
    this->field_C = 0;
    this->field_84 = {};
    this->field_88 = {};
    this->field_A4 = 0;
    this->field_D8 = 0;
    this->field_124 = 0;
    this->field_128 = 0;
    this->field_12C = 0;
    this->field_130 = 0;
    this->field_134 = 0;
    this->field_138 = 0;
    this->field_13C = 0;
    this->field_140 = 0;
    this->field_144 = 0;
    this->field_148 = 0;
    this->field_14C = 0;
    this->field_150 = 0;
    this->field_154 = 0;
    this->field_158 = 0;
    this->field_17C = 0;
    this->field_17D = 0;
    this->field_184 = 0;
    this->field_185 = 0;
    this->field_186 = 0;

    if (!g_generating_vtables) {
        this->add_to_phys_ifc_list();
        this->field_174 = nullptr;
        this->m_bp_sys = nullptr;
        this->field_10 = 0.0;
        this->field_14 = 0.0;
        this->field_18 = 0;
        this->field_1C = 0;

        this->m_velocity = ZEROVEC;
        this->field_2C = ZEROVEC;
        this->field_38 = ZEROVEC;

        this->field_74 = -YVEC;

        this->field_44 = ZEROVEC;
        this->field_80 = 0;
        this->field_90 = 0.5;
        this->field_8C = 0.64999998;
        this->field_94 = 0.025;
        this->field_EC = 0.55000001;
        this->field_15C = 0.0;
        this->field_160 = 0.0;
        this->field_E8 = nullptr;
        this->field_164 = 1.0;
        this->field_168 = 1.0;
        this->field_16C = 1.0;
        this->m_gravity_multiplier = 1.0;
        this->field_17C = 1;
        this->field_F0 = -10.0;
        this->field_F4 = 0;
        this->ground_elevation = 0.0f;
        this->field_F8 = 0;
        this->field_184 = 0;
        this->field_180 = g_world_ptr->time_manager.field_8;
        this->field_100 = YVEC;
        this->field_10C = 0;
        this->field_C8 = 1.0;
        this->field_CC = -1.0;
        this->field_17D = 0;

        this->field_110[0] = nullptr;
        this->field_110[1] = nullptr;
        this->field_110[2] = nullptr;
        this->field_110[3] = nullptr;
        this->field_110[4] = nullptr;

        this->field_C |= 0x1000u;
        this->field_9C = 1.0;
        this->field_A0 = 1.0;
        this->field_A4 = 0;
        this->field_170 = 0;
        this->field_D8 = 0;
        this->field_D4 = 1.0;
        this->field_98 = 0.0;
        this->field_1AC = 1.0;

        this->field_188 = "";
        this->field_198 = "";
        this->field_DC = 0;
        this->field_E0 = 0;
        this->field_E4 = 10.0;
        this->field_1A8 = 0;
    }
}

pendulum *physical_interface::get_pendulum(int num)
{
    assert(num >= 0 && num < PHYS_IFC_MAX_PENDULUM_CONSTRAINTS);
    return this->field_110[num];
}

void physical_interface::get_parent_terrain_type(string_hash *out)
{
    if (!field_184) {
        *out = string_hash{"INVALID"};
    } else if (auto *parent = field_84.get_volatile_ptr()) {
        *out = terrain_types_manager::get_terrain_type_by_index(static_cast<unsigned char>(parent->field_41));
    } else {
        *out = field_88;
    }
}

bool physical_interface::set_ifc_num(const resource_key &key, Float value, bool log)
{
    if constexpr (!STANDALONE_SYSTEM)
        return (bool)THISCALL(0x004C9500, this, &key, value, log);
    if (key.get_type() != RESOURCE_KEY_TYPE_IFC_ATTRIBUTE)
        return false;
    const bool nonzero = !(value <= 0.0f && value >= 0.0f);
    const auto set_flag = [this, nonzero](uint32_t flag) {
        field_C = nonzero ? field_C | flag : field_C & ~flag;
    };
    switch (key.m_hash.source_hash_code) {
    case to_hash("MASS"): field_10 = value; break;
    case to_hash("VOLUME"): field_14 = value; break;
    case to_hash("SLIDE_FACTOR"): field_8C = value; break;
    case to_hash("BOUNCE_FACTOR"): field_90 = value; break;
    case to_hash("STICKY_OFFSET"): field_94 = value; break;
    case to_hash("DRAG_COEFFICIENT"): field_15C = value; break;
    case to_hash("DRAG_COEFFICIENT_MIN_SPEED"): field_160 = value; break;
    case to_hash("DRAG_COEFFICIENT_HORZ_SCALE"): field_164 = value; break;
    case to_hash("DRAG_COEFFICIENT_UP_SCALE"): field_168 = value; break;
    case to_hash("DRAG_COEFFICIENT_DOWN_SCALE"): field_16C = value; break;
    case to_hash("FRICTION_SCALE"): field_98 = value; break;
    case to_hash("GAME_PHYSICS_TIME_DILATION"): field_1AC = value; break;
    case to_hash("BOUNCY"): set_flag(0x10000000u); break;
    case to_hash("STICKY"): set_flag(0x20000000u); break;
    case to_hash("ENABLED"): enable(nonzero); break;
    case to_hash("SUSPENDED"): suspend(nonzero); break;
    case to_hash("GRAVITY"): set_gravity(nonzero); break;
    case to_hash("GRAVITY_MULTIPLIER"): m_gravity_multiplier = value; break;
    case to_hash("STICKY_ORIENT"): set_flag(0x40000000u); break;
    case to_hash("USE_CHAR_VEL_PHYSICS"): set_flag(0x10000u); break;
    case to_hash("USE_PENDULUM_ORIENTATION"): set_flag(0x20000u); break;
    case to_hash("IMMOBILE"): set_flag(0x100u); break;
    case to_hash("ALLOW_MANAGE_STANDING"): set_allow_manage_standing(nonzero); break;
    case to_hash("FLOOR_OFFSET_SCALE"): field_C8 = value; break;
    case to_hash("FLOOR_OFFSET_USE_RENDER_SCALE"): field_17D = static_cast<int>(value) != 0; break;
    default: return false;
    }
    return true;
}

bool physical_interface::get_ifc_num(const resource_key &key, float &value, bool log)
{
    if constexpr (!STANDALONE_SYSTEM)
        return (bool)THISCALL(0x004BD160, this, &key, &value, log);
    if (key.get_type() != RESOURCE_KEY_TYPE_IFC_ATTRIBUTE)
        return false;
    switch (key.m_hash.source_hash_code) {
    case to_hash("MASS"): value = field_10; break;
    case to_hash("VOLUME"): value = field_14; break;
    case to_hash("SLIDE_FACTOR"): value = field_8C; break;
    case to_hash("BOUNCE_FACTOR"): value = field_90; break;
    case to_hash("STICKY_OFFSET"): value = field_94; break;
    case to_hash("DRAG_COEFFICIENT"): value = field_15C; break;
    case to_hash("DRAG_COEFFICIENT_MIN_SPEED"): value = field_160; break;
    case to_hash("DRAG_COEFFICIENT_HORZ_SCALE"): value = field_164; break;
    case to_hash("DRAG_COEFFICIENT_UP_SCALE"): value = field_168; break;
    case to_hash("DRAG_COEFFICIENT_DOWN_SCALE"): value = field_16C; break;
    case to_hash("FRICTION_SCALE"): value = field_98; break;
    case to_hash("GAME_PHYSICS_TIME_DILATION"): value = field_1AC; break;
    case to_hash("BOUNCY"): value = (field_C & 0x10000000u) != 0; break;
    case to_hash("STICKY"): value = (field_C & 0x20000000u) != 0; break;
    case to_hash("ENABLED"): value = (field_C & 1u) != 0; break;
    case to_hash("SUSPENDED"): value = (field_C & 2u) != 0; break;
    case to_hash("GRAVITY"): value = (field_C & 4u) != 0; break;
    case to_hash("GRAVITY_MULTIPLIER"): value = m_gravity_multiplier; break;
    case to_hash("HAS_BOUNCED"): value = (field_C & 0x80u) != 0; break;
    case to_hash("IS_STUCK"): value = (field_C & 0x80000000u) != 0; break;
    case to_hash("STICKY_ORIENT"): value = (field_C & 0x40000000u) != 0; break;
    case to_hash("USE_CHAR_VEL_PHYSICS"): value = (field_C & 0x10000u) != 0; break;
    case to_hash("USE_PENDULUM_ORIENTATION"): value = (field_C & 0x20000u) != 0; break;
    case to_hash("ALLOW_MANAGE_STANDING"): value = (field_C & 0x1000u) != 0; break;
    case to_hash("FLOOR_OFFSET_SCALE"): value = field_C8; break;
    case to_hash("FLOOR_OFFSET_USE_RENDER_SCALE"): value = field_17D != 0; break;
    default: return false;
    }
    return true;
}

bool physical_interface::get_ifc_vec(const resource_key &key, vector3d &value, bool log)
{
    if constexpr (!STANDALONE_SYSTEM)
        return (bool)THISCALL(0x004CEEC0, this, &key, &value, log);
    if (key.get_type() != RESOURCE_KEY_TYPE_IFC_ATTRIBUTE)
        return false;
    switch (key.m_hash.source_hash_code) {
    case to_hash("VELOCITY"): value = get_velocity(); break;
    case to_hash("ANGULAR_VELOCITY"): value = field_2C; break;
    case to_hash("BOUNCE_POS"): value = field_A8; break;
    case to_hash("BOUNCE_NORM"): value = field_B4; break;
    case to_hash("GRAVITY_VECTOR"): value = field_74; break;
    default: return false;
    }
    return true;
}

bool physical_interface::set_ifc_vec(const resource_key &key, const vector3d &value, bool log)
{
    if constexpr (!STANDALONE_SYSTEM)
        return (bool)THISCALL(0x004CF0A0, this, &key, &value, log);
    if (key.get_type() != RESOURCE_KEY_TYPE_IFC_ATTRIBUTE)
        return false;
    switch (key.m_hash.source_hash_code) {
    case to_hash("VELOCITY"): set_velocity(value, false); break;
    case to_hash("ANGULAR_VELOCITY"): field_2C = value; break;
    case to_hash("GRAVITY_VECTOR"): field_74 = value; break;
    default: return false;
    }
    return true;
}

void physical_interface::add_to_phys_ifc_list()
{
    if (all_phys_interfaces == nullptr)
        all_phys_interfaces = new _std::vector<physical_interface *>;
    all_phys_interfaces->push_back(this);
}

bool physical_interface::is_biped_physics_running() const
{
    return 0x80000 & this->field_C;
}

biped_system *physical_interface::get_biped_system()
{
    if (!this->is_biped_physics_running()) {
        return nullptr;
    }

    assert(m_bp_sys != nullptr);

    return this->m_bp_sys;
}

bool sub_5019B0(float *a1, float *a2, Float a3, Float a4, Float a5)
{
    if (equal(float{a3}, 0.0f)) {
        return false;
    }

    auto v5 = a4 * a4 - a3 * a5 * 4.0f;
    if (v5 < 0.0f) {
        return false;
    }

    auto v6 = std::sqrt(v5);
    auto v7 = a3 + a3;
    *a1 = (-a4 - v6) / v7;
    *a2 = (v6 - a4) / v7;
    return true;
}

void physical_interface::un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5)
{
    if constexpr (STANDALONE_SYSTEM) {
        this->field_4 = static_cast<actor *>(a3);
        this->dynamic = false;
        this->field_C = *a5->get_from_shared<uint32_t>();
        this->add_to_phys_ifc_list();
        this->field_174 = nullptr;
        this->m_bp_sys = nullptr;
        this->m_velocity = ZEROVEC;
        this->field_2C = ZEROVEC;
        this->field_38 = ZEROVEC;
        this->field_44 = ZEROVEC;
        this->field_50 = ZEROVEC;
        this->field_5C = ZEROVEC;
        this->field_68 = ZEROVEC;
        this->field_9C = 1.0f;
        this->field_A0 = 1.0f;
        this->field_E8 = nullptr;
        this->field_84 = {};
        this->field_88 = string_hash{-1};
        this->field_A8 = ZEROVEC;
        this->field_B4 = ZEROVEC;
        this->field_C0 = 0;
        this->field_C4 = 0;
        this->field_F4 = 0;
        this->field_F8 = 0;
        this->ground_elevation = 0.0f;
        this->field_100 = YVEC;
        this->field_10C = 0.0f;
        this->field_CC = -1.0f;
        for (auto &constraint : this->field_110) {
            constraint = nullptr;
        }
        this->field_148 = 0;
        this->field_158 = false;
        this->field_170 = 0;
        this->field_A4 = 0;
        this->field_D4 = 1.0f;
        this->field_1A8 = 0;
        auto read_string = [a5](mString &value) {
            new (&value) mString{};
            a5->rebase_shared(4u);
            const auto size = *a5->get_from_shared<uint32_t>();
            value = reinterpret_cast<const char *>(
                a5->get_from_shared<uint8_t>(size));
        };
        read_string(this->field_188);
        read_string(this->field_198);
    } else {
        THISCALL(0x004DF4A0, this, a2, a3, a4, a5);
    }
}

void physical_interface::frame_advance_rotators(Float elapsed)
{
    const auto cosine = [](float angle) {
        const float phase = -std::fabs(angle) * 0.15915493667125702f;
        const float t = std::fabs(std::ceil(phase) - phase - 0.5f) - 0.25f;
        const float squared = t * t;
        const float cubed = squared * t;
        const float fourth = squared * squared;
        const float fifth = fourth * t;
        float result = fifth * fourth * 39.71065902709961f;
        result += cubed * fourth * -76.57495880126953f;
        result += fifth * 81.60222625732422f;
        result += cubed * -41.3416748046875f;
        return result + t * 6.283185005187988f;
    };
    for (int index = 0; index < rotators_num; ++index) {
        auto *entry = &rotators.get()[index * 4];
        auto *owner = reinterpret_cast<vhandle_type<entity> *>(entry + 3)->get_volatile_ptr();
        if (owner == nullptr) {
            --rotators_num;
            if (index < rotators_num)
                std::copy_n(&rotators.get()[rotators_num * 4], 4, entry);
            continue;
        }
        vector3d axis{bit_cast<float>(entry[0]), bit_cast<float>(entry[1]), bit_cast<float>(entry[2])};
        const float angle = -elapsed.value * axis.length();
        axis.normalize();
        const float sine = cosine(angle + 4.71238899230957f);
        const float cos = cosine(angle);
        const float complement = 1.0f - cos;
        po rotation;
        rotation.m[0] = vector4d{
            axis.x * axis.x * complement + cos,
            axis.x * axis.y * complement + axis.z * sine,
            axis.x * axis.z * complement - axis.y * sine, 0.0f};
        rotation.m[1] = vector4d{
            axis.x * axis.y * complement - axis.z * sine,
            axis.y * axis.y * complement + cos,
            axis.y * axis.z * complement + axis.x * sine, 0.0f};
        rotation.m[2] = vector4d{
            axis.x * axis.z * complement + axis.y * sine,
            axis.y * axis.z * complement - axis.x * sine,
            axis.z * axis.z * complement + cos, 0.0f};
        auto &relative = owner->get_rel_po();
        const auto position = relative.get_position();
        relative.set_from_ptr_to_po_world(ptr_to_po{&rotation.m, &relative.m});
        relative.set_position(position);
        relative.sub_48D840();
        owner->field_8 |= 0x10000040u;
        for (auto *child = owner->m_child; child != nullptr; child = child->field_28)
            if ((child->field_8 & 0x10000000u) == 0)
                child->dirty_family(false);
        if ((owner->field_4 & 0x8004u) != 0)
            owner->dirty_model_po_family();
        owner->po_changed();
    }
}

void physical_interface::frame_advance_all_phys_interfaces(Float elapsed)
{
    frame_advance_rotators(elapsed);
    if (all_phys_interfaces == nullptr || all_phys_interfaces->empty())
        return;
    biped_system::process_biped_physics(elapsed);
    stack_allocator saved;
    scratchpad_stack::save_state(&saved);
    const auto capacity = all_phys_interfaces->size();
    auto **interfaces = static_cast<physical_interface **>(
        scratchpad_stack::alloc(capacity * sizeof(physical_interface *)));
    auto *steps = static_cast<float *>(scratchpad_stack::alloc(capacity * sizeof(float)));
    int count = 0;
    for (auto *physical : *all_phys_interfaces) {
        auto *owner = physical->field_4;
        if ((owner->is_in_limbo() || owner->get_primary_region() == nullptr) &&
            (owner->field_4 & 8u) == 0)
            continue;
        interfaces[count] = physical;
        const float scale = owner->field_58 != nullptr
            ? static_cast<float>(owner->field_58->sub_4ADE50())
            : g_world_ptr->time_manager.field_0;
        steps[count++] = scale * elapsed.value;
    }
    for (int index = 0; index < count; ++index) {
        auto *physical = interfaces[index];
        physical->field_C &= ~0x60u;
        physical->field_C4 = 0;
        if (physical->field_4->colgeom != nullptr)
            physical->field_4->colgeom->field_8 = false;
    }
    for (int index = 0; index < count; ++index) {
        auto *physical = interfaces[index];
        physical->field_D8 = std::max(0.0f, physical->field_D8 - steps[index]);
        physical->field_A4 = std::max(0.0f, physical->field_A4 - elapsed.value);
        if ((physical->field_C & 0x80000u) != 0 && physical->get_velocity().length2() < 0.6f)
            physical->field_170 += elapsed.value;
        else
            physical->field_170 = 0.0f;
    }
    for (int index = 0; index < count; ++index) {
        auto *physical = interfaces[index];
        if ((physical->field_C & 1u) == 0 || (physical->field_C & 2u) != 0 ||
            physical->field_174 != nullptr)
            continue;
        const Float step{steps[index]};
        physical->frame_advance(step);
        if ((physical->field_C & 0x80000u) == 0) {
            const po start = physical->field_4->get_abs_po();
            const auto velocity = physical->get_velocity();
            po result;
            vector3d result_velocity;
            physical->integrate(step, start, velocity, result, result_velocity);
            physical->backpropagate(step, result, result_velocity);
        }
    }
    for (int index = 0; index < count; ++index) {
        auto *physical = interfaces[index];
        if ((physical->field_C & 0x80000u) != 0) {
            if (auto *core = physical->field_4->get_ai_core())
                core->adjust_colgeom(true);
            auto *biped = physical->m_bp_sys;
            if (biped->contact) {
                physical->field_C |= 0x60u;
                physical->field_C4 = 0;
                physical->field_5C = biped->contact_normal;
                physical->field_68 = biped->contact_position;
            }
        }
        physical->field_C &= ~0x8000u;
    }
    prop_system::frame_advance(elapsed);
    scratchpad_stack::restore_state(saved);
}

void physical_interface::frame_advance(Float elapsed)
{
    field_88 = string_hash{-1};
    auto *owner = field_4;
    if (owner == nullptr || owner->get_primary_region() == nullptr) {
        return;
    }
    if (field_E8 != nullptr && (field_C & 0x80000000u) == 0) {
        auto advance = reinterpret_cast<void(__fastcall *)(rocket_guidance_sys *, void *, Float)>(
            get_vfunc(field_E8->m_vtbl, 8));
        advance(field_E8, nullptr, elapsed);
    }
    field_C &= ~0x80u;
    owner->update_abs_po(true);
    auto position = owner->get_abs_position();
    if (position.y < -1000.0f) {
        if ((field_C & 0x80000u) != 0)
            stop_biped_physics(false);
        if (field_174 != nullptr)
            stop_prop_physics(false);
        cancel_all_velocity();
        position.y = -999.0f;
        entity_set_abs_position(owner, position);
        return;
    }
    if ((field_C & 0x80000u) != 0 || field_174 != nullptr) {
        owner->set_allow_tunnelling_into_next_frame(true);
        owner->invalidate_frame_delta();
    } else {
        moved_entities::add_moved(vhandle_type<entity>{owner->get_my_vhandle()});
        if ((field_C & 0x20000u) != 0)
            frame_advance_pendulum_orientation(elapsed);
    }
}

void physical_interface::frame_advance_pendulum_orientation(Float)
{
    for (auto *constraint : field_110) {
        if (constraint == nullptr || !constraint->m_active)
            continue;
        auto *anchor = constraint->get_volatile_ptr();
        if (anchor == nullptr)
            continue;
        if (anchor->is_conglom_member() && anchor->m_parent != nullptr)
            anchor = anchor->get_conglom_owner();
        po transform;
        transform.set_po(anchor->get_abs_po().get_z_facing(), YVEC,
            field_4->get_abs_position());
        entity_set_abs_po(field_4, transform);
        return;
    }
}

bool physical_interface::integrate(Float elapsed, const po &start,
    const vector3d &velocity, po &result, vector3d &result_velocity)
{
    const float step = elapsed.value * field_1AC;
    result = start;
    if ((field_4->field_4 & 0x40u) == 0) {
        result_velocity = velocity;
        return false;
    }
    result_velocity = velocity + field_38;
    auto position = start.get_position() + (result_velocity - field_44) * step;
    field_C &= ~0x200000u;
    position = apply_positional_constraints(Float{step}, position, true);
    result.set_position(position);
    if (step > EPSILON)
        result_velocity = (position - start.get_position()) / step + field_44;
    if ((field_C & 0x30000000u) != 0) {
        const po predicted = result;
        const auto predicted_velocity = result_velocity;
        process_projectile_collision(Float{step}, start, velocity, predicted,
            predicted_velocity, result, result_velocity);
    }
    return true;
}

void physical_interface::process_projectile_collision(Float,
    const po &start, const vector3d &velocity, const po &predicted,
    const vector3d &predicted_velocity, po &result, vector3d &result_velocity)
{
    result = predicted;
    result_velocity = predicted_velocity;
    if ((field_C & 0x80000000u) != 0) {
        auto *parent = field_4->m_parent;
        if (parent != nullptr && parent->is_an_entity() &&
            !parent->is_alive() && field_17C) {
            field_4->set_parent(nullptr);
            result_velocity = ZEROVEC;
            set_gravity(true);
            field_2C = ZEROVEC;
            field_C &= ~0x80000000u;
        }
        parent = field_4->m_parent;
        field_17C = parent == nullptr || !parent->is_an_entity() || parent->is_alive();
        return;
    }
    auto direction = predicted.get_position() - start.get_position();
    direction.normalize();
    vector3d point{}, normal{};
    entity *hit = nullptr;
    const bool collisions = field_4->are_collisions_active();
    field_4->set_collisions_active(false, false);
    float radius = field_4->get_visual_radius();
    if (radius < EPSILON && field_4->m_child != nullptr &&
        field_4->m_child->is_an_actor())
        radius = field_4->m_child->get_visual_radius();
    const float extension = std::max(0.1f, radius * 0.75f);
    const bool recorded_contact = field_4->colgeom != nullptr && (field_C & 0x60u) != 0;
    if (recorded_contact ||
        find_intersection(start.get_position() - direction * 0.1f,
            predicted.get_position() + direction * extension,
            *local_collision::entfilter_blocks_beams, *local_collision::obbfilter_lineseg_test,
            &point, &normal, nullptr, &hit, nullptr, false)) {
        if (recorded_contact) {
            normal = field_5C;
            point = field_68;
            hit = reinterpret_cast<entity *>(field_C4);
        }
        normal.normalize();
        auto incoming = velocity;
        if (dot(incoming, direction) < EPSILON)
            incoming = ZEROVEC;
        bounce_internal(point, normal, hit, start, incoming, result, result_velocity);
    }
    field_4->set_collisions_active(collisions, false);
}

void physical_interface::apply_air_resistance(Float elapsed, vector3d &velocity)
{
    auto direction = -velocity;
    const float squared_speed = direction.length2();
    if (squared_speed > 1.0e-10f)
        direction *= 1.0f / std::sqrt(squared_speed);
    const float speed = velocity.length();
    const float excess = speed - field_160;
    const auto drag_velocity = excess < 0.01f || speed <= EPSILON
        ? ZEROVEC : velocity * (excess / speed);
    const float horizontal = std::sqrt(drag_velocity.x * drag_velocity.x +
        drag_velocity.z * drag_velocity.z);
    float coefficient = 0.0f;
    if (speed > 0.001f) {
        const float vertical_coefficient = drag_velocity.y > 0.0f ? field_168 : field_16C;
        coefficient = vertical_coefficient * (1.0f - horizontal / speed) +
            horizontal / speed * field_164;
    }
    const auto candidate = velocity + direction *
        (drag_velocity.length2() * field_15C * coefficient * elapsed.value);
    velocity = dot(candidate, velocity) >= 0.0f ? candidate : ZEROVEC;
}

void physical_interface::backpropagate(Float elapsed, const po &result,
    const vector3d &velocity)
{
    const float step = elapsed.value * field_1AC;
    const auto angular_velocity = field_2C;
    if ((field_4->field_4 & 0x40u) == 0) {
        set_velocity(ZEROVEC, false);
        field_2C = ZEROVEC;
    } else {
        auto final_velocity = velocity;
        apply_air_resistance(Float{step}, final_velocity);
        set_velocity(final_velocity, false);
        po rotation;
        rotation.set_rot(angular_velocity * step);
        po transform = result;
        transform.set_from_ptr_to_po_world(ptr_to_po{&rotation.m, &result.m});
        transform.set_position(result.get_position());
        if (field_4->m_parent != nullptr)
            transform.set_from_ptr_to_po_world(ptr_to_po{
                &field_4->get_rel_po().m, &field_4->m_parent->get_abs_po().m});
        transform.sub_48D840();
        entity_set_abs_po(field_4, transform);
        moved_entities::add_moved(vhandle_type<entity>{field_4->get_my_vhandle()});
    }
    bool moving_anchor = false;
    for (auto *constraint : field_110)
        if (constraint != nullptr && constraint->m_active && constraint->has_a_moving_anchor())
            moving_anchor = true;
    if (((field_C & 0x20u) != 0 || moving_anchor) && field_4->is_hero())
        synchronize_pendulum_constraints_with_position();
    field_50 = field_44;
    field_44 = field_38 = ZEROVEC;
    if ((field_C & 0x100000u) != 0) {
        field_18 = std::max(field_18, m_velocity.length());
        field_1C = std::max(field_1C,
            std::sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z));
    }
}

void physical_interface::bounce_internal(const vector3d &point,
    const vector3d &normal, entity *hit, const po &start,
    const vector3d &velocity, po &result, vector3d &result_velocity)
{
    result_velocity = velocity;
    field_A8 = point;
    field_B4 = normal;
    field_C0 = reinterpret_cast<int>(hit);
    if ((field_C & 0x20000000u) != 0) {
        result_velocity = ZEROVEC;
        set_gravity(false);
        field_2C = ZEROVEC;
        if ((field_C & 0x40000000u) != 0) {
            result = po_identity_matrix;
            result.set_facing(normal);
            result.set_position(point + normal * field_94);
        } else {
            result.set_position(point - start.get_z_facing() * field_94);
        }
        if (hit != nullptr) {
            field_4->set_parent(hit);
            result = start;
            field_17C = hit->is_alive();
        }
        field_C |= 0x80000000u;
    } else if ((field_C & 0x10000000u) != 0) {
        const float speed = velocity.length();
        const float radius = field_4->get_visual_radius();
        const float offset = radius <= 0.0f ? 0.1f : radius * 0.75f;
        if (speed <= EPSILON) {
            result.set_position(point + normal * offset);
        } else {
            const auto incoming = -velocity * (1.0f / speed);
            result.set_position(point + incoming * offset);
            const auto reflected =
                normal * (2.0f * dot(incoming, normal) / normal.length2()) - incoming;
            result_velocity = vector3d{reflected.x * speed * field_8C,
                reflected.y * speed * field_90, reflected.z * speed * field_8C};
        }
    }
    field_C |= 0x80u;
    if (auto *type = event_manager::get_event_type(event::BOUNCED))
        type->raise_event(field_4->get_my_vhandle(), nullptr);
}

void physical_interface::bounce(Float elapsed, const vector3d &point,
    const vector3d &normal, entity *hit)
{
    po result;
    vector3d velocity;
    bounce_internal(point, normal, hit, field_4->get_abs_po(),
        get_velocity(), result, velocity);
    backpropagate(Float{elapsed.value * field_1AC}, result, velocity);
}

vector3d physical_interface::calculate_force_vector_2(const vector3d *a2, const vector3d *a3, Float a4, Float a5)
{
    static const Var<float> g_gravity = (0x00921E3C);

    vector3d v21 = (*a3) - (*a2);

    a5.value *= g_gravity();
    float v20 = v21[0] * v21[0] + v21[2] * v21[2];
    float v5 = v21[1] * v21[1] + v20;
    float v6 = v21[1] * a5 - a4 * a4;
    float v7 = a5 * a5 * 0.25f;

    float a2_1;
    float a3_1;
    if (sub_5019B0(&a2_1, &a3_1, v7, v6, v5)) {
        bool v8 = (a2_1 <= 0.0 || a3_1 <= 0.0) ? (a2_1 < a3_1) : (a3_1 < a2_1);

        float *v9 = &a3_1;
        if (!v8) {
            v9 = &a2_1;
        }

        auto v10 = std::sqrt(*v9);
        auto v11 = 1.0f / v10;

        v21[0] = v21[0] * v11;
        v21[1] = (v10 * v10 * a5 * 0.5f + v21[1]) * v11;
        v21[2] = v21[2] * v11;

    } else {
        auto v17 = std::sqrt(v20);
        if (v17 > std::abs(v21[1])) {
            v21[1] = v17;
        }
    }

    auto v15 = v21.length2();
    if (v15 > 9.9999994e-11) {
        auto v16 = 1.0f / std::sqrt(v15);
        v21 = v21 * v16;
    }

    vector3d result = v21 * a4;

    return result;
}

void physical_interface::cancel_all_velocity()
{
    set_velocity(ZEROVEC, false);
    field_2C = field_38 = field_44 = field_50 = ZEROVEC;
}

bool physical_interface::is_effectively_standing()
{
    return this->field_184;
}

void physical_interface::set_velocity(const vector3d &new_velocity, bool a3)
{
    if constexpr (1) {

        if (this->field_C & 0x80000) {
            biped_system *bp_sys = this->m_bp_sys;

            assert(bp_sys != nullptr);

            if (new_velocity.length() >= EPSILON) {
                if (a3) {
                    auto *v28 = bp_sys->field_0.m_list_rigid_body.m_data[0];

                    vector3d v29 = v28->sub_503B80();

                    phys_vector3d v49;
                    v49[0] = new_velocity[0] - (v28->field_130 * v29[0] + v28->field_D0[0]);
                    v49[1] = new_velocity[1] - (v28->field_130 * v29[1] + v28->field_D0[1]);
                    v49[2] = new_velocity[2] - (v28->field_130 * v29[2] + v28->field_D0[2]);
                    bp_sys->field_0.sub_4ADEF0(0, v49);
                } else {
                    for (int i = 0; i < 10; ++i) {
                        assert(i >= 0 && i < bp_sys->field_0.m_list_rigid_body.m_alloc_count);

                        auto *v35 = bp_sys->field_0.m_list_rigid_body.m_data[i];
                        auto v36 = v35->field_130;

                        auto v42 = 1.0f / v35->field_130;

                        phys_vector3d a3a;
                        a3a.field_0[0] = (new_velocity[0] - (v35->field_110[0] * v36 + v35->field_D0[0])) * v42;
                        a3a.field_0[1] = (new_velocity[1] - (v35->field_110[1] * v36 + v35->field_D0[1])) * v42;
                        a3a.field_0[2] = (new_velocity[2] - (v35->field_110[2] * v36 + v35->field_D0[2])) * v42;
                        bp_sys->field_0.apply_pulse(i, a3a);
                    }
                }
            } else if (a3) {
                auto *v4 = bp_sys->field_0.m_list_rigid_body.m_data[0];
                v4->sub_502600(ZEROVEC);
                v4->sub_502640(ZEROVEC);
            } else {
                for (int i = 0; i < 10; ++i) {
                    auto *v5 = bp_sys->field_0.m_list_rigid_body.m_data[i];

                    v5->sub_502600(ZEROVEC);
                    v5->sub_502640(ZEROVEC);
                }
            }
        } else {
            auto *prop = this->field_174;
            if (prop != nullptr) {
                prop->body->sub_502600(new_velocity);

            } else {
                this->m_velocity = new_velocity;
            }
        }
    } else {
        THISCALL(0x004CA1C0, this, new_velocity, a3);
    }
}

vector3d physical_interface::get_velocity() const
{
    vector3d result;

    if (this->field_C & 0x80000) {
        auto &v3 = this->m_bp_sys->field_0.m_list_rigid_body.m_data[0]->field_D0;

        auto v8 = v3;
        if (!v3.is_valid()) {
            v8 = ZEROVEC;
        }

        result = v8;
    } else {
        auto *prop = this->field_174;
        if (prop != nullptr) {
            auto &v11 = prop->body->field_D0;

            result = v11;
        } else {
            result = this->m_velocity;
        }
    }

    return result;
}

void physical_interface::start_biped_physics(physical_interface::biped_physics_body_types type)
{
    auto &inside = var<bool>(0x0095A6AB);
    const bool previous = inside;
    inside = true;
    if ((field_C & 0x40000u) == 0) {
        if (auto *ai = field_4->get_ai_core())
            ai->stop_movement();
        set_control_parent(nullptr);
        auto *owner = static_cast<conglomerate *>(field_4);
        if (owner->m_parent != nullptr) {
            const po absolute = owner->get_abs_po();
            owner->clear_parent(true);
            owner->set_abs_po(absolute);
        }
        m_bp_sys = create_biped_ragdoll(owner, 0, type);
        if (field_184) {
            field_180 = g_world_ptr->time_manager.field_4;
            field_184 = false;
        }
        field_185 = false;
        set_control_parent(nullptr);
        field_C |= 0x80000u;
        set_gravity(true);
        owner->field_8 |= 0x20000000u;
        for (auto *member : owner->members)
            member->field_8 |= 0x20000000u;
        for (auto *bone : owner->skin_bones)
            bone->field_8 |= 0x20000000u;
        field_80 = event_manager::add_callback(event::COLLISION_EVENT,
            owner->get_my_vhandle(), prop_system::collision_callback, nullptr, false);
        event_manager::raise_event(event::PROP_PHYSICS_START, owner->get_my_vhandle());
        if (owner->has_sound_and_pfx_ifc()) {
            static const string_hash start_sound{"biped_physics_start"};
            owner->my_sound_and_pfx_interface->play_sound_grp(
                start_sound, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
        }
    }
    inside = previous;
}

void physical_interface::set_control_parent(entity *a2)
{
    if constexpr (1) {
        if (a2 != nullptr) {
            entity_set_abs_parent(this->field_4, a2);
            this->field_84.field_0.field_0 = a2->my_handle.field_0;
            return;
        }

        auto *v3 = &this->field_84;
        if (this->field_84.get_volatile_ptr() != nullptr) {
            auto *v4 = this->field_4;
            auto *v5 = v4->m_parent;
            if (v5 != nullptr) {
                if (v5 == this->field_84.get_volatile_ptr()) {
                    entity_set_abs_parent(this->field_4, nullptr);
                    v3->field_0.field_0 = 0;
                    return;
                }
            } else {
                auto &a2a = v4->get_abs_po();
                this->field_4->set_abs_po(a2a);
            }

            v3->field_0.field_0 = 0;
        }
    } else {
        THISCALL(0x004ECC10, this, a2);
    }
}

bool physical_interface::allow_manage_standing()
{
    return (this->field_C >> 12) & 1;
}

void physical_interface::set_allow_manage_standing(bool a2)
{
    uint32_t v3;

    auto v2 = this->field_C;
    if (a2) {
        v3 = v2 | 0x1000;
    } else {
        v3 = v2 & 0xFFFFEFFF;
    }

    this->field_C = v3;
}

void physical_interface::clear_static_lists()
{
    memset(physical_interface::rotators, 0, sizeof(physical_interface::rotators));
    physical_interface::rotators_num = 0;
}

void physical_interface::set_current_gravity_vector(const vector3d &a2)
{
    this->field_74 = a2;
}

float physical_interface::get_floor_offset()
{
    if constexpr (1) {
        if (this->field_CC <= 0.0f) {
            if (this->is_biped_physics_running() || this->is_prop_physics_running()) {
                this->field_10C = 0.25f;
            } else {
                this->field_10C = this->field_4->get_floor_offset();
            }

        } else {
            this->field_10C = this->field_CC;
        }

        if (this->field_17D) {
            this->field_10C *= this->field_4->get_render_scale()[1];
        }

        return this->field_10C;

    } else {
        return (float)THISCALL(0x004BCC50, this);
    }
}

float physical_interface::calc_height_above_ground()
{
    if (std::equal_to<float>{}(ground_elevation, -10000.0f))
        return field_F0;
    const auto height = field_4->get_abs_position().y;
    return height - get_floor_offset() - ground_elevation;
}

bool physical_interface::is_biped_stable() const
{
    return (field_C & 0x80000u) == 0 || m_bp_sys->field_0.field_420;
}

void physical_interface::suspend(bool a2)
{
    if (a2) {
        this->field_C |= 2;
    } else {
        this->field_C &= 0xFFFFFFFD;
    }
}

vector3d physical_interface::calculate_perfect_force_vector(const vector3d &start, const vector3d &target, Float max_y,
                                                            Float gravity_multiplier)
{
    assert(start != target);

    auto v5 = g_gravity * gravity_multiplier * 0.5f;

    assert(max_y > start.y);

    auto v0_y_sq = (4.0 * v5) * (max_y - start.y);
    assert(v0_y_sq > 0.0f);

    vector3d result;
    result.y = std::sqrt(v0_y_sq);

    assert(max_y > target.y);
    auto determinant = (max_y - target.y) / v5;

    assert(determinant > 0.0f);

    auto total_time = (result.y / (2.0 * v5)) + std::sqrt(determinant);
    assert(total_time > 0.0f);

    result.x = (target.x - start.x) / total_time;
    result.z = (target.z - start.z) / total_time;
    return result;
}

void physical_interface::calculate_force_vector(Float a1, Float a2, float *a3, float *a4, Float a5)
{
    *a4 = 0.0;
    *a3 = 0.0;
    auto v5 = g_gravity * a5;
    if (a1 < LARGE_EPSILON) {
        a1 = LARGE_EPSILON;
    }

    if (v5 > 0.0) {
        auto v6 = std::sqrt(a1 * v5 + a1 * v5);
        *a4 = v6;
        auto v7 = (v6 + v6) / v5;
        if (std::abs(v7) > EPSILON) {
            *a3 = a2 / v7;
        }
    }
}

bool physical_interface::is_enabled() const
{
    return this->is_flag(1);
}

int physical_interface::get_num_active_pendulums() const
{
    int v3 = 0;
    for (auto &p : this->field_110) {
        if (p != nullptr) {
            if (p->m_active) {
                ++v3;
            }
        }
    }

    return v3;
}

void physical_interface::enable(bool a2)
{
    if (a2) {
        this->field_C |= 1;
    } else {
        this->field_C &= 0xFFFFFFFE;
    }
}

void physical_interface::set_pendulum(int num, pendulum *a3)
{
    assert(num >= 0 && num < PHYS_IFC_MAX_PENDULUM_CONSTRAINTS);

    auto *previous = field_110[num];
    if (previous != nullptr && previous != a3)
        previous->sub_4BD990(this);
    if (a3 != nullptr && previous != a3) {
        a3->biped_physics_constraint = nullptr;
        a3->pivot_rigid_body = nullptr;
    }
    field_110[num] = a3;
}

void physical_interface::set_gravity(bool a2)
{
    unsigned int v2 = this->field_C;
    if (((v2 >> 2) & 1) != a2) {
        if (a2) {
            this->field_C |= 4;
        } else {
            this->field_C &= 0xFFFFFFFB;
        }

        this->field_A4 = 0;
    }
}

vector3d physical_interface::apply_positional_constraints(Float, const vector3d &position, bool)
{
    auto result = position;
    for (auto *constraint : field_110) {
        if (constraint == nullptr || !constraint->m_active)
            continue;
        const auto pivot = constraint->get_pivot_abs_pos();
        const auto offset = result - pivot;
        const float length = offset.length();
        if (length > constraint->m_constraint) {
            const float lenience = std::clamp(
                g_world_ptr->time_manager.field_8 * constraint->field_20, 0.0f, 1.0f);
            const float radius = constraint->m_constraint +
                (length - constraint->m_constraint) * lenience;
            result = pivot + offset * (radius / length);
        }
    }
    return result;
}

void physical_interface::apply_force_increment_in_biped_physics_mode(const vector3d &a2,
                                                                     physical_interface::force_type a3,
                                                                     const vector3d &a4, int a5)
{
    if constexpr (1) {
        biped_system *v5 = nullptr;
        if ((this->field_C & 0x80000) != 0) {
            v5 = this->m_bp_sys;
        }

        if (a3 == 1) {
            phys_vector3d a2a;
            phys_vector3d a3a;
            float a4a;
            v5->field_0.get_ballistic_info(&a2a, &a3a, &a4a);

            auto v6 = a2[0];
            auto v7 = a2[1];
            auto v15 = a2[2];
            auto v8 = v5->field_0.m_list_rigid_body.m_data;
            auto v13 = v6;
            auto v9 = *v8;
            auto v10 = a4a * v9->field_130;
            auto v14 = v10 * v7;
            v15 = v15 * v10;
            auto v11 = 1.f / v9->field_130;
            auto v19 = v11;

            phys_vector3d v16;
            v16[0] = v11 * (v13 * v10);
            v16[1] = v19 * v14;
            v16[2] = v19 * v15;

            v5->field_0.apply_pulse(0, v16);
        }

    } else {
        THISCALL(0x004C9430, this, &a2, a3, &a4, a5);
    }
}

void physical_interface::apply_force_increment(const vector3d &force, physical_interface::force_type type,
                                               const vector3d &point, int limb)
{
    if ((field_C & 0x80000u) != 0) {
        apply_force_increment_in_biped_physics_mode(force, type, point, limb);
    } else if (field_174 != nullptr) {
        if (type == 1)
            set_velocity(get_velocity() + force, false);
    } else if (!(field_10 <= 0.0f && field_10 >= 0.0f)) {
        const auto increment = force * (1.0f / field_10);
        field_38 += increment;
        if (increment.length2() > EPSILON)
            moved_entities::add_moved(vhandle_type<entity>{field_4->get_my_vhandle()});
        if (type == 0)
            field_44 += increment * 0.5f;
        field_4->field_4 |= 0x40u;
    }
    if (dot(field_74, force) < 0.0f) {
        if (field_4->get_ai_core() != nullptr) {
            if (auto *parent = field_84.get_volatile_ptr()) {
                auto velocity = ZEROVEC;
                if (parent->has_physical_ifc())
                    velocity = parent->physical_ifc()->get_velocity();
                else if (parent->is_an_actor() && static_cast<actor *>(parent)->is_frame_delta_valid())
                    velocity = (parent->get_abs_position() - parent->get_last_position()) /
                        g_world_ptr->time_manager.field_8;
                field_38 += velocity;
                set_control_parent(nullptr);
            }
        }
        field_184 = false;
        field_185 = false;
        field_170 = field_180 = 0;
    }
    if (field_4->get_ai_core() != nullptr)
        set_control_parent(nullptr);
    field_170 = 0;
}

string_hash physical_interface::calc_obb_face_terrain_type(const vector3d &position,
                                                         subdivision_node_obb_base *obb)
{
    vector4d half, row_x, row_y, row_z;
    const bool rotated = obb->unpack_xform(half, row_x, row_y, row_z);
    const vector3d delta = position - obb->center;
    const vector3d local = rotated
        ? vector3d{delta.x * row_x[0] + delta.y * row_y[0] + delta.z * row_z[0],
                   delta.x * row_x[1] + delta.y * row_y[1] + delta.z * row_z[1],
                   delta.x * row_x[2] + delta.y * row_y[2] + delta.z * row_z[2]}
        : delta;
    const float x = local.x / half[0], y = local.y / half[1], z = local.z / half[2];
    const float ax = std::fabs(x), ay = std::fabs(y), az = std::fabs(z);
    unsigned face;
    if (ax >= ay && ax >= az)
        face = x > 0.0f ? 0 : 1;
    else if (ay >= ax && ay >= az)
        face = y > 0.0f ? 2 : 3;
    else if (az >= ax && az >= ay)
        face = z > 0.0f ? 4 : 5;
    else
        return string_hash{static_cast<int>(to_hash("INVALID"))};
    const auto packed = static_cast<unsigned char>(obb->terrain_type_info[face >> 1]);
    return terrain_types_manager::get_terrain_type_by_index((packed >> ((~(4 * face)) & 4)) & 0xF);
}

void physical_interface::synchronize_pendulum_constraints_with_position()
{
    for (auto *constraint : field_110) {
        if (constraint != nullptr && constraint->m_active) {
            const auto &position = field_4->get_abs_position();
            const auto &pivot = constraint->get_pivot_abs_pos();
            constraint->m_constraint = (position - pivot).length();
        }
    }
}

void physical_interface::manage_standing(bool force)
{
    manage_standing_internal(force, 0.0f);
}

void physical_interface::manage_standing_internal(bool force, float)
{
    if (!(field_C & 0x1000) || field_4->get_primary_region() == nullptr)
        return;


    const bool must_update =
        (field_C & 0x80000) || field_174 != nullptr || force || (field_C & 0x200) ||
        field_4->is_hero() || field_4->get_ai_core() == nullptr ||
        !field_4->get_occluded_last_frame() ||
        ((field_4->field_4 & 0x200) &&
         (!field_184 || field_84.get_volatile_ptr() != nullptr ||
          m_velocity.length2() >= EPSILON ||
          (field_4->is_frame_delta_valid() &&
           field_4->get_frame_delta()->get_position().length2() >= EPSILON)));
    if (!must_update || !(field_4->field_4 & 4))
        return;
    using category_callback = bool (__fastcall *)(actor *, void *);
    if (reinterpret_cast<category_callback>(get_vfunc(field_4->m_vtbl, 0xC8))(field_4, nullptr) ||
        reinterpret_cast<category_callback>(get_vfunc(field_4->m_vtbl, 0xF0))(field_4, nullptr))
        return;

    const auto set_standing = [this](bool standing) {
        if (field_184 != standing) {
            field_180 = g_world_ptr->time_manager.field_8;
            field_184 = standing;
        }
    };
    const auto remember_elevation = [this] {
        if (std::not_equal_to<float>{}(ground_elevation, -10000.0f))
            field_F8 = ground_elevation;
        field_F4 = ground_elevation;
    };
    const auto query_elevation = [this, &remember_elevation](
                                     vector3d &position, entity *&ground,
                                     subdivision_node_obb_base *&obb) {
        remember_elevation();
        ground = nullptr;
        obb = nullptr;
        ground_elevation = g_world_ptr->the_terrain->get_elevation(
            position, field_100, field_4, &ground, &obb, field_F0);
        if (std::equal_to<float>{}(ground_elevation, -10000.0f) ||
            (ground_elevation <= position.y + 10.0f && ground_elevation >= -10010.0f)) {
            if (obb != nullptr)
                field_88 = calc_obb_face_terrain_type(field_4->get_abs_position(), obb);
        } else {
            ground_elevation = field_184 ? position.y - get_floor_offset() : -10000.0f;
            ground = nullptr;
            obb = nullptr;
        }
    };
    const auto set_ground_pose = [this](po &transform) {
        if (field_4->m_parent != nullptr)
            transform = sub_48F770(transform, *field_4->m_parent->get_abs_po().inverse());
        field_4->set_abs_po(transform);
        synchronize_pendulum_constraints_with_position();
    };
    const auto align_up = [](po &transform, const vector3d &up) {
        auto forward = transform.get_z_facing();
        forward.normalize();
        if (is_colinear(forward, up, 0.0099999998f))
            forward = transform.get_y_facing();
        transform.set_po(forward, up, transform.get_position());
        transform.sub_48D840();
    };

    if (field_C & 0x80000) {
        set_control_parent(nullptr);
        set_standing(false);
        if (m_bp_sys->field_0.field_420 || field_170 >= 0.25f) {
            vector3d position = field_4->get_abs_position();
            if (field_4->field_4 & 4) {
                static const string_hash pelvis{static_cast<int>(to_hash("BIP01 PELVIS"))};
                if (auto *bone = static_cast<conglomerate *>(field_4)->get_bone(pelvis, true))
                    position = bone->get_abs_position();
            }
            entity *ground;
            subdivision_node_obb_base *obb;
            query_elevation(position, ground, obb);
            if (position.y - get_floor_offset() - ground_elevation < 0.5f)
                set_standing(true);
        }
        field_185 = field_184;
        return;
    }

    const bool always_stand = force || (field_C & 0x400);
    vector3d position = field_4->get_abs_position();
    if (field_C & 0x200) {
        remember_elevation();
        field_100 = field_4->get_abs_po().get_y_facing();
        entity *ground = nullptr;
        subdivision_node_obb_base *obb = nullptr;
        vector3d from = position + field_100 * 1.5f;
        vector3d contact = g_world_ptr->the_terrain->get_elevation_adv(
            from, field_100, field_4, &ground, &obb, field_F0);
        if (contact.is_valid()) {
            if (obb != nullptr)
                field_88 = calc_obb_face_terrain_type(field_4->get_abs_position(), obb);
        } else {
            field_100 = field_4->get_abs_po().get_y_facing();
            contact = field_184 ? position - field_100 * get_floor_offset()
                                : vector3d{-10000.0f, -10000.0f, -10000.0f};
            ground = nullptr;
            obb = nullptr;
        }
        ground_elevation = contact.y;
        const float floor = get_floor_offset();
        const float separation = (contact - position).length() - floor;
        if (ground_elevation > -10000.0f && separation < 10.0f &&
            ((always_stand && std::fabs(separation) < 100.0f) || separation < field_EC)) {
            const vector3d up = ground != nullptr ? field_4->get_abs_po().get_y_facing() : field_100;
            po transform = field_4->get_abs_po();
            const float threshold = field_4->is_hero() ? 1.0f : 0.99989998f;
            if (dot(transform.get_y_facing(), field_100) < threshold)
                align_up(transform, field_100);
            transform.set_position(contact + up * floor);
            set_ground_pose(transform);
            set_standing(true);
            set_velocity(ZEROVEC, false);
            field_38 = ZEROVEC;
            field_44 = ZEROVEC;
            field_50 = ZEROVEC;
            if (field_4->get_ai_core() != nullptr)
                set_control_parent(ground);
            field_185 = field_184;
            return;
        }
    } else if ((field_C & 4) && field_74.y < 0.0f) {
        entity *ground;
        subdivision_node_obb_base *obb;
        query_elevation(position, ground, obb);
        const float separation = position.y - get_floor_offset() - ground_elevation;
        const vector3d velocity = get_velocity();
        const float threshold = velocity.y > 0.05f ? 0.1f : (field_185 ? 1.0f : 0.2f);
        if ((always_stand && ground_elevation > -10000.0f) || separation < threshold) {
            po transform = field_4->get_abs_po();
            if (dot(transform.get_y_facing(), YVEC) < 0.99000001f)
                align_up(transform, YVEC);
            vector3d grounded = position;
            grounded.y -= separation;
            transform.set_position(grounded);
            set_ground_pose(transform);
            if (velocity.y <= 1.2f && field_38.y <= 1.0f && field_A4 <= 0.0f) {
                set_standing(true);
                auto clipped_velocity = get_velocity();
                if (clipped_velocity.y < 0.0f)
                    clipped_velocity.y = 0.0f;
                set_velocity(clipped_velocity, false);
                if (field_38.y < 0.0f)
                    field_38.y = 0.0f;
                if (field_44.y < 0.0f)
                    field_44.y = 0.0f;
                if (field_50.y < 0.0f)
                    field_50.y = 0.0f;
                if (field_4->get_ai_core() != nullptr)
                    set_control_parent(ground);
                field_185 = field_184;
                return;
            }
        }
    } else if (field_C & 0x800) {
        remember_elevation();
        entity *ground = nullptr;
        subdivision_node_obb_base *obb = nullptr;
        position = field_4->get_abs_position();
        ground_elevation = g_world_ptr->the_terrain->get_elevation(
            position, field_100, field_4, &ground, &obb, field_F0);
    }
    set_standing(false);
    if (field_4->get_ai_core() != nullptr)
        set_control_parent(nullptr);
    field_185 = field_184;
}

void physical_interface::stop_biped_physics(bool destroying)
{
    if ((field_C & 0x80000u) == 0)
        return;
    for (auto *constraint : field_110) {
        if (constraint != nullptr && (field_C & 0x80000u) != 0) {
            if (constraint->biped_physics_constraint != nullptr) {
                phys_sys::destroy(constraint->biped_physics_constraint);
                constraint->biped_physics_constraint = nullptr;
            }
            if (constraint->pivot_rigid_body != nullptr) {
                phys_sys::destroy(static_cast<user_rigid_body *>(constraint->pivot_rigid_body));
                constraint->pivot_rigid_body = nullptr;
            }
        }
    }
    const vector3d velocity = get_velocity();
    field_C &= ~0x80000u;
    destroy_biped_ragdoll(m_bp_sys);
    m_bp_sys = nullptr;
    set_velocity(velocity, false);
    if (!destroying)
        field_4->invalidate_frame_delta();
    auto *owner = static_cast<conglomerate *>(field_4);
    owner->field_8 &= ~0x20000000u;
    for (auto *member : owner->members)
        member->field_8 &= ~0x20000000u;
    for (auto *bone : owner->skin_bones)
        bone->field_8 &= ~0x20000000u;
    remove_collision_event_callback();
    event_manager::raise_event(event::PROP_PHYSICS_STOP, owner->get_my_vhandle());
}

void physical_interface::remove_collision_event_callback()
{
    event_manager::remove_callback(field_80, event::COLLISION_EVENT, field_4->get_my_vhandle());
    field_80 = 0;
}

void physical_interface::stop_prop_physics(bool a2)
{
    if constexpr (STANDALONE_SYSTEM)
        prop_system::stop(field_4, a2);
    else
        THISCALL(0x004F10F0, this, a2);
}

bool physical_interface::start_prop_physics(const vector3d &velocity,
    float randomness, float lifetime, prop_phys_priority priority)
{
    if constexpr (STANDALONE_SYSTEM)
        return prop_system::start(field_4, velocity, randomness, lifetime, priority);
    else
        return THISCALL(0x004F7D50, this, &velocity, randomness, lifetime, priority);
}

void physical_interface::remove_from_phys_ifc_list()
{
    for (auto it = all_phys_interfaces->begin(); it != all_phys_interfaces->end(); ++it) {
        if ((*it) == this) {
            all_phys_interfaces->erase(it);
            break;
        }
    }

    if (all_phys_interfaces->empty()) {
        delete all_phys_interfaces;
        all_phys_interfaces = nullptr;
    }
}

physical_interface::~physical_interface()
{
    if (!g_generating_vtables) {
        if (field_E8 != nullptr) {
            if (LOBYTE(field_E8->field_1C)) {
                auto destroy = reinterpret_cast<void(__fastcall *)(void *, void *, bool)>(
                    get_vfunc(field_E8->m_vtbl, 0));
                destroy(field_E8, nullptr, true);
            }
            field_E8 = nullptr;
        }
        if ((field_C & 0x80000u) != 0)
            stop_biped_physics(true);
        if (field_174 != nullptr)
            stop_prop_physics(true);
        remove_from_phys_ifc_list();
    }
    field_4 = nullptr;
}

void physical_interface::release_ifc()
{
    auto *v2 = this->field_E8;
    if (v2 != nullptr) {
        if (LOBYTE(v2->field_1C)) {
            void(__fastcall * finalize)(void *, void *edx, bool) = CAST(finalize, get_vfunc(v2->m_vtbl, 0x0));
            finalize(v2, nullptr, true);
        }

        this->field_E8 = nullptr;
    }

    if ((this->field_C & 0x80000) != 0) {
        this->stop_biped_physics(true);
    }

    field_188.~mString();
    field_198.~mString();

    if (this->field_174) {
        this->stop_prop_physics(true);
    }

    this->remove_from_phys_ifc_list();
}

void physical_interface_patch()
{
    {
        FUNC_ADDRESS(address, &physical_interface::start_biped_physics);

        //SET_JUMP(0x004F2460, address);
    }

    {
        REDIRECT(0x005584B2, physical_interface::frame_advance_all_phys_interfaces);
    }
}
