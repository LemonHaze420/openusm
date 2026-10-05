#include "colmesh.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "osassert.h"
#include "parse_generic_mash.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

Var<std::intptr_t> collision_mesh_v_table{0x0095A668};
#if STANDALONE_SYSTEM
namespace {
const bool collision_mesh_identity_initialized = [] {
    collision_mesh_v_table() = 0x00888E58;
    return true;
}();
}
#endif

VALIDATE_SIZE(cg_mesh, 0x18u);

cg_mesh::cg_mesh()
{
    this->m_vtbl = 0x00888E58;
    this->field_8 = false;
    this->field_9 = true;
    this->field_C = 2;
    this->data = nullptr;
    this->field_14 = 2;
}

collision_geometry *cg_mesh::make_instance(actor *a2)
{
#ifdef OPENUSM_XBPACK_V10
    return reinterpret_cast<collision_geometry *>(THISCALL(0x005267F0, this, a2));
#else
    auto *mem = mem_alloc(sizeof(cg_mesh));
    auto *result = new (mem) cg_mesh {};
    result->owner = a2;
    if (this->field_8 != result->field_8) {
        result->field_8 = this->field_8;
    }

    result->data = this->data;
    return result;
#endif
}

vector3d cg_mesh::get_local_space_bounding_sphere_center()
{
    return this->data->field_10[0].field_0;
}

float cg_mesh::get_bounding_sphere_radius()
{
    return this->data->field_10[0].field_C;
}

int cg_mesh::get_type()
{
    return collision_geometry::MESH;
}

void cg_mesh::_un_mash([[maybe_unused]] generic_mash_header *a2, [[maybe_unused]] void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("cg_mesh::un_mash");

#if STANDALONE_SYSTEM
    owner = nullptr;
    field_9 = false;
    const resource_key key = *a4->get_from_shared<resource_key>();
    int size = 0;
    data = static_cast<collision_mesh *>(static_cast<void *>(resource_manager::get_resource(key, &size, nullptr)));
    assert(data != nullptr && "Missing collision mesh in the active resource context");
    if (data->field_0[3] != 'Z') {
        assert(data->m_version == 0x10003F && "Unsupported collision mesh version");
        data->field_0[3] = 'Z';
    }
#elif defined(OPENUSM_XBPACK_V10)
    {
        collision_geometry::un_mash(a2, a3, a4);

        this->field_9 = false;

        static resource_key col_mesh_name {};

        std::memcpy(&col_mesh_name, a4->field_4, sizeof(resource_key));
        a4->field_4 += sizeof(resource_key);
        col_mesh_name.set_type(RESOURCE_KEY_TYPE_COLLISION_MESH);

        int size = 0;
        auto *resource = resource_manager::get_resource(col_mesh_name, &size, nullptr);
        
        if (resource == nullptr) {
            auto *str = col_mesh_name.m_hash.to_string();
            error("Couldn't acquire memory image '%s' for collision geometry.", str);
        }

        this->data = CAST(this->data, resource);
        if (this->data->field_0[3] != 'Z') {
            if (memcmp(this->data->field_0, "COLL", 4) != 0 && memcmp(this->data->field_0, "COLB", 4) != 0) {
                auto *str = col_mesh_name.m_hash.to_string();
                error("corruption collision mesh file %s", str);
            }

            if (this->data->m_version != 0x10003F) {
                auto *str = col_mesh_name.m_hash.to_string();
                error("unsupported collision mesh version in file %s", str);
            }

            this->data->field_0[3] = 'Z';
        }
    }
#else
    THISCALL(0x0053B100, this, a2, a3, a4);
#endif
}

void cg_mesh_patch()
{
    {
        FUNC_ADDRESS(address, &cg_mesh::_un_mash);
        set_vfunc(0x00888E8C, address);
    }
}
