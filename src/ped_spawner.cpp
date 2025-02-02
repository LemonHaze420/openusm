#include "ped_spawner.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"

VALIDATE_SIZE(ped_spawner, 0x4C);

_std::vector<ped_spawner *> & ped_spawner::ped_spawner_list = var<_std::vector<ped_spawner *>>(0x0096D270);

ped_spawner::ped_spawner(int a2) : spawnable(vhandle_type<entity> {0})
{
    this->m_vtbl = 0x008A5B70;
    this->field_C = a2;
}

void ped_spawner::do_spawn(
        vector3d a2,
        vector3d a3,
        traffic_path_lane *a8,
        int a9,
        int a10,
        int a11)
{
    TRACE("ped_spawner::do_spawn");

    if constexpr (1)
    {
        if ( this->get_my_actor() != nullptr )
        {
            this->sub_6BBD30(a8);
            this->spawn(a2, a3);
        }
    }
    else
    {
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

    char v10[32] {};
    sprintf(v10, "PED_%u", this->field_C);

    entity *eb = nullptr;
    if ( (unsigned int)(rand() * 0.000061035156) )
    {
        static string_hash ped_fem_hash {int(to_hash("ped_fem"))};

        string_hash v7 {v10};
        eb = g_world_ptr->ent_mgr.acquire_entity(ped_fem_hash, v7, 129);
        this->field_44 = 2;
    }
    else
    {
        static string_hash ped_male_hash {int(to_hash("ped_male"))};

        string_hash v7 {v10};
        eb = g_world_ptr->ent_mgr.acquire_entity(ped_male_hash, v7, 129);
        this->field_44 = 1;
    }

    if ( eb != nullptr )
    {
        assert(eb->is_an_actor());

        eb->set_visible(false, false);

        assert(!eb->has_physical_ifc() && "Peds should not have physical ifc");
        if ( eb->has_physical_ifc() ) {
            eb->set_collisions_active(false, true);
        }

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

    CDECL_CALL(0x006D1960);
}

void ped_spawner::cleanup()
{
    TRACE("ped_spawner::cleanup");

    CDECL_CALL(0x006CDB50);
}

void ped_spawner::advance_peds(Float a1)
{
    TRACE("ped_spawner::advance_peds");

    CDECL_CALL(0x006D1B30, a1);
}

void ped_spawner::populate_lanes()
{
    TRACE("ped_spawner::populate_lanes");

    CDECL_CALL(0x006D0590);
}

ped_spawner * ped_spawner::assign_non_ped_actor(vhandle_type<actor> a2, int a3)
{
    return (ped_spawner *) CDECL_CALL(0x006CAC50, a2, a3);
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

