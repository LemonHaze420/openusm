#include "physics_system.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "phys_mem_info.h"
#include "phys_mem.h"
#include "tl_system.h"
#include "utility.h"

#include <rb_ragdoll_model.h>

static int &g_physics_system_size = var<int>(0x0098456C);

static int &g_physics_system_alignment = var<int>(0x00984570);

physics_system *&g_physics_system = var<physics_system *>(0x00984568);

VALIDATE_SIZE(physics_system, 0x2D8);
VALIDATE_OFFSET(physics_system, field_1C4, 0x1C4);

//0x0059F4B0
void physics_system_collision_callback()
{
    CDECL_CALL(0x0059F4B0);
}

void calc_bone_mat_from_rb(void *a1, rigid_body *a2, int a3)
{
    CDECL_CALL(0x00592CE0, a1, a2, a3);
}

void calc_rb_mat_from_bone(void *a1, rigid_body *a2, int a3)
{
    CDECL_CALL(0x0059AEC0, a1, a2, a3);
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
{
    if constexpr (0) {
        this->field_1A0 = 0;
        this->field_1A4 = 0;
        this->field_1AC = 0;
        this->field_1A8 = 0;
        this->field_1B8 = 0;
        this->field_1BC = 0;
        this->field_1C0 = 0;

        this->field_200 = 0;
        this->field_204 = 0;
        this->field_208 = 0;
        this->field_20C = 0;
        this->field_210 = 0;
        this->field_214 = 0;
        this->field_218 = 0;
        this->field_21C = 0;
        this->field_220 = 0;
        this->field_224 = 0;
        this->field_228 = 0;
        this->field_22C = 0;
        this->field_230 = 0;
        this->field_234 = 0;
        this->field_238 = 0;
        this->field_23C = 0;
        this->field_240 = 0;
        this->field_244 = 0;
        this->field_248 = 0;
        this->field_24C = 0;
        this->field_250 = 0;
        this->field_254[0] = 0;
        this->field_254[1] = 0;
        this->field_254[2] = 0;
        this->field_254[3] = 0;
        this->field_264 = 0;
        this->field_268[0] = 0;
        this->field_268[1] = 0;
        this->field_268[2] = 0;
        this->field_268[3] = 0;
        this->field_278 = 0;
        this->field_27C[0] = 0;
        this->field_27C[1] = 0;
        this->field_27C[2] = 0;
        this->field_27C[3] = 0;
        this->field_28C = 0;

        auto *v2 = &this->field_200;
        auto *v3 = &this->field_20C;
        this->field_290 = 0;
        this->field_294 = 0;
        this->field_298 = 0;
        this->field_29C = 0;
        this->field_2A0 = 0;
        this->field_2A4 = 0;
        this->field_2A8 = 0;
        this->field_2AC = 0;
        this->field_2B0 = 0;
        this->field_2B4 = 0;
        this->field_2B8 = 0;
        this->field_2BC = 0;
        this->field_2C0 = 0;
        this->field_2C4 = 0;
        this->field_2C8 = 0;
        this->field_2CC = 0;
        this->field_2D0 = 0;
        this->field_2D4 = 0;
        this->field_34.set();
        this->field_8 = 0;
        this->m_callback = nullptr;
        this->field_14 = 4;
        this->field_18 = 4;
        this->field_20 = 4;
        this->field_2C = (int)v2;
        this->field_30 = (int)v3;
        this->field_10 = 0.051282052;
        this->field_1C = 0.010000001;
        this->field_24 = 8;
        this->field_28 = 0.25;

    } else {
        THISCALL(0x007AB3B0, this);
    }
}

void physics_system::frame_advance(Float a2)
{
    THISCALL(0x007AB170, this, a2);
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

physics_system *physics_system::allocate_buffer(const phys_mem_info &a1, phys_memory_heap &a2)
{
#if 0
    {
        //sp_log("a2.field_8 = %d", a2.field_8);

        sp_log("(a2.field_8 + 3) & 0xFFFFFFFC = 0x%08X",
               bit_cast<uint32_t>(a2.field_8 + 3) & 0xFFFFFFFC);

        auto *v2 = (physics_system *) (bit_cast<uint32_t>(a2.field_8 + 3) & 0xFFFFFFFC);

        bool res = (v2->field_2D8 <= a2.field_4 && (a2.field_8 = v2->field_2D8, v2));

        sp_log("res = %d", res);
    }
#endif

    return (physics_system *)CDECL_CALL(0x007AB5E0, &a1, &a2);
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

rigid_body_constraint_distance *phys_sys::create_rbc_dist(rigid_body *a1, rigid_body *a2)
{
    return (rigid_body_constraint_distance *)CDECL_CALL(0x007A1820, a1, a2);
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
    return (rigid_body_constraint_contact *)CDECL_CALL(0x007A1D60, a1, a2);
}

void phys_sys::destroy(rigid_body_constraint_distance *a1)
{
    CDECL_CALL(0x007A1A50, a1);
}

void phys_sys::destroy(user_rigid_body *a1)
{
    CDECL_CALL(0x007A25C0, a1);
}

void phys_sys::phys_shutdown()
{
    physics_system::destroy_inst();
}

void physics_system_patch()
{
    REDIRECT(0x007AB8B4, physics_system::allocate_buffer);
}
