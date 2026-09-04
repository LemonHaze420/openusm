#include "decal_data_interface.h"

#include "anim_event.h"
#include "common.h"
#include "conglom.h"
#include "entity_base_vhandle.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "memory.h"

VALIDATE_SIZE(decal_data_interface, 0x80);

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
    CDECL_CALL(0x004D1CC0, a1);
}

void decal_data_interface::add_to_decal_ifc_list()
{
    if (all_decal_interfaces == nullptr) {
        auto *mem = mem_alloc(sizeof(_std::vector<decal_data_interface *>));
        all_decal_interfaces = new (mem) _std::vector<decal_data_interface *>{};
    }

    all_decal_interfaces->push_back(this);
}

void decal_data_interface::remove_from_decal_ifc_list()
{
    THISCALL(0x004D5F80, this);
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
