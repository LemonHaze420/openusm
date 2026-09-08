#include "ped_spawner.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"
#include "game.h"
#include "mstring.h"
#include "os_developer_options.h"
#include "oldmath_po.h"
#include "variable.h"
#include "vtbl.h"

VALIDATE_SIZE(ped_spawner, 0x4C);

_std::vector<ped_spawner *> &ped_spawner::ped_spawner_list = var<_std::vector<ped_spawner *>>(0x0096D270);
static auto &peds_initialized = var<bool>(0x0096C9B8);
static auto &special_proc_index = var<int>(0x0096C9C0);
static auto &special_proc_index_0 = var<int>(0x0096C9C4);
static auto &special_proc_timer = var<float>(0x0096C9C8);
static auto &num_peds_spawned = var<int>(0x0096C9CC);
static auto &peds_single_step = var<bool>(0x0096C9D0);
static auto &peds_paused = var<bool>(0x0096C9D1);
static auto &ped_density = var<float>(0x00937FF0);

ped_spawner::ped_spawner(int a2) : spawnable(vhandle_type<entity>{0})
{
    this->m_vtbl = 0x008A5B70;
    this->field_C = a2;
}

void ped_spawner::do_spawn(vector3d a2, vector3d a3, traffic_path_lane *a8, int a9, int a10, int a11)
{
    TRACE("ped_spawner::do_spawn");

    if constexpr (1) {
        if (this->get_my_actor() != nullptr) {
            this->sub_6BBD30(a8);
            this->spawn(a2, a3);
        }
    } else {
        THISCALL(0x006BBDA0, this, a2, a3, a8, a9, a10, a11);
    }
}

actor *ped_spawner::get_my_actor()
{
    return this->field_3C.get_volatile_ptr();
}

void ped_spawner::spawn(vector3d a4, const vector3d &a2)
{
    TRACE("ped_spawner::spawn");

    THISCALL(0x006BBE50, this, a4, &a2);
}

actor *ped_spawner::create_ped_actor()
{
    TRACE("ped_spawner::create_ped_actor");

    char v10[32]{};
    sprintf(v10, "PED_%u", this->field_C);

    entity *eb = nullptr;
    if ((unsigned int)(rand() * 0.000061035156)) {
        static string_hash ped_fem_hash{int(to_hash("ped_fem"))};

        string_hash v7{v10};
        eb = g_world_ptr->ent_mgr.acquire_entity(ped_fem_hash, v7, 129);
        this->field_44 = 2;
    } else {
        static string_hash ped_male_hash{int(to_hash("ped_male"))};

        string_hash v7{v10};
        eb = g_world_ptr->ent_mgr.acquire_entity(ped_male_hash, v7, 129);
        this->field_44 = 1;
    }

    if (eb != nullptr && eb->m_vtbl == 0) {
        return nullptr;
    }
    if (eb != nullptr) {
        auto *get_mesh_address = get_vfunc(eb->m_vtbl, 0x1B0);
        if (get_mesh_address == nullptr) {
            return nullptr;
        }
        nglMesh *(__fastcall *get_mesh)(entity *) =
            CAST(get_mesh, get_mesh_address);
        if (get_mesh(eb) == nullptr) {
            return nullptr;
        }
    }

    if (eb != nullptr) {
        assert(eb->is_an_actor());

        eb->set_visible(false, false);

        return bit_cast<actor *>(eb);
    }

    return nullptr;
}

void ped_spawner::sub_6BBD30(traffic_path_lane *a2)
{
    THISCALL(0x006BBD30, this, a2);
}

void ped_spawner::sub_6C2EA0(Float a2)
{
    TRACE("ped_spawner::sub_6C2EA0");

    THISCALL(0x006C2EA0, this, a2);
}

void ped_spawner::sub_6B9B60(Float a2)
{
    TRACE("ped_spawner::sub_6B9B60");

    THISCALL(0x006B9B60, this, a2);
}

void ped_spawner::init()
{
    TRACE("ped_spawner::init");

    if (peds_initialized) {
        return;
    }
    ped_spawner_list.clear();
    for (int index = 0; index < 10; ++index) {
        auto *spawner = new ped_spawner{index};
        spawner->field_8 = 0;
        spawner->field_10 = nullptr;
        spawner->field_14 = nullptr;
        spawner->field_18 = nullptr;
        spawner->field_1C = 0;
        spawner->field_20 = 0;
        spawner->field_24 = 0;
        spawner->field_28 = 0;
        spawner->field_2C = 0;
        spawner->field_30 = ZEROVEC;
        spawner->field_3C = vhandle_type<actor>{0};
        spawner->field_40 = false;
        spawner->field_44 = 0;
        spawner->field_48 = 0;

        auto *ped_actor = spawner->create_ped_actor();
        if (ped_actor == nullptr) {
            delete spawner;
            break;
        }
        spawner->field_3C = vhandle_type<actor>{ped_actor->get_my_vhandle()};
        ped_spawner_list.push_back(spawner);
    }
    special_proc_index = 0;
    special_proc_index_0 = 0;
    special_proc_timer = 0.0f;
    num_peds_spawned = 0;
    peds_initialized = true;
    const vector3d forward{0.0f, 0.0f, 1.0f};
    const vector3d up{0.0f, 1.0f, 0.0f};
    const vector3d position{-1234.0f, -1234.0f, -1234.0f};
    spawnable::last_camera_po.set_po(forward, up, position);
}

void ped_spawner::cleanup()
{
    TRACE("ped_spawner::cleanup");
    if (!peds_initialized) {
        return;
    }
    for (auto *spawner : ped_spawner_list) {
        delete spawner;
    }
    ped_spawner_list.clear();
    peds_initialized = false;
    num_peds_spawned = 0;
}

void ped_spawner::advance_peds(Float elapsed)
{
    TRACE("ped_spawner::advance_peds");

    if (peds_single_step) {
        peds_single_step = false;
    } else if (peds_paused) {
        return;
    }

    if (!os_developer_options::instance->get_flag(mString{"ENABLE_PEDESTRIANS"})) {
        if (peds_initialized) {
            cleanup();
        }
        return;
    }
    if (!peds_initialized) {
        if (g_game_ptr->get_current_view_camera(0) == nullptr) {
            return;
        }
        init();
    }

    const int target_count = static_cast<int>(
        static_cast<float>(ped_spawner_list.size()) * ped_density);
    if (num_peds_spawned < target_count) {
        if ((g_world_ptr->field_158.field_C & 1) == 0) {
            populate_quad_paths();
        }
        if (num_peds_spawned < target_count) {
            populate_lanes();
        }
    }
    for (auto *spawner : ped_spawner_list) {
        if (spawner->get_my_actor() == nullptr) {
            auto *ped_actor = spawner->create_ped_actor();
            if (ped_actor != nullptr) {
                spawner->field_3C = vhandle_type<actor>{ped_actor->get_my_vhandle()};
            }
        }
        spawner->sub_6C2EA0(elapsed);
    }

    special_proc_timer += elapsed.value;
    constexpr float interval = 0.011111111f;
    if (special_proc_timer > interval) {
        const int steps = static_cast<int>(special_proc_timer * 90.0f);
        special_proc_index = (special_proc_index_0 + 1) % 10;
        special_proc_index_0 = (special_proc_index_0 + steps) % 10;
        special_proc_timer -= static_cast<float>(steps) * interval;
    }
}
void ped_spawner::populate_quad_paths()
{
    CDECL_CALL(0x006CCD60);
}

void ped_spawner::populate_lanes()
{
    TRACE("ped_spawner::populate_lanes");

    CDECL_CALL(0x006D0590);
}

ped_spawner *ped_spawner::assign_non_ped_actor(vhandle_type<actor> a2, int a3)
{
    return (ped_spawner *)CDECL_CALL(0x006CAC50, a2, a3);
}

void ped_spawner_patch()
{
    {
        FUNC_ADDRESS(address, &ped_spawner::do_spawn);
        //SET_JUMP(0x006BBDA0, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::spawn);
        //set_vfunc(0x008A5B94, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::create_ped_actor);
        SET_JUMP(0x006C30A0, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::sub_6C2EA0);
        REDIRECT(0x006D1C1D, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::sub_6B9B60);
        REDIRECT(0x006C2EE7, address);
    }

    REDIRECT(0x006D1B92, &ped_spawner::init);

    REDIRECT(0x006D1B74, &ped_spawner::cleanup);

    REDIRECT(0x006D86DA, &ped_spawner::advance_peds);

    REDIRECT(0x006D1BEA, &ped_spawner::populate_lanes);
}
