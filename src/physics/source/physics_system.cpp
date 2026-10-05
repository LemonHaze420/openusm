#include "physics_system.h"

#include "common.h"
#include "func_wrapper.h"
#include "phys_mem_info.h"
#include "phys_mem.h"
#include "tl_system.h"
#include "utility.h"
#include "entity_base.h"
#include "oldmath_po.h"
#include "physics_system_internal.h"
#include "nuge.h"
#include "phys_vector3d.h"
#include "outer_time.h"
#include "prop_system.h"
#include "biped_system.h"
#include "scratchpad_stack.h"

#include <algorithm>
#include <cstring>
#include <new>
#include <type_traits>

#include <rb_ragdoll_model.h>

static int &g_physics_system_size = var<int>(0x0098456C);

static int &g_physics_system_alignment = var<int>(0x00984570);

physics_system *&g_physics_system = var<physics_system *>(0x00984568);

VALIDATE_SIZE(physics_system, 0x2D8);
VALIDATE_OFFSET(physics_system, field_1C4, 0x1C4);
VALIDATE_SIZE(physics_contact_array, 0xC);
VALIDATE_OFFSET(physics_system, field_200, 0x200);
VALIDATE_OFFSET(physics_system, field_20C, 0x20C);
VALIDATE_OFFSET(physics_system, field_290, 0x290);
VALIDATE_SIZE(contact_point_info, 0x6C);
VALIDATE_SIZE(rigid_body_constraint_contact, 0x30);
VALIDATE_SIZE(rigid_body_constraint_distance, 0x5C);

//0x0059F4B0
void physics_system_collision_callback()
{
    if constexpr (!STANDALONE_SYSTEM) {
        CDECL_CALL(0x0059F4B0);
        return;
    }
    biped_system::collision_callback();
    int contacts = 0;
    prop_system::environment_collision_callback(contacts);
}

void calc_bone_mat_from_rb(void *bone, rigid_body *body, const po *offset)
{
    po::compose(*static_cast<entity_base *>(bone)->my_abs_po,
                reinterpret_cast<const po &>(body->field_0), *offset);
}

void calc_rb_mat_from_bone(void *bone, rigid_body *body, const po *binding)
{
    po inverse = po_identity_matrix;
    for (int axis = 0; axis < 3; ++axis)
        for (int component = 0; component < 3; ++component)
            inverse[axis][component] = (*binding)[component][axis];
    inverse.set_position(binding->inverse_xform(ZEROVEC));
    po::compose(reinterpret_cast<po &>(body->field_0),
        *static_cast<entity_base *>(bone)->my_abs_po, inverse);
}

void physics_system_init()
{
    if constexpr (STANDALONE_SYSTEM) {
        phys_mem_info v1{};
        v1.field_0 += 122;
        v1.field_4 += 45;
        v1.field_8 += 490;
        v1.field_C += 185;
        v1.field_18 += 45;
        v1.field_1C += 81;
        v1.field_24 += 81;
        v1.field_28 = 128;

        phys_sys::phys_init(v1);
        phys_sys::set_collision_callback(physics_system_collision_callback);
        phys_sys::set_v_tol(4, 4, 0.010000001f);
        phys_sys::set_vp_tol(4, 8, 0.25f);

        ragdoll_callbacks a1;
        a1.m_calc_bone_mat_from_rb = (void *)&calc_bone_mat_from_rb;
        a1.m_calc_rb_mat_from_bone = (void *)&calc_rb_mat_from_bone;
        rb_ragdoll_model::set_ragdoll_callbacks(a1);
    } else {
        CDECL_CALL(0x0059F4D0);
    }
}

void physics_system_shutdown()
{
    phys_sys::phys_shutdown();
}

uint32_t physics_system::get_buffer_size(const phys_mem_info &a1)
{
    return (44 * a1.field_0 +
            ((((((((((((((108 * a1.field_8 +
                          ((108 * a1.field_8 +
                            ((56 * a1.field_C +
                              ((372 * a1.field_0 +
                                ((((56 * a1.field_0 + ((24 * a1.field_C + 731) & 0xFFFFFFFC) + 3) & 0xFFFFFFFC) +
                                  444 * a1.field_4 + 3) &
                                 0xFFFFFFFC) +
                                3) &
                               0xFFFFFFFC) +
                              3) &
                             0xFFFFFFFC) +
                            3) &
                           0xFFFFFFFC) +
                          3) &
                         0xFFFFFFFC) +
                        80 * a1.field_10 + 3) &
                       0xFFFFFFFC) +
                      236 * a1.field_14 + 3) &
                     0xFFFFFFFC) +
                    100 * a1.field_18 + 15) &
                   0xFFFFFFF0) +
                  328 * a1.field_1C + 3) &
                 0xFFFFFFFC) +
                216 * a1.field_20 + 3) &
               0xFFFFFFFC) +
              200 * a1.field_24 + 3) &
             0xFFFFFFFC) +
            148 * (a1.field_28 + 1) + 15) &
           0xFFFFFFF0;
}

physics_system::physics_system()
    : field_0(0), field_4(0), field_8(0), m_callback(nullptr),
      field_10(0.051282052f), field_14(4), field_18(4), field_1C(0.010000001f),
      field_20(4), field_24(8), field_28(0.25f),
      field_2C(reinterpret_cast<physics_contact_array *>(&field_200)),
      field_30(reinterpret_cast<physics_contact_array *>(&field_20C)),
      field_1A0(0), field_1A4(0), field_1A8(0), field_1AC(0),
      field_1B0(0), field_1B4(0), field_1B8(nullptr), field_1BC(0), field_1C0(0),
      field_1C4{}, field_1D8{}, field_1EC{},
      field_200(nullptr), field_204(0), field_208(0),
      field_20C(nullptr), field_210(0), field_214(0),
      field_218{}, field_22C{}, field_240{}, field_254{}, field_268{}, field_27C{},
      field_290(nullptr), field_294(nullptr), field_298(nullptr),
      field_29C(0), field_2A0(0), field_2A4(0), field_2A8(0), field_2AC(0),
      field_2B0(0), field_2B4(0), field_2B8(0), field_2BC(0), field_2C0(0),
      field_2C4(0), field_2C8(0), field_2CC(0), field_2D0(0), field_2D4(0)
{
    field_34.set();
}

void physics_system::frame_advance(Float elapsed)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x007AB170, this, elapsed);
        return;
    }
    const float frame_time = elapsed;
    int steps = std::max(1, static_cast<int>(frame_time / field_10));
    if (frame_time / steps > field_10)
        ++steps;
    const float step_time = frame_time / steps;
    const outer_time outer{frame_time};
    for (int index = 0; index < field_1D8.m_alloc_count; ++index)
        field_1D8.m_alloc_list[index]->prolog_frame_advance(frame_time);
    for (int index = 0; index < field_1C4.m_alloc_count; ++index) {
        auto *body = field_1C4.m_alloc_list[index];
        nuge::calc_velocities(body->field_0,
            *reinterpret_cast<const matrix4x4 *>(body->m_dictator),
            frame_time * body->field_13C,
            reinterpret_cast<phys_vector3d &>(body->field_D0),
            reinterpret_cast<phys_vector3d &>(body->field_E0));
    }
    for (int index = 0; index < field_240.m_alloc_count; ++index)
        field_240.m_alloc_list[index]->outer_prolog_update(outer);
    for (int index = 0; index < field_27C.m_alloc_count; ++index) {
        auto *actuator = field_27C.m_alloc_list[index];
        auto *bytes = reinterpret_cast<char *>(actuator);
        auto &previous = *reinterpret_cast<matrix4x4 *>(bytes + 0xC);
        auto &target = *reinterpret_cast<matrix4x4 *>(bytes + 0x4C);
        for (int axis = 0; axis < 3; ++axis) {
            previous[3][axis] = 0.0f;
            target[3][axis] = 0.0f;
        }
        auto *body = actuator->b1;
        if (body == nullptr || (body->field_144 & 0x10) != 0)
            body = actuator->b2;
        phys_vector3d linear;
        nuge::calc_velocities(previous, target, frame_time * body->field_13C,
            linear, *reinterpret_cast<phys_vector3d *>(bytes + 0x8C));
    }
    for (int step = 0; step < steps; ++step)
        time_step(step_time, step + 1 == steps);
    for (int index = 0; index < field_1D8.m_alloc_count; ++index) {
        auto *body = field_1D8.m_alloc_list[index];
        std::memset(body->field_110, 0, 16);
        std::memset(&body->field_120, 0, 16);
    }
    for (int index = 0; index < field_240.m_alloc_count; ++index)
        field_240.m_alloc_list[index]->outer_epilog_update(outer);
    for (int index = 0; index < field_27C.m_alloc_count; ++index) {
        auto *bytes = reinterpret_cast<char *>(field_27C.m_alloc_list[index]);
        std::memcpy(bytes + 0xC, bytes + 0x4C, sizeof(matrix4x4));
    }
}

void physics_system::time_step(float elapsed, bool final_step)
{
    field_4 = bit_cast<int>(elapsed);
    field_0 |= 1;
    for (int index = 0; index < field_1D8.m_alloc_count; ++index)
        field_1D8.m_alloc_list[index]->predict_pose(elapsed);
    for (int index = 0; index < field_1C4.m_alloc_count; ++index)
        field_1C4.m_alloc_list[index]->predict_pose(elapsed);
    if (m_callback != nullptr)
        m_callback();
    field_0 &= ~1;
    auto *partitions = physics_build_constraint_partitions(this);
    int max_steps = 0;
    tlScratchpadLocked = true;
    for (auto *partition = partitions; partition != nullptr; partition = partition->next) {
        partition->elapsed = elapsed * partition->body->field_13C;
        const float limit = partition->body->field_140;
        partition->steps = std::max(1, static_cast<int>(partition->elapsed / limit));
        if (partition->elapsed / partition->steps > limit)
            ++partition->steps;
        max_steps = std::max(max_steps, partition->steps);
    }
    const int next_visit = field_8 + max_steps;
    for (auto *partition = partitions; partition != nullptr; partition = partition->next)
        physics_execute_constraint_solver(this, partition, field_8, next_visit);
    field_8 = next_visit;
    tlScratchpadLocked = false;
    for (int index = 0; index < field_1C4.m_alloc_count; ++index) {
        auto *body = field_1C4.m_alloc_list[index];
        if (final_step)
            body->field_0 = *reinterpret_cast<const matrix4x4 *>(body->m_dictator);
        else
            body->field_0 = body->field_40;
    }
    std::swap(field_2C, field_30);
    field_30->count = 0;
    field_1A8 = 0;
    field_1AC = 0;
    int index = 0;
    while (index < field_1EC.m_alloc_count) {
        auto *contact = field_1EC.m_alloc_list[index];
        auto *points = reinterpret_cast<int *>(contact->field_28);
        if (points[2] != 0) {
            std::swap(contact->field_24, contact->field_28);
            std::memset(reinterpret_cast<void *>(contact->field_28), 0, 12);
            auto **slot = physics_contact_insert(this, contact->b1, contact->b2, contact);
            contact->field_2C = reinterpret_cast<int>(slot);
            ++index;
        } else {
            contact->field_2C = 0;
            field_1EC.destroy_member(contact);
        }
    }
}

void physics_system::create_inst(const phys_mem_info &a1)
{
    g_physics_system_size = physics_system::get_buffer_size(a1);

    g_physics_system_alignment = physics_system::get_buffer_alignment();

    auto *addr = tlMemAlloc(g_physics_system_size, g_physics_system_alignment, 0x5000000u);
    phys_memory_heap a2{};
    a2.init(addr, g_physics_system_size, g_physics_system_alignment);
    g_physics_system = physics_system::allocate_buffer(a1, a2);

    assert(addr == g_physics_system);
}

void physics_system::destroy_inst()
{
    g_physics_system->~physics_system();
    tlMemFree(g_physics_system);
    g_physics_system = nullptr;

    g_physics_system_size = 0;
    g_physics_system_alignment = 0;
}

uint32_t physics_system::get_buffer_alignment()
{
    return 16u;
}

physics_system *physics_system::allocate_buffer(const phys_mem_info &info, phys_memory_heap &heap)
{
    if constexpr (!STANDALONE_SYSTEM)
        return reinterpret_cast<physics_system *>(CDECL_CALL(0x007AB5E0, &info, &heap));
    auto *storage = heap.allocate(sizeof(physics_system), 4);
    auto *world = new (storage) physics_system;
    if (info.field_C > 0) {
        world->field_1A4 = info.field_C;
        world->field_1A0 = reinterpret_cast<int>(heap.allocate(24 * info.field_C, 4));
    }
    if (info.field_0 > 0) {
        world->field_1BC = info.field_0;
        world->field_1B8 = static_cast<char *>(heap.allocate(56 * info.field_0, 4));
    }
    const auto allocate_pool = [&heap](auto &pool, int count, int alignment) {
        if (count <= 0)
            return;
        using member_type = std::remove_pointer_t<decltype(pool.m_slot_array)>;
        auto *buffer = static_cast<char *>(heap.allocate((sizeof(member_type) + 8) * count, alignment));
        pool.m_slot_array_size = count;
        pool.m_slot_array = reinterpret_cast<member_type *>(buffer);
        pool.m_alloc_list = reinterpret_cast<member_type **>(buffer + sizeof(member_type) * count);
        pool.m_index_array = reinterpret_cast<int *>(pool.m_alloc_list + count);
        for (int index = 0; index < count; ++index) {
            pool.m_alloc_list[index] = &pool.m_slot_array[index];
            pool.m_index_array[index] = index;
        }
        pool.m_alloc_count = 0;
    };
    allocate_pool(world->field_1C4, info.field_4, 4);
    allocate_pool(world->field_1D8, info.field_0, 4);
    allocate_pool(world->field_1EC, info.field_C, 4);
    if (info.field_8 > 0) {
        world->field_204 = world->field_210 = info.field_8;
        world->field_200 = static_cast<contact_point_info *>(heap.allocate(108 * info.field_8, 4));
        world->field_20C = static_cast<contact_point_info *>(heap.allocate(108 * info.field_8, 4));
    }
    allocate_pool(world->field_218, info.field_10, 4);
    allocate_pool(world->field_22C, info.field_14, 4);
    allocate_pool(world->field_240, info.field_18, 4);
    allocate_pool(world->field_254, info.field_1C, 16);
    allocate_pool(world->field_268, info.field_20, 4);
    allocate_pool(world->field_27C, info.field_24, 4);
    const int scratch_size = 44 * info.field_0 + 148 * (info.field_28 + 1);
    world->field_290 = static_cast<char *>(heap.allocate(scratch_size, 4));
    world->field_294 = world->field_290 + scratch_size;
    world->field_298 = world->field_290;
    return world;
}

void phys_sys::phys_init(const phys_mem_info &a1)
{
    physics_system::create_inst(a1);
}

void phys_sys::set_collision_callback(void (*a1)())
{
    g_physics_system->m_callback = a1;
}

void phys_sys::set_v_tol(int a1, int a2, Float a3)
{
    g_physics_system->field_14 = a1;
    g_physics_system->field_18 = a2;
    g_physics_system->field_1C = a3;
}

void phys_sys::set_vp_tol(int a1, int a2, Float a3)
{
    g_physics_system->field_20 = a1;
    g_physics_system->field_24 = a2;
    g_physics_system->field_28 = a3;
}

rigid_body *phys_sys::create_rigid_body()
{
    auto v0 = g_physics_system->field_1D8.m_alloc_count;
    if (v0 >= g_physics_system->field_1D8.m_slot_array_size) {
        return nullptr;
    }

    auto *result = g_physics_system->field_1D8.m_alloc_list[v0];
    g_physics_system->field_1D8.m_alloc_count = v0 + 1;
    return result;
}

void phys_sys::destroy(rigid_body *body)
{
    destroy_all_constraint(body);
    g_physics_system->field_1D8.destroy_member(body);
}

rigid_body_constraint_distance *phys_sys::create_rbc_dist(rigid_body *a1, rigid_body *a2)
{
    if constexpr (!STANDALONE_SYSTEM)
        return reinterpret_cast<rigid_body_constraint_distance *>(CDECL_CALL(0x007A1820, a1, a2));
    auto &pool = g_physics_system->field_240;
    assert(pool.m_alloc_count < pool.m_slot_array_size);
    auto *constraint = pool.m_alloc_list[pool.m_alloc_count++];
    for (auto &cache : constraint->field_44)
        cache.field_0 = -1;
    constraint->b1 = a1;
    constraint->b2 = a2;
    return constraint;
}

user_rigid_body *phys_sys::create_user_rigid_body()
{
    auto v0 = g_physics_system->field_1C4.m_alloc_count;
    if (v0 >= g_physics_system->field_1C4.m_slot_array_size) {
        return nullptr;
    }

    auto *result = g_physics_system->field_1C4.m_alloc_list[v0];
    g_physics_system->field_1C4.m_alloc_count = v0 + 1;
    return result;
}

environment_rigid_body *phys_sys::get_environment_rigid_body()
{
    return &g_physics_system->field_34;
}

void phys_sys::phys_frame_advance(Float a1)
{
    g_physics_system->frame_advance(a1);
}

rigid_body_constraint_contact *phys_sys::create_no_error_rbc_contact(rigid_body *a1, rigid_body *a2)
{
    if constexpr (!STANDALONE_SYSTEM)
        return reinterpret_cast<rigid_body_constraint_contact *>(CDECL_CALL(0x007A1D60, a1, a2));
    auto *world = g_physics_system;
    if (world->field_1A8 == world->field_1A4)
        return nullptr;
    auto **slot = physics_contact_insert(world, a1, a2, nullptr);
    if (slot == nullptr)
        return nullptr;
    if (*slot == nullptr) {
        auto &pool = world->field_1EC;
        assert(pool.m_alloc_count < pool.m_slot_array_size);
        auto *contact = pool.m_alloc_list[pool.m_alloc_count++];
        contact->filed_C = contact->field_10 = contact->field_14 = 0;
        contact->field_18 = contact->field_1C = contact->field_20 = 0;
        contact->field_24 = reinterpret_cast<int>(&contact->filed_C);
        contact->field_28 = reinterpret_cast<int>(&contact->field_18);
        contact->field_2C = reinterpret_cast<int>(slot);
        contact->b1 = a1;
        contact->b2 = a2;
        *slot = contact;
    }
    return *slot;
}

void phys_sys::destroy(rigid_body_constraint_distance *a1)
{
    g_physics_system->field_240.destroy_member(a1);
}

void phys_sys::destroy(user_rigid_body *a1)
{
    destroy_all_constraint(a1);
    g_physics_system->field_1C4.destroy_member(a1);
}

void phys_sys::destroy_all_constraint(rigid_body *body)
{
    const auto remove_attached = [body](auto &pool) {
        int index = 0;
        while (index < pool.m_alloc_count) {
            auto *constraint = pool.m_alloc_list[index];
            if ((constraint->b1 != nullptr && constraint->b1 == body) ||
                (constraint->b2 != nullptr && constraint->b2 == body)) {
                if constexpr (std::is_same_v<std::remove_pointer_t<decltype(constraint)>,
                    rigid_body_constraint_contact>) {
                    if (constraint->field_2C != 0)
                        *reinterpret_cast<rigid_body_constraint_contact **>(constraint->field_2C) = nullptr;
                }
                pool.destroy_member(constraint);
            } else {
                ++index;
            }
        }
    };
    remove_attached(g_physics_system->field_1EC);
    remove_attached(g_physics_system->field_218);
    remove_attached(g_physics_system->field_22C);
    remove_attached(g_physics_system->field_240);
    remove_attached(g_physics_system->field_254);
    remove_attached(g_physics_system->field_268);
    remove_attached(g_physics_system->field_27C);
}

void phys_sys::phys_shutdown()
{
    physics_system::destroy_inst();
}

void physics_system_patch()
{
    REDIRECT(0x007AB8B4, physics_system::allocate_buffer);
}
