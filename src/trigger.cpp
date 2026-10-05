#include "trigger.h"
#include "box_trigger.h"
#include "entity_trigger.h"
#include "point_trigger.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include <algorithm>

#include "vtbl.h"

VALIDATE_SIZE(trigger, 0x58);

trigger::trigger(string_hash a2) : signaller(true)
{
    TRACE("trigger::trigger");

    this->m_vtbl = 0x008887F8;
    this->set_flag_recursive(static_cast<entity_flag_t>(0x2000), true);
    assert(my_rel_po == nullptr);

    assert(my_abs_po == nullptr);

    assert(adopted_children == nullptr);

    assert(my_conglom_root == nullptr);

    this->field_10 = a2;
    this->field_48 = 0.0;
    this->m_next_trigger = nullptr;
    this->trigger_current_entities = nullptr;
    this->field_4C = {0};
}

trigger::~trigger()
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = 0x008887F8;
        m_next_trigger = nullptr;
        delete trigger_current_entities;
        trigger_current_entities = nullptr;
        my_rel_po = nullptr;
        my_abs_po = nullptr;
        adopted_children = nullptr;
        my_conglom_root = nullptr;
    } else {
        THISCALL(0x0056FE50, this);
    }
}

void trigger::update(trigger_struct *subjects, int subject_count)
{
#if STANDALONE_SYSTEM
    if (subjects == nullptr || subject_count <= 0) {
        field_4C = {0};
        return;
    }
    if (trigger_current_entities == nullptr) {
        trigger_current_entities = new _std::list<vhandle_type<entity>>{};
    }
    field_4C = {0};
    for (int index = 0; index < subject_count; ++index) {
        auto &subject = subjects[index];
        auto *candidate = subject.handle.get_volatile_ptr();
        if (candidate == nullptr || !subject.field_10) {
            continue;
        }
        bool inside = false;
        if (m_vtbl == 0x0088A0B8) {
            auto *box = static_cast<box_trigger *>(this);
            box->update_center();
            inside = box->triggered(subject.position);
        } else if (m_vtbl == 0x00889F30) {
            auto *point = static_cast<point_trigger *>(this);
            inside = (subject.position - point->field_58).length2() <
                     field_48 * field_48;
        } else if (m_vtbl == 0x0088A240) {
            auto *entity_trigger_ptr = static_cast<entity_trigger *>(this);
            auto *center_entity = entity_trigger_ptr->get_ent();
            if (center_entity != nullptr) {
                const auto center =
                    center_entity->get_abs_position() + entity_trigger_ptr->field_5C;
                inside = (subject.position - center).length2() <
                         field_48 * field_48;
            }
        }

        auto existing = std::find(trigger_current_entities->begin(),
                                  trigger_current_entities->end(), subject.handle);
        if (inside) {
            field_4C = subject.handle;
            if (existing == trigger_current_entities->end()) {
                trigger_current_entities->push_back(subject.handle);
            }
        } else if (existing != trigger_current_entities->end()) {
            trigger_current_entities->erase(existing);
        }
    }
#else
    THISCALL(0x0053C470, this, subjects, subject_count);
#endif
}

void trigger::set_use_any_char(bool a2)
{
    auto v2 = this->field_4;
    if (a2) {
        this->field_4 = (v2 | 0x10);
    } else {
        this->field_4 = (v2 & (~0x10));
    }
}

void trigger::set_sees_dead_people(bool a2)
{
    auto v2 = this->field_4;
    if (a2) {
        this->field_4 = (v2 | 0x20000);
    } else {
        this->field_4 = (v2 & (~0x20000));
    }
}

entity *trigger::get_triggered_ent()
{
    auto v3 = this->field_4C;
    if (v3.get_volatile_ptr() == nullptr) {
        return nullptr;
    }

    v3 = this->field_4C;
    auto *result = v3.get_volatile_ptr();
    if (result == nullptr) {
        this->field_4C = {0};
    }

    return result;
}

void trigger::set_multiple_entrance(bool enabled)
{
    if (enabled) {
        if ((field_4 & 0x20) == 0) {
            field_4 |= 0x20;
            trigger_current_entities = new _std::list<vhandle_type<entity>>{};
        }
    } else if ((field_4 & 0x20) != 0) {
        field_4 &= ~0x20u;
        delete trigger_current_entities;
        trigger_current_entities = nullptr;
    }
}

bool trigger::is_point_trigger() const
{
    return m_vtbl == 0x00889F30;
}

bool trigger::is_box_trigger() const
{
    return m_vtbl == 0x0088A0B8;
}

bool trigger::is_entity_trigger() const
{
    return m_vtbl == 0x0088A240;
}

vector3d trigger::get_position()
{
    if (is_point_trigger())
        return static_cast<point_trigger *>(this)->field_58;
    if (is_box_trigger()) {
        auto *box = static_cast<box_trigger *>(this);
        auto *owner = box->get_box_ent();
        return owner != nullptr ? owner->get_abs_position() : box->field_5C;
    }
    if (is_entity_trigger()) {
        auto *owner = static_cast<entity_trigger *>(this)->get_ent();
        if (owner != nullptr)
            return owner->get_abs_position();
        const auto unset = bit_cast<float>(0xFFFFFFFFu);
        return vector3d{unset, unset, unset};
    }
    return ZEROVEC;
}

bool trigger::contains(const vector3d &position)
{
    if (is_point_trigger())
        return (position - static_cast<point_trigger *>(this)->field_58).length2() < field_48 * field_48;
    if (is_box_trigger())
        return static_cast<box_trigger *>(this)->triggered(position);
    if (is_entity_trigger())
        return (position - static_cast<entity_trigger *>(this)->field_5C).length2() < field_48 * field_48;
    return false;
}
