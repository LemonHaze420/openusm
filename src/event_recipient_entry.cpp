#include "event_recipient_entry.h"

#include "code_event_callback.h"
#include "common.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "memory.h"
#include "trace.h"
#include "vm.h"
#include "script_event_callback.h"
#include "vtbl.h"
#include "chuck/vm/script_object.h"
#include "wds.h"
#include "chuck/vm/vm_executable.h"

VALIDATE_SIZE(event_recipient_entry, 0x28u);

event_recipient_entry::event_recipient_entry(entity_base_vhandle a2, bool)
{
    this->field_0 = a2;
    this->field_20 = 0;
    this->field_24 = 0;
}

event_recipient_entry::~event_recipient_entry()
{
    TRACE("event_recipient_entry::~event_recipient_entry");

    this->clear();

    this->field_10.clear();
    this->m_callbacks.clear();
}

void *event_recipient_entry::operator new(size_t size)
{
    return mem_alloc(size);
}

void event_recipient_entry::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}

void event_recipient_entry::clear()
{
    TRACE("event_recipient_entry::clear");

    this->field_0 = {0};
    this->field_20 = 0;
    this->field_24 = 0;
    this->clear_callbacks();
}

void event_recipient_entry::clear_callbacks()
{
    for (auto &cb : this->m_callbacks) {
        if (cb != nullptr) {
            cb->finalize(true);
        }
    }

    this->m_callbacks.clear();
}

int event_recipient_entry::add_callback(void (*cb)(event *, entity_base_vhandle, void *), void *a3, bool a4)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *new_callback = new code_event_callback{cb, a3, a4};
        assert(new_callback != nullptr && "probably out of memory");

        this->m_callbacks.push_back(new_callback);
        return new_callback->id;
    } else {
        return THISCALL(0x004D6260, this, cb, a3, a4);
    }
}

int event_recipient_entry::add_callback(script_instance *a2, const vm_executable *a3, char *a4, bool a5)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *callback = new script_event_callback{a2, a3, a4, a5};
        m_callbacks.push_back(callback);
        return callback->id;
    } else {
        return THISCALL(0x004C02A0, this, a2, a3, a4, a5);
    }
}

bool event_recipient_entry::callback_exists(int a2) const
{
    if (a2 == 0) {
        return false;
    }

    for (auto &v3 : this->m_callbacks) {
        if (v3->id == a2) {
            return true;
        }
    }

    return false;
}

void event_recipient_entry::remove_callback(unsigned int a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        field_10.push_back(reinterpret_cast<void *>(a2));
    } else {
        THISCALL(0x004DB7F0, this, a2);
    }
}

void event_recipient_entry::clear_stale_callbacks()
{
    for (auto it = m_callbacks.begin(); it != m_callbacks.end();) {
        auto *callback = *it;
        bool(__fastcall * is_script)(event_callback *, void *) = CAST(is_script, get_vfunc(callback->m_vtbl, 0xC));
        bool remove =
            is_script(callback, nullptr) && static_cast<script_event_callback *>(callback)->instance == nullptr;
        for (auto id : field_10)
            remove |= callback->id == reinterpret_cast<std::intptr_t>(id);
        if (remove) {
            callback->finalize(true);
            it = m_callbacks.erase(it);
        } else {
            ++it;
        }
    }
    field_10.clear();
}

void event_recipient_entry::clear_script_callbacks(script_executable *executable)
{
    for (auto it = m_callbacks.begin(); it != m_callbacks.end();) {
        auto *callback = *it;
        auto is_script =
            reinterpret_cast<bool(__fastcall *)(event_callback *, void *)>(get_vfunc(callback->m_vtbl, 0xC));
        if (is_script(callback, nullptr)) {
            auto *instance = static_cast<script_event_callback *>(callback)->instance;
            auto *owner = instance != nullptr ? instance->parent->parent : nullptr;
            if (executable == nullptr || owner == executable) {
                it = m_callbacks.erase(it);
                callback->finalize(true);
                continue;
            }
        }
        ++it;
    }
}

void event_recipient_entry::clear_script_callback(string_hash function)
{
    for (auto it = m_callbacks.begin(); it != m_callbacks.end();) {
        auto *callback = *it;
        auto is_script =
            reinterpret_cast<bool(__fastcall *)(event_callback *, void *)>(get_vfunc(callback->m_vtbl, 0xC));
        if (is_script(callback, nullptr)) {
            auto *script_callback = static_cast<script_event_callback *>(callback);
            if (script_callback->instance && script_callback->executable->get_fullname() == function) {
                it = m_callbacks.erase(it);
                callback->finalize(true);
                continue;
            }
        }
        ++it;
    }
}

bool event_recipient_entry::does_script_have_callbacks(const script_executable *executable) const
{
    for (auto *callback : m_callbacks) {
        auto is_script =
            reinterpret_cast<bool(__fastcall *)(event_callback *, void *)>(get_vfunc(callback->m_vtbl, 0xC));
        if (is_script(callback, nullptr)) {
            auto *instance = static_cast<script_event_callback *>(callback)->instance;
            auto *owner = instance != nullptr ? instance->parent->parent : nullptr;
            if (owner == executable)
                return true;
        }
    }
    return false;
}


bool event_recipient_entry::garbage_collect()
{
    if (this->field_0.field_0 != 0 && this->field_0.get_volatile_ptr() == nullptr) {
        return true;
    }

    bool v3 = true;
    if (!this->m_callbacks.empty()) {
        this->clear_stale_callbacks();
        v3 = this->m_callbacks.empty();
    }

    auto v4 = g_world_ptr->time_manager.field_C;
    auto v5 = this->field_24;
    if (v5 == v4 - 1 || (v5 != v4 && this->field_20 == v4 - 1)) {
        return false;
    }

    return v3;
}

void event_recipient_entry::sub_4DB840(event *the_event)
{
    assert(the_event != nullptr);

    this->garbage_collect();
    the_event->raise();
    auto v3 = g_world_ptr->time_manager.field_C;
    auto v4 = this->field_24;
    if (v4 != v3) {
        this->field_20 = v4;
        this->field_24 = v3;
    }

    process_event_callbacks(the_event, this->field_0, &this->m_callbacks);
}
