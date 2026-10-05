#include "decal_data_interface.h"

#include "anim_event.h"
#include "common.h"
#include "conglom.h"
#include "entity_base_vhandle.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "memory.h"
#include "vtbl.h"

VALIDATE_SIZE(decal_data_interface, 0x80);
#if STANDALONE_SYSTEM
static _std::vector<decal_data_interface *> *&standalone_decal_interfaces()
{
    static _std::vector<decal_data_interface *> *interfaces = nullptr;
    return interfaces;
}
#endif
VALIDATE_OFFSET(decal_data_interface, field_38, 0x38);

decal_data_interface::decal_data_interface(conglomerate *a2) : conglomerate_interface(a2)
{
    this->m_vtbl = 0x00883DD4;

    this->field_C = false;
    this->field_D = false;

    this->field_10 = a2;
    this->constructor_common();
}

bool decal_data_interface::is_dynamic() const
{
    return this->dynamic;
}

void decal_data_interface::frame_advance_all_decal_interfaces(Float a1)
{
#if STANDALONE_SYSTEM
    auto *interfaces = standalone_decal_interfaces();
    if (interfaces == nullptr) {
        return;
    }
    for (auto *interface_ptr : *interfaces) {
        if (interface_ptr == nullptr || interface_ptr->m_vtbl != 0x00883DD4) {
            continue;
        }

        auto *address = get_vfunc(interface_ptr->m_vtbl, 0x34);
        if (address != nullptr) {
            void(__fastcall * frame_advance)(decal_data_interface *, void *, Float) = CAST(frame_advance, address);
            frame_advance(interface_ptr, nullptr, a1);
        }
    }
#else
    CDECL_CALL(0x004D1CC0, a1);
#endif
}

void decal_data_interface::add_to_decal_ifc_list()
{
#if STANDALONE_SYSTEM
    auto &interfaces = standalone_decal_interfaces();
    if (interfaces == nullptr) {
        auto *mem = mem_alloc(sizeof(*interfaces));
        interfaces = new (mem) _std::vector<decal_data_interface *>{};
    }
    interfaces->push_back(this);
#else
    if (all_decal_interfaces == nullptr) {
        auto *mem = mem_alloc(sizeof(_std::vector<decal_data_interface *>));
        all_decal_interfaces = new (mem) _std::vector<decal_data_interface *>{};
    }
    all_decal_interfaces->push_back(this);
#endif
}

void decal_data_interface::remove_from_decal_ifc_list()
{
#if STANDALONE_SYSTEM
    auto &interfaces = standalone_decal_interfaces();
    for (auto it = interfaces->begin(); it != interfaces->end(); ++it) {
        if (*it == this) {
            interfaces->erase(it);
            break;
        }
    }
    if (interfaces->empty()) {
        interfaces->~vector();
        mem_dealloc(interfaces, sizeof(*interfaces));
        interfaces = nullptr;
    }
#else
    THISCALL(0x004D5F80, this);
#endif
}

void terrain_fx_callback(event *the_event, entity_base_vhandle, void *a3)
{
    assert(the_event != nullptr);

    auto v3 = bit_cast<anim_event *>(the_event)->field_C;

    void(__fastcall * sub_509380)(void *, void *edx, string_hash *) = CAST(sub_509380, 0x00509380);
    sub_509380(a3, nullptr, &v3);
}

void decal_data_interface::release_ifc()
{
    this->destructor_common();
}

void decal_data_interface::constructor_common()
{
    this->field_14 = nullptr;
    this->field_C = true;

    if (this->field_D) {
        auto *mem = mem_alloc(sizeof(_std::vector<entity *>));
        this->field_14 = new (mem) _std::vector<entity *>{};
        auto v6 = this->my_conglomerate->get_my_handle();
        this->field_18 = event_manager::add_callback(event::TERRAIN_FX, v6, terrain_fx_callback, this->field_14, false);
    }
    this->add_to_decal_ifc_list();
}

void decal_data_interface::destructor_common()
{
    if (this->field_D) {
        auto finalize = [](auto *self) -> void {
            if (self != nullptr) {
                self->clear();
                mem_dealloc(self, sizeof(*self));
            }
        };

        finalize(this->field_14);
        this->field_14 = nullptr;

        auto *v2 = this->my_conglomerate;
        auto v3 = this->field_18;
        auto v4 = v2->get_my_handle();
        event_manager::remove_callback(v3, event::TERRAIN_FX, v4);
    }

    this->remove_from_decal_ifc_list();
    for (auto i = 0; !this->field_C; ++i) {
        if (i >= 9)
            break;
    }
}
