#include "event_type.h"

#include "binary_search_array_cmp.h"
#include "common.h"
#include "event.h"
#include "event_callback.h"
#include "event_recipient_entry.h"
#include "event_type.h"
#include "func_wrapper.h"
#include "vtbl.h"
#include "wds.h"
#include <new>

namespace {
void dispatch_callbacks(_std::list<event_callback *> &callbacks, event *payload, entity_base_vhandle handle)
{
    for (auto it = callbacks.begin(); it != callbacks.end();) {
        auto *callback = *it;
        if (!callback->field_C) {
            void (__fastcall *spawn)(event_callback *, void *, event *, entity_base_vhandle) =
                CAST(spawn, get_vfunc(callback->m_vtbl, 4));
            spawn(callback, nullptr, payload, handle);
        }
        if (!callback->field_C && callback->field_D) {
            callback->_finalize(true);
            it = callbacks.erase(it);
        } else {
            ++it;
        }
    }
}
}

VALIDATE_SIZE(event_type, 0x2C);

#if STANDALONE_SYSTEM
event_type::event_type(string_hash event_id, bool pollable)
    : field_0{event_id},
      event_to_raise{new (mem_alloc(sizeof(event))) event{event_id}},
      field_8{},
      field_18{false},
      field_1C{},
      field_28{pollable}
{
}
#else
event_type::event_type(string_hash event_id, bool pollable)
{
    THISCALL(0x004E18B0, this, event_id, pollable);
}
#endif

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
        std::sort(this->field_8.begin(), this->field_8.end(), [](const auto *left, const auto *right) {
            return left->field_0.field_0 < right->field_0.field_0;
        });
        this->field_18 = true;
    }

    auto v7 = this->field_8.size();
    if (v7 == 0)
        return nullptr;

    if (binary_search_array_cmp<entity_base_vhandle, event_recipient_entry *>(
            &a2, &this->field_8[0], 0, v7, &a5, compare_deref<entity_base_vhandle, event_recipient_entry *>)) {
        return this->field_8[a5];
    } else {
        return nullptr;
    }
}

event_recipient_entry *event_type::create_recipient_entry(entity_base_vhandle a2)
{
    if constexpr (STANDALONE_SYSTEM) {
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
    if constexpr (STANDALONE_SYSTEM) {
        event *payload = a3 != nullptr ? a3 : event_to_raise;
        auto *recipient = find_recipient_entry(a2);
        if (recipient == nullptr && field_28)
            recipient = create_recipient_entry(a2);
        if (recipient != nullptr) {
            recipient->clean_up_callbacks();
            void (__fastcall *raise)(event *, void *) = CAST(raise, get_vfunc(payload->m_vtbl, 0x18));
            raise(payload, nullptr);
            const int ticks = g_world_ptr->time_manager.field_C;
            if (recipient->field_24 != ticks) {
                recipient->field_20 = recipient->field_24;
                recipient->field_24 = ticks;
            }
            dispatch_callbacks(recipient->field_4, payload, a2);
        }
        dispatch_callbacks(field_1C, payload, a2);
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
    if constexpr (STANDALONE_SYSTEM) {
        for (auto it = field_1C.begin(); it != field_1C.end();) {
            auto *callback = *it;
            if (static_cast<unsigned int>(callback->id) == a2) {
                callback->_finalize(true);
                it = field_1C.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        THISCALL(0x004D4320, this, a2);
    }
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

    if constexpr (STANDALONE_SYSTEM) {
        return field_8.empty() && field_1C.empty() && !field_28;
    } else {
        bool(__fastcall *func)(void *) = CAST(func, 0x004D65B0);
        return func(this);
    }
}
