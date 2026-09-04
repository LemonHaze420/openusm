#include "event_type.h"

#include "binary_search_array_cmp.h"
#include "common.h"
#include "event.h"
#include "event_callback.h"
#include "event_recipient_entry.h"
#include "event_type.h"
#include "func_wrapper.h"

VALIDATE_SIZE(event_type, 0x2C);

event_type::event_type(string_hash a2, bool a3)
{
    if constexpr (0) {
    } else {
        THISCALL(0x004E18B0, this, a2, a3);
    }
}

event_type::~event_type()
{
    TRACE("event_type::~event_type");

    this->clear();

    if (this->event_to_raise != nullptr) {
        this->event_to_raise->_finalize(true);
    }

    this->field_1C.clear();
    this->field_8.clear();
}

void *event_type::operator new(size_t sz)
{
    return mem_alloc(sz);
}

void event_type::operator delete(void *ptr, size_t sz)
{
    mem_dealloc(ptr, sz);
}

void event_type::clear()
{
    TRACE("event_type::clear");

    this->field_0 = {0};

    for (auto &entry : this->field_8) {
        if (entry != nullptr) {
            delete entry;
        }
    }

    this->field_8.clear();
    this->field_18 = false;
    this->clear_callbacks();
}

void event_type::clear_callbacks()
{
    TRACE("event_type::clear_callbacks");

    if constexpr (1) {
        for (auto &entry : this->field_8) {
            entry->clear_callbacks();
        }

        for (auto &cb : this->field_1C) {
            if (cb != nullptr) {
                cb->_finalize(true);
            }
        }

        this->field_1C.clear();
    } else {
        THISCALL(0x004D1ED0, this);
    }
}

event_recipient_entry *event_type::find_recipient_entry(entity_base_vhandle a2)
{
    int a5 = -1;
    if (!this->field_18) {
        std::sort(this->field_8.begin(), this->field_8.end());
        this->field_18 = true;
    }

    auto v7 = this->field_8.size();

    if (binary_search_array_cmp<entity_base_vhandle, event_recipient_entry *>(
            &a2, &this->field_8[0], 0, v7, &a5, compare_deref<entity_base_vhandle, event_recipient_entry *>)) {
        return this->field_8[a5];
    } else {
        return nullptr;
    }
}

event_recipient_entry *event_type::create_recipient_entry(entity_base_vhandle a2)
{
    if constexpr (0) {
        auto *ret_val = this->find_recipient_entry(a2);
        if (ret_val == nullptr) {
            ret_val = new event_recipient_entry{a2, false};
            assert(ret_val != nullptr);

            if (ret_val != nullptr) {
                this->field_8.push_back(ret_val);
                this->field_18 = false;
            }
        }

        return ret_val;
    } else {
        return (event_recipient_entry *)THISCALL(0x004EE620, this, a2);
    }
}

void event_type::raise_event(entity_base_vhandle a2, event *a3)
{
    if constexpr (0) {
    } else {
        THISCALL(0x004EE6C0, this, a2, a3);
    }
}

bool event_type::callback_exists(int a2) const
{
    if (a2 == 0) {
        return false;
    }

    for (auto &v1 : this->field_1C) {
        if (v1->id == a2) {
            return true;
        }
    }

    for (auto &v1 : this->field_8) {
        if (v1->callback_exists(a2)) {
            return true;
        }
    }

    return false;
}

void event_type::remove_default_callback(unsigned int a2)
{
    THISCALL(0x004D4320, this, a2);
}

void event_type::clear_script_callbacks(entity_base_vhandle a2, script_executable *a3)
{
    for (auto &v1 : this->field_8) {
        if (v1->field_0 == a2) {
            v1->clear_script_callbacks(a3);
        }
    }
}

bool event_type::garbage_collect()
{
    TRACE("event_type::garbage_collect");

    bool(__fastcall * func)(void *) = CAST(func, 0x004D65B0);
    return func(this);
}
