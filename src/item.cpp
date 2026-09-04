#include "item.h"

#include "base_ai_data.h"
#include "common.h"
#include "dynamic_rtree.h"
#include "func_wrapper.h"
#include "signaller.h"

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

void item::frame_advance_all_items(Float a3)
{
    CDECL_CALL(0x004CD9D0, a3);
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
