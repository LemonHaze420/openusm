#include "box_trigger.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(box_trigger, 0xECu);

box_trigger::box_trigger(string_hash a2, const vector3d &a3) : trigger(a2)
{
    this->m_vtbl = 0x0088A0B8;
    this->field_58 = {0};
    this->field_5C = a3;
    this->field_68 = a3;

    this->update_center();
}

box_trigger::box_trigger(string_hash a2, entity_base *a3) : trigger(a2)
{
    this->m_vtbl = 0x0088A0B8;
    this->field_58 = a3->get_my_handle();
    this->field_5C = ZEROVEC;
    this->field_68 = ZEROVEC;

    a3->set_flag_recursive(static_cast<entity_flag_t>(0x400u), true);
    this->update_center();
}

entity_base *box_trigger::get_box_ent() const
{
    return this->field_58.get_volatile_ptr();
}

bool box_trigger::triggered(const vector3d &a2)
{
    auto v2 = a2 - this->field_68;
    auto v3 = v2.length2();

    if (this->field_48 * this->field_48 <= v3) {
        return false;
    }

    assert(box.bbox.size() != ZEROVEC);

    auto *v6 = this->field_58.get_volatile_ptr();
    if (v6 == nullptr) {
        return this->box.sub_55EDB0(a2, this->field_5C);
    }

    return this->box.sub_55EE20(a2, v6->get_abs_po());
}

void box_trigger::set_box_info(const convex_box &a2)
{
    THISCALL(0x0050CD30, this, &a2);
}

bool box_trigger::_is_point_trigger()
{
    return false;
}

bool box_trigger::_is_box_trigger()
{
    return true;
}

bool box_trigger::_is_entity_trigger()
{
    return false;
}

bool box_trigger::get_bounding_sphere(vector3d *a2, float *a3)
{
    auto *v4 = this->field_58.get_volatile_ptr();
    auto v5 = (v4 != nullptr ? v4->get_abs_position() : this->field_5C);

    *a2 = v5;
    *a3 = this->field_48;
    return true;
}

void box_trigger::update_center()
{
    if constexpr (0) {
    } else {
        THISCALL(0x0050CE70, this);
    }
}
