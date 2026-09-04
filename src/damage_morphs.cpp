#include "damage_morphs.h"

#include "camera.h"
#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "grenade.h"
#include "memory.h"
#include "ngl.h"
#include "oldmath_po.h"
#include "variables.h"

VALIDATE_SIZE(balanced_tree::tree_node, 0x20);

#if !STANDALONE_SYSTEM

int &damage_morphs::allocations_intercept_reference_count = var<int>(0x0095A760);

damage_morph_memory_pool &damage_morphs::write_combine_pool = var<damage_morph_memory_pool>(0x0095ABA4);

damage_morph_memory_pool &damage_morphs::normal_pool = var<damage_morph_memory_pool>(0x00921AC8);

balanced_tree &damage_morphs::registration_tree = var<balanced_tree>(0x0095AB98);

#else

int &damage_morphs::allocations_intercept_reference_count = []() -> auto & {
    static int g_allocations_intercept_reference_count{};
    return g_allocations_intercept_reference_count;
}();

damage_morph_memory_pool &damage_morphs::write_combine_pool = []() -> auto & {
    static damage_morph_memory_pool g_write_combine_pool{0};
    return g_write_combine_pool;
}();

damage_morph_memory_pool &damage_morphs::normal_pool = []() -> auto & {
    static damage_morph_memory_pool g_normal_pool{0x1400};
    return g_normal_pool;
}();

balanced_tree &damage_morphs::registration_tree = []() -> auto & {
    static balanced_tree g_registration_tree{};
    return g_registration_tree;
}();

#endif

damage_morph_memory_pool::damage_morph_memory_pool(int a2)
{
    this->allocation_list = nullptr;
    this->list_end = nullptr;
    this->field_8 = a2;
    this->memory_pool = nullptr;
}

void damage_morph_memory_pool::init()
{
    assert(this->memory_pool == nullptr);

    this->memory_pool = arch_memalign(4, this->field_8);

    this->field_10 = (int)this->memory_pool + this->field_8;
    this->field_14 = (int)this->memory_pool;
    this->field_18 = this->field_10;
}

void *damage_morph_memory_pool::memalloc(int a2, int a3)
{
    int v4 = this->field_10;
    int v5 = this->field_14;
    auto v6 = a3 + v5 + a2;
    if (v6 >= v4) {
        v5 = int(this->memory_pool);
        v6 = a3 + v5 + a2;
    }

    this->field_14 = v6;
    auto *v7 = (void *)(a2 - v5 % a2 + v5);
    auto *v8 = (int *)arch_memalign(4u, 12u);
    *v8 = (int)v7;
    *((char *)v8 + 4) = 1;
    v8[2] = 0;
    if (this->allocation_list != nullptr) {
        this->list_end[2] = (int)v8;
    } else {
        this->allocation_list = v8;
    }

    this->list_end = v8;
    return v7;
}

bool balanced_tree::retrieve(int a2, int *a3)
{
    auto *v3 = (int *)this->field_0;
    if (!this->field_0) {
        return false;
    }

    while (*v3 != a2) {
        if (*v3 >= a2)
            v3 = (int *)v3[3];
        else
            v3 = (int *)v3[4];
        if (!v3)
            return 0;
    }
    *a3 = v3[1];
    return true;
}

bool balanced_tree::remove(int a2)
{
    return (bool)THISCALL(0x004C5120, this, a2);
}

void damage_morphs::init_memory_pools()
{
    if constexpr (1) {
        normal_pool.init();

        write_combine_pool.init();
    } else {
        CDECL_CALL(0x004CE0E0);
    }
}

bool damage_morphs::intercepting_allocations()
{
    return allocations_intercept_reference_count > 0;
}

bool damage_morphs::is_subject_off_screen(actor *a1)
{
    auto *v2 = g_game_ptr->get_current_view_camera(0);
    if (v2 == nullptr) {
        return false;
    }

    auto &v17 = v2->get_abs_po();
    auto camera_facing = v17.get_z_facing();

    assert(camera_facing.length() > 0.999f && camera_facing.length() < 1.001f);

    vector3d v13 = v17.m[3];

    v17 = a1->get_abs_po();
    vector3d v5 = v17.m[3] - v13;
    v5.normalize();
    return 0.25f > dot(camera_facing, v5);
}

bool damage_morphs::unregister_mesh_copy(int a1)
{
    int v4;
    damage_morphs::registration_tree.retrieve(a1, &v4);
    auto *v1 = (int *)v4;
    if (bit_cast<vhandle_type<actor> *>(v4)->get_volatile_ptr()) {
        auto *v2 = (grenade *)((vhandle_type<actor> *)v1)->get_volatile_ptr();
        v2->get_mesh();
        v2->sub_4D6B10(v1[1]);
    }

    ++allocations_intercept_reference_count;
    nglDestroyMesh((nglMesh *)v1[2]);
    --allocations_intercept_reference_count;
    operator delete(v1);
    return registration_tree.remove(a1);
}

void *damage_morphs::memalloc(int a1, int a2, bool a3)
{
    if constexpr (1) {
        auto v3 = a1 + a2;
        auto *v4 = &damage_morphs::write_combine_pool;
        if (!a3) {
            v4 = &damage_morphs::normal_pool;
        }

        auto *v5 = (vhandle_type<actor> *)a3;
        while (1) {
            auto v6 = v4->field_14;  // damage_morph_memory_pool::check_requirements(int)
            auto v7 = v4->field_18;

            int v8;
            if (v6 >= v7) {
                if ((int)(v4->field_10 - v6) > v3) {
                    return v4->memalloc(a1, a2);
                }

                v8 = v7 - int(v4->memory_pool);
            } else {
                v8 = v7 - v6;
            }

            if (v8 > v3) {
                return v4->memalloc(a1, a2);
            }

            if (damage_morphs::registration_tree.field_4 == nullptr) {
                return nullptr;
            }

            auto v9 = damage_morphs::registration_tree.field_4->field_0;
            auto *v10 = damage_morphs::registration_tree.field_0;
            if (damage_morphs::registration_tree.field_0) {
                while (v10->field_0 != v9) {
                    if (v10->field_0 >= v9) {
                        v10 = v10->field_C;
                    } else {
                        v10 = v10->field_10;
                    }

                    if (v10 == nullptr) {
                        goto LABEL_18;
                    }
                }

                v5 = CAST(v5, v10->field_4);
            }

        LABEL_18:
            auto *v11 = v5->get_volatile_ptr();

            if (v11 != nullptr) {
                damage_morphs::is_subject_off_screen(v11);
            }

            damage_morphs::unregister_mesh_copy(v9);
        }

    } else {
        return (void *)CDECL_CALL(0x004F0E80, a1, a2, a3);
    }
}

void damage_morphs::memfree(void *a1)
{
    CDECL_CALL(0x004CE140, a1);
}
