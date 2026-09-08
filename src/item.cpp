#include "item.h"

#include "base_ai_data.h"
#include "common.h"
#include "dynamic_rtree.h"
#include "func_wrapper.h"
#include "signaller.h"
#include "time_interface.h"
#include "vtbl.h"
#include "wds.h"

VALIDATE_SIZE(item, 0x100u);

Var<item *> item::inactive_items{0x0095A730};

Var<item *> item::active_items{0x0095A734};

item::item(const string_hash &a2, uint32_t a3) : actor(a2, a3)
{
    this->field_C8 = false;
    this->field_C9 = false;
    this->field_CA = false;
    this->field_CB = false;
    this->field_CC = false;
    this->field_D4 = {};
    this->field_F4 = 0.0;
}

item::~item()
{
    this->field_D4.~mString();
}

void item::frame_advance_all_items(Float elapsed)
{
    for (auto *current = active_items(); current != nullptr;) {
        auto *next = current->field_C0;
        const float scale = current->field_58 != nullptr
            ? static_cast<float>(current->field_58->sub_4ADE50())
            : g_world_ptr->field_158.field_0;
        if (current->m_vtbl != 0) {
            auto *address = get_vfunc(current->m_vtbl, 0x1A4);
            if (address != nullptr) {
                void(__fastcall *frame_advance)(item *, void *, Float) =
                    CAST(frame_advance, address);
                frame_advance(current, nullptr, Float{scale * elapsed.value});
            }
        }
        current = next;
    }
}

bool item::is_same_item(const item &a2)
{
    bool result;

    auto v2 = this->field_D0;
    if (v2 != a2.field_D0) {
        return false;
    }

    switch (v2) {
    case 1:
    case 2:
    case 9:
        return false;
    case 3:
    case 4:
        result = this->field_D4.operator==(a2.field_D4.c_str());
        break;
    default:
        result = true;
        break;
    }

    return result;
}

void item::remove_from_list()
{
    auto *v1 = this->field_C4;
    if (v1 != nullptr) {
        v1->field_C0 = this->field_C0;
    } else if (item::inactive_items() == this) {
        item::inactive_items() = this->field_C0;
    } else if (item::active_items() == this) {
        item::active_items() = this->field_C0;
    }

    auto *v2 = this->field_C0;
    if (v2 != nullptr) {
        v2->field_C4 = this->field_C4;
    }

    this->field_C0 = nullptr;
    this->field_C4 = nullptr;
}

bool item::give_to_entity(actor *a2)
{
    return (bool)THISCALL(0x004F7590, this, a2);
}

void item::release_mem()
{
    this->remove_from_list();
    this->field_D4.~mString();
    auto v2 = this->field_7C;
    if (v2 != nullptr) {
        v2->destruct_mashed_class();
    }

    collision_dynamic_rtree().remove_entity(this);
    actor::common_destruct();
    entity::release_mem();
}
