#include "interactable_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "interaction.h"
#include "trace.h"
#include "utility.h"
#include "entity.h"
#include "actor.h"
#include "variables.h"
#include "memory.h"
#include "vtbl.h"
#include <algorithm>

VALIDATE_SIZE(interactable_interface, 0x1C);
VALIDATE_ALIGNMENT(interactable_interface, 4);

Var<_std::vector<interactable_interface *>> interactable_interface::all_ifcs{0x0095A998};

Var<mVectorBasic<vhandle_type<actor>>> interactable_interface::throw_list{0x0095A958};
Var<mVectorBasic<vhandle_type<actor>>> interactable_interface::generic_list{0x0095AF50};

interactable_interface::interactable_interface(actor *act) : field_4(), field_18(false)
{
    if (!g_is_the_packer) {
        add_interface_to_list(this);
    }
    field_0 = act;
}

interactable_interface::interactable_interface(from_mash_in_place_constructor *tag) : field_4(tag), field_18(true)
{
    if (!g_is_the_packer) {
        add_interface_to_list(this);
    }
    field_0 = nullptr;
}

void interactable_interface::add_interface_to_list(interactable_interface *who_to_add)
{
    TRACE("interactable_interface::add_interface_to_list");

    assert(std::find(all_ifcs().begin(), all_ifcs().end(), who_to_add) == all_ifcs().end());

    all_ifcs().push_back(who_to_add);
}

void interactable_interface::frame_advance_all(Float a1)
{
    TRACE("interactable_interface::frame_advance_all");

    if constexpr (1) {
        for (auto &ifc : all_ifcs()) {
            ifc->frame_advance(a1);
        }
    } else {
        CDECL_CALL(0x004D1C10, a1);
    }
}

void interactable_interface::frame_advance(Float a2)
{
    for (uint16_t i = 0; i < this->field_4.m_size; ++i) {
        auto *v3 = this->field_4.m_data[i];
        auto v4 = v3->field_38 - a2;
        v3->field_38 = v4;
        if (v4 < 0.0f) {
            v3->field_38 = 0.0;
        }
    }
}

bool interactable_interface::has_enabled_interaction_of_this_kind(interaction_type_enum kind) const
{
    for (auto *entry : field_4) {
        if (entry->field_28.value == kind.value && entry->field_44 && entry->field_38 <= 0.0f) {
            return true;
        }
    }
    return false;
}

void interactable_interface::update_registrations()
{
    const auto register_actor = [this](mVectorBasic<vhandle_type<actor>> &list) {
        const vhandle_type<actor> handle{field_0->get_my_vhandle()};
        for (int i = 0; i < list.m_size; ++i) {
            if (list.m_data[i].field_0 == handle.field_0) {
                return;
            }
        }
        list.push_back(handle);
    };
    if (has_enabled_interaction_of_this_kind({3})) {
        register_actor(throw_list());
    }
    if (has_enabled_interaction_of_this_kind({1}) || has_enabled_interaction_of_this_kind({0}) ||
        has_enabled_interaction_of_this_kind({2})) {
        register_actor(generic_list());
    }
}

void interactable_interface::sub_4DAE90(actor *a2)
{
    this->field_0 = a2;
    this->update_registrations();
}

void interactable_interface::unmash(mash_info_struct *a2, void *)
{
    a2->unmash_class_in_place(this->field_4, this);
}

void interactable_interface::finalize(mash::allocation_scope scope)
{
    if (scope == mash::FROM_MASH) {
        auto unregister = [this](auto &list) {
            const auto handle = field_0->get_my_vhandle();
            for (int index = 0; index < list.m_size; ++index) {
                if (list.m_data[index].field_0 == handle) {
                    std::copy(list.m_data + index + 1, list.m_data + list.m_size, list.m_data + index);
                    --list.m_size;
                    break;
                }
            }
        };
        if (has_enabled_interaction_of_this_kind({3}))
            unregister(throw_list());
        if (has_enabled_interaction_of_this_kind({1}) || has_enabled_interaction_of_this_kind({0}) ||
            has_enabled_interaction_of_this_kind({2}))
            unregister(generic_list());
    }
    if (!g_is_the_packer) {
        auto &list = all_ifcs();
        auto found = std::find(list.begin(), list.end(), this);
        if (found != list.end())
            list.erase(found);
    }
}

void interactable_interface::release()
{
    const bool from_mash = field_18;
    finalize(from_mash ? mash::FROM_MASH : mash::ALLOCATED);
    if (field_4.field_10) {
        for (auto *value : field_4) {
            if (field_4.is_pointer_in_mash_image(value)) {
                auto destroy = reinterpret_cast<void(__fastcall *)(interaction *, void *)>(
                    get_vfunc(*reinterpret_cast<int *>(value), 0));
                destroy(value, nullptr);
            } else if (value != nullptr) {
                auto destroy = reinterpret_cast<void(__fastcall *)(interaction *, void *, bool)>(
                    get_vfunc(*reinterpret_cast<int *>(value), 8));
                destroy(value, nullptr, true);
            }
        }
    }
    if (!field_4.is_pointer_in_mash_image(field_4.m_data))
        mem_dealloc(field_4.m_data, field_4.m_max_size * sizeof(interaction *));
    field_4.m_data = nullptr;
    field_4.m_max_size = 0;
    field_4.m_size = 0;
    field_4.field_0 = 0;
    if (!from_mash)
        ::operator delete(this);
}

void interactable_interface_patch()
{
    SET_JUMP(0x004DAFB0, interactable_interface::add_interface_to_list);

    SET_JUMP(0x004D1C10, interactable_interface::frame_advance_all);
}
