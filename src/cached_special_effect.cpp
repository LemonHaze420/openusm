#include "cached_special_effect.h"

#include "common.h"
#include "func_wrapper.h"
#include "fx_cache.h"
#include "memory.h"
#include "parse_generic_mash.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"
#include "handheld_item.h"
#include "oldmath_po.h"
#include "resource_manager.h"
#include "script.h"
#include "sound_and_pfx_interface.h"
#include "vtbl.h"
#include <cstdlib>

#include <algorithm>
#include <functional>
#include <new>

VALIDATE_SIZE(cached_special_effect, 0x40u);

cached_special_effect::cached_special_effect()
{
    TRACE("cached_special_effect::cached_special_effect");

    this->field_10 = vector3d{1.0f, 0.0f, 0.0f};
    this->field_8.set_type(RESOURCE_KEY_TYPE_ENTITY);
    field_1C = nullptr;
    field_20 = nullptr;
    field_24 = {};
    field_34 = 0;
    field_36 = 0;
    this->field_28 = nullptr;
    this->field_2C = -1.0f;
    this->field_30 = nullptr;
    this->field_3C = false;
    this->field_3D = false;
    this->field_38 = 5;
}

cached_special_effect::~cached_special_effect()
{
    if (field_30 != nullptr) {
        if (--field_30->field_0 == 0) {
            field_30->~fx_cache();
            mem_dealloc(field_30, sizeof(*field_30));
        }
        field_30 = nullptr;
    }
    if (field_1C != nullptr && field_3C)
        ::operator delete[](field_1C);
    if (field_20 != nullptr && field_3D)
        ::operator delete[](field_20);
}

void cached_special_effect::initialize() {}

void cached_special_effect::frame_advance(Float a2)
{
    auto *v2 = this->field_30;
    if (v2 != nullptr) {
        v2->frame_advance(a2);
    }
}

void cached_special_effect::spawn(bool a1, const vector3d &a2, const vector3d &a3, handheld_item *a6, entity_base *a7,
                                  entity_base *a8, const vector3d &a9, bool a10, bool a11)
{
    TRACE("cached_special_effect::spawn");

    if constexpr (STANDALONE_SYSTEM) {
        kill(nullptr, false);
        if (field_8.m_hash.source_hash_code != 0) {
            po transform = po_identity_matrix;
            if (a8 != nullptr && a10) {
                transform.set_position(a9);
            } else {
                transform.set_facing(a3);
                if (field_10.y > 0.0f)
                    std::rand();
                transform.set_position(a2);
                if (a8 != nullptr) {
                    auto inverse = a8->get_abs_po().inverse();
                    transform.set_from_ptr_to_po_world(ptr_to_po{&transform.m, &inverse->m});
                }
            }
            if (!a1 && field_30 != nullptr && !field_30->field_8.empty()) {
                auto &entry = *field_30->field_10;
                field_28 = entry.field_8.get_volatile_ptr();
                entry.field_0 = field_10.x;
                entry.field_4 = field_10.z * field_10.x;
                auto color = field_28->get_render_color();
                color.set_alpha(255);
                field_28->set_render_color(color);
                if (++field_30->field_10 == field_30->field_8.data() + field_30->field_8.size())
                    field_30->field_10 = field_30->field_8.data();
            } else {
                if (a6 != nullptr)
                    resource_manager::push_resource_context(a6->m_resource_context);
                mString path{"fx\\"};
                field_28 = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
                    field_8.m_hash, make_unique_entity_id(), transform, path, 524417, nullptr);
                if (field_28 != nullptr && field_28->is_an_actor())
                    field_2C = field_28->get_visual_radius();
                if (a6 != nullptr)
                    resource_manager::pop_resource_context();
            }
            field_28->set_abs_po(transform);
            if (a8 != nullptr)
                field_28->set_parent(a8);
            field_28->update_abs_po(false);
            if ((field_28->field_4 & 4) != 0) {
                auto restart = reinterpret_cast<void(__fastcall *)(entity *, void *)>(
                    get_vfunc(field_28->m_vtbl, 0x2B4));
                restart(field_28, nullptr);
            }
            field_28->set_active(true);
            auto reset = reinterpret_cast<void(__fastcall *)(entity *, void *, int)>(
                get_vfunc(field_28->m_vtbl, 0x208));
            reset(field_28, nullptr, 0);
            field_28->compute_sector(g_world_ptr->the_terrain, false, nullptr);
            auto visible = reinterpret_cast<void(__fastcall *)(entity *, void *, bool, bool)>(
                get_vfunc(field_28->m_vtbl, 0x44));
            visible(field_28, nullptr, false, false);
            if (field_28->is_renderable())
                visible(field_28, nullptr, true, false);
            field_28->update_proximity_maps();
            if (!a1) {
                if (field_30 == nullptr || field_30->field_8.empty())
                    g_world_ptr->ent_mgr.make_time_limited(field_28, field_10.x);
                field_28 = nullptr;
            }
        }
        if (field_1C != nullptr && *field_1C != '\0' &&
            find_func_and_spawn_new_thread(a6, string_hash(field_1C)) != nullptr) {
            if (a6 != nullptr)
                script::push_arg(a6);
            script::push_arg(a7);
            script::push_arg(a2);
            script::push_arg(a3);
            script::exec_thread(true);
        }
        if (field_0.m_hash.source_hash_code != 0 && a11 && field_36 == 0 &&
            a6 != nullptr && a6->has_sound_and_pfx_ifc()) {
            auto *ifc = a6->my_sound_and_pfx_interface;
            entity_base *owner = nullptr;
            if (a1) {
                auto get_owner = reinterpret_cast<entity_base *(__fastcall *)(handheld_item *, void *)>(
                    get_vfunc(a6->m_vtbl, 0x2DC));
                owner = get_owner(a6, nullptr);
            }
            if (owner != nullptr && owner->has_sound_and_pfx_ifc()) {
                field_24 = ifc->play_sound_grp_at(field_0.m_hash, nullptr,
                    1.0f, 1.0f, 1.0f, -1.0f, -1.0f, owner->my_sound_and_pfx_interface);
            } else {
                auto sound = ifc->play_sound_grp_at(field_0.m_hash, &a2,
                    1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
                if (a1)
                    field_24 = sound;
            }
        }
        if (++field_36 > field_34)
            field_36 = 0;
    } else {
        THISCALL(0x004EFC00, this, a1, &a2, &a3, a6, a7, a8, &a9, a10, a11);
    }
}

void cached_special_effect::kill(handheld_item *owner, bool run_script)
{
    if (auto *sound = field_24.get_sound_instance_ptr())
        sound->stop();
    field_24 = {};
    if (run_script && field_20 != nullptr && *field_20 != '\0' && owner != nullptr &&
        find_func_and_spawn_new_thread(owner, string_hash(field_20)) != nullptr) {
        script::push_arg(owner);
        script::exec_thread(true);
    }
    if (field_28 != nullptr) {
        g_world_ptr->ent_mgr.make_time_limited(field_28, 0.0f);
        field_28 = nullptr;
    }
}

void cached_special_effect::fill_cache()
{
    TRACE("cached_special_effect::fill_cache");

    if constexpr (STANDALONE_SYSTEM) {
        if (field_8.m_hash.source_hash_code == 0)
            return;
        field_8.set_type(RESOURCE_KEY_TYPE_ENTITY);
        if (field_30 == nullptr) {
            field_30 = new (mem_alloc(sizeof(fx_cache))) fx_cache{};
            ++field_30->field_0;
        }
        auto &entries = field_30->field_8;
        if (field_38 > entries.size()) {
            auto *storage = static_cast<fx_cache_ent *>(
                ::operator new[](sizeof(fx_cache_ent) * field_38));
            for (int i = 0; i < field_38; ++i)
                new (storage + i) fx_cache_ent{};
            std::copy_n(entries.m_data, entries.size(), storage);
            if (entries.m_data != nullptr)
                ::operator delete[](entries.m_data);
            const auto previous_size = entries.m_size;
            entries.m_data = storage;
            entries.m_size = static_cast<uint16_t>(field_38);
            for (int i = previous_size; i < field_38; ++i) {
                mString path{"fx\\"};
                auto *effect = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
                    field_8.m_hash, make_unique_entity_id(), po_identity_matrix,
                    path, 524417, nullptr);
                effect->set_active(false);
                auto visible = reinterpret_cast<void(__fastcall *)(entity *, void *, bool, bool)>(
                    get_vfunc(effect->m_vtbl, 0x44));
                visible(effect, nullptr, false, false);
                if (std::equal_to<float>{}(field_2C, -1.0f) && effect->is_an_actor())
                    field_2C = effect->get_visual_radius();
                entries[i].field_8 = {effect->my_handle};
                entries[i].field_0 = -1.0f;
                entries[i].field_4 = 0.0f;
            }
        }
        field_30->field_10 = entries.m_data;
    } else {
        THISCALL(0x004D4E10, this);
    }
}

void cached_special_effect::un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("cached_special_effect::un_mash");

    if constexpr (1) {
        this->field_24 = 0;
        this->field_30 = nullptr;
        this->field_28 = nullptr;

        a4->rebase_shared(4u);

        auto v5 = *a4->get_from_shared<int>();

        this->field_1C = a4->get_from_shared<char>(v5);

        a4->rebase_shared(4u);

        auto v9 = *a4->get_from_shared<int>();

        this->field_20 = a4->get_from_shared<char>(v9);

        a4->rebase_shared(4u);

        this->field_30 = a4->get_from_shared<fx_cache>();

        auto *v13 = this->field_30;
        v13->un_mash(a2, this, v13, a4);

        this->field_3C = false;
        this->field_3D = false;
    } else {
        THISCALL(0x004D3650, this, a2, a3, a4);
    }
}

void cached_special_effect::release_mem()
{
    auto *v2 = this->field_30;
    if (v2 != nullptr) {
        if (--v2->field_0 == 0) {
            v2->~fx_cache();
            mem_dealloc(v2, sizeof(*v2));
        }
        this->field_30 = nullptr;
    }

    if (this->field_1C != nullptr && this->field_3C) {
        operator delete[](this->field_1C);
        this->field_1C = nullptr;
    }

    if (this->field_20 != nullptr && this->field_3D) {
        operator delete[](this->field_20);
        this->field_20 = nullptr;
    }

    if (this->field_28 != nullptr) {
        g_world_ptr->ent_mgr.destroy_entity(this->field_28);
        this->field_28 = nullptr;
    }
}

void *__fastcall cached_special_effect_constructor(void *mem)
{
    return new (mem) cached_special_effect{};
}

void cached_special_effect_patch()
{
    {
        auto *func = cached_special_effect_constructor;
        REDIRECT(0x004DEABA, func);
    }

    {
        FUNC_ADDRESS(address, &cached_special_effect::un_mash);
        REDIRECT(0x004D9EF8, address);
    }

    {
        FUNC_ADDRESS(address, &cached_special_effect::fill_cache);
        REDIRECT(0x0054AC6D, address);
        REDIRECT(0x0054AD0D, address);
    }
}
