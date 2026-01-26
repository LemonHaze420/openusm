#include "interactable_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "interaction.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(interactable_interface, 0x1C);
VALIDATE_ALIGNMENT(interactable_interface, 4);

Var<_std::vector<interactable_interface *>> interactable_interface::all_ifcs {0x0095A998};

interactable_interface::interactable_interface(actor *act) {
    THISCALL(0x004ED630, this, act);
}

interactable_interface::interactable_interface(from_mash_in_place_constructor *a2) {
    THISCALL(0x004ED5C0, this, a2);
}

void interactable_interface::add_interface_to_list(interactable_interface *who_to_add)
{
    TRACE("interactable_interface::add_interface_to_list");

    assert(std::find(all_ifcs().begin(), all_ifcs().end(), who_to_add) == all_ifcs().end());

    if constexpr (0) { 
        all_ifcs().push_back(who_to_add);
    } else {
        void (__fastcall *sub_5E4BD0)(void *, void *, void *, int, interactable_interface **) = CAST(sub_5E4BD0, 0x005E4BD0);
        sub_5E4BD0(
          &all_ifcs(),
          nullptr,
          all_ifcs().m_last,
          1,
          &who_to_add);
    }
}

void interactable_interface::frame_advance_all(Float a1) {
    TRACE("interactable_interface::frame_advance_all");

    if constexpr (1) {
        for ( auto &ifc : all_ifcs() ) {
            ifc->frame_advance(a1);
        }
    } else {
        CDECL_CALL(0x004D1C10, a1);
    }
}

void interactable_interface::frame_advance(Float a2) {
    for (uint16_t i = 0; i < this->field_4.m_size; ++i) {
        auto *v3 = this->field_4.m_data[i];
        auto v4 = v3->field_38 - a2;
        v3->field_38 = v4;
        if (v4 < 0.0f) {
            v3->field_38 = 0.0;
        }
    }
}

void interactable_interface::update_registrations()
{
    TRACE("interactable_interface::update_registrations");

    THISCALL(0x004D5BE0, this);
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

void interactable_interface_patch() {

    SET_JUMP(0x004DAFB0, interactable_interface::add_interface_to_list);

    SET_JUMP(0x004D1C10, interactable_interface::frame_advance_all);
}
