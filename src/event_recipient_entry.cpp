#include "event_recipient_entry.h"

#include "code_event_callback.h"
#include "common.h"
#include "event.h"
#include "func_wrapper.h"
#include "memory.h"
#include "trace.h"
#include "vm.h"
#include "script_event_callback.h"
#include "vtbl.h"
#include "chuck/vm/script_object.h"

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
    this->field_4.clear();
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
    for (auto &cb : this->field_4) {
        if (cb != nullptr) {
            cb->_finalize(true);
        }
    }

    this->field_4.clear();
}

int event_recipient_entry::add_callback(void (*cb)(event *, entity_base_vhandle, void *), void *a3, bool a4)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *new_callback = new code_event_callback{cb, a3, a4};
        assert(new_callback != nullptr && "probably out of memory");

        this->field_4.push_back(new_callback);
        return new_callback->id;
    } else {
        return THISCALL(0x004D6260, this, cb, a3, a4);
    }
}

int event_recipient_entry::add_callback(script_instance *a2, const vm_executable *a3, char *a4, bool a5)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *callback = new script_event_callback{a2, a3, a4, a5};
        field_4.push_back(callback);
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

    for (auto &v3 : this->field_4) {
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

void event_recipient_entry::clean_up_callbacks()
{
    for (auto it = field_4.begin(); it != field_4.end();) {
        auto *callback = *it;
        bool (__fastcall *is_script)(event_callback *, void *) =
            CAST(is_script, get_vfunc(callback->m_vtbl, 0xC));
        bool remove = is_script(callback, nullptr) &&
            static_cast<script_event_callback *>(callback)->instance == nullptr;
        for (auto id : field_10)
            remove |= callback->id == reinterpret_cast<std::intptr_t>(id);
        if (remove) {
            callback->_finalize(true);
            it = field_4.erase(it);
        } else {
            ++it;
        }
    }
    field_10.clear();
}

void event_recipient_entry::clear_script_callbacks(script_executable *executable)
{
    for (auto it = field_4.begin(); it != field_4.end();) {
        auto *callback = *it;
        auto is_script = reinterpret_cast<bool (__fastcall *)(event_callback *, void *)>(
            get_vfunc(callback->m_vtbl, 0xC));
        if (is_script(callback, nullptr)) {
            auto *instance = static_cast<script_event_callback *>(callback)->instance;
            auto *owner = instance != nullptr ? instance->parent->parent : nullptr;
            if (executable == nullptr || owner == executable) {
                it = field_4.erase(it);
                callback->_finalize(true);
                continue;
            }
        }
        ++it;
    }
}

bool event_recipient_entry::does_script_have_callbacks(const script_executable *executable) const
{
    for (auto *callback : field_4) {
        auto is_script = reinterpret_cast<bool (__fastcall *)(event_callback *, void *)>(
            get_vfunc(callback->m_vtbl, 0xC));
        if (is_script(callback, nullptr)) {
            auto *instance = static_cast<script_event_callback *>(callback)->instance;
            auto *owner = instance != nullptr ? instance->parent->parent : nullptr;
            if (owner == executable)
                return true;
        }
    }
    return false;
}
