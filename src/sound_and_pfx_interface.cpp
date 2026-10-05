#include "sound_and_pfx_interface.h"

#include "common.h"
#include "config.h"
#include "func_wrapper.h"
#include "memory.h"
#include "tl_system.h"
#include "native_enx.h"
#include "parse_generic_mash.h"
#include "resource_key.h"
#include "variable.h"
#include <algorithm>
#include <vector.hpp>

VALIDATE_SIZE(sound_and_pfx_interface, 0x40);

#if STANDALONE_SYSTEM
namespace {

template<class T, class Decode>
void decode_vector(mashable_vector<T> &values, generic_mash_data_ptrs *data, Decode decode)
{
    if (values.is_shared()) {
        data->rebase_shared(8);
        auto *metadata = data->get_from_shared<std::uint32_t>(4);
        data->rebase_shared(8);
        data->rebase_shared(4);
        values.m_data = data->get_from_shared<T>(values.size());
        if (metadata[3] != 0) {
            data->get<std::uint8_t>(metadata[0]);
            data->get_from_shared<std::uint8_t>(metadata[1] - sizeof(T) * values.size());
        } else {
            for (auto &entry : values)
                decode(entry);
        }
        ++metadata[3];
        data->rebase_shared(4);
    } else {
        data->rebase(8);
        data->rebase(4);
        values.m_data = data->get<T>(values.size());
        for (auto &entry : values)
            decode(entry);
        data->rebase(4);
    }
}

void decode_event(sound_interface_event_info &event, generic_mash_data_ptrs *data)
{
    *reinterpret_cast<std::uint8_t *>(&event.field_14) = 0;
    decode_vector(event.resources, data, [](sound_interface_resource_info &resource) {
        resource.flags &= ~1u;
    });

    event.available = new _std::list<sound_interface_resource_info *>;
}

void release_events(mashable_vector<sound_interface_event_info> &events)
{
    if (events.is_shared()) {
        auto *references = reinterpret_cast<std::uint32_t *>(events.m_data) - 1;
        if (--*references != 0)
            return;
    }
    for (auto &event : events) {
        if (event.resources.is_shared())
            --*(reinterpret_cast<std::uint32_t *>(event.resources.m_data) - 1);
        delete event.available;
        event.available = nullptr;
    }
}

void release_owned_shared_info(shared_sound_interface_info *info)
{
    for (auto &event : info->events) {
        delete event.available;
        if (!event.resources.from_mash())
            delete[] event.resources.m_data;
    }
    if (!info->events.from_mash())
        delete[] info->events.m_data;
    if (!info->web_parameters.from_mash())
        delete[] info->web_parameters.m_data;
    if (!info->groups.from_mash())
        delete[] info->groups.m_data;
    delete info;
}

void remove_sound_interface(sound_interface *interface_ptr)
{
    auto &interfaces = var<_std::vector<sound_interface *> *>(0x0095A6A4);
    if (interfaces == nullptr)
        return;
    auto found = std::find(interfaces->begin(), interfaces->end(), interface_ptr);
    if (found != interfaces->end())
        interfaces->erase(found);
    if (interfaces->empty()) {
        delete interfaces;
        interfaces = nullptr;
    }
}

bool __fastcall unsupported_attribute(sound_and_pfx_interface *, void *, const resource_key *, void *, bool)
{

    return false;
}

const char *__fastcall interface_type(sound_and_pfx_interface *, void *)
{
    return "entity_base";
}

void __fastcall unmash_interface(sound_and_pfx_interface *self, void *, generic_mash_header *header,
                                void *owner, void *storage, generic_mash_data_ptrs *data)
{
    self->un_mash(header, owner, storage, data);
}

void __fastcall release_interface(sound_and_pfx_interface *self, void *)
{
    self->release_ifc();
}

void __fastcall destroy_interface(sound_and_pfx_interface *self, void *, unsigned flags)
{
    self->release_ifc();
    ::operator delete(reinterpret_cast<void *>(self->field_34));
    self->field_34 = self->field_38 = self->field_3C = 0;
    self->field_4 = 0;
    if ((flags & 1) != 0)
        tlMemFree(self);
}

void __fastcall advance_interface(sound_and_pfx_interface *self, void *, Float elapsed)
{
    frame_advance_native_sound_emitter(self, elapsed);
}
}
#endif

web_sound_params *shared_sound_interface_info::get_web_sound_params(string_hash name)
{
#if STANDALONE_SYSTEM

    for (auto &group : groups) {
        if (group.category == name)
            return &group;
    }
    return nullptr;
#else
    web_sound_params *(__fastcall *func)(void *, void *, string_hash) = CAST(func, 0x00564670);
    return func(this, nullptr, name);
#endif
}

sound_and_pfx_interface::sound_and_pfx_interface() {}

std::intptr_t sound_and_pfx_interface::native_vtable()
{
#if STANDALONE_SYSTEM
    static void *table[] = {
        reinterpret_cast<void *>(destroy_interface),
        reinterpret_cast<void *>(unsupported_attribute), reinterpret_cast<void *>(unsupported_attribute),
        reinterpret_cast<void *>(unsupported_attribute), reinterpret_cast<void *>(unsupported_attribute),
        reinterpret_cast<void *>(unsupported_attribute), reinterpret_cast<void *>(unsupported_attribute),
        reinterpret_cast<void *>(unmash_interface), reinterpret_cast<void *>(interface_type),
        reinterpret_cast<void *>(release_interface), reinterpret_cast<void *>(advance_interface)
    };
    return reinterpret_cast<std::intptr_t>(table);
#else
    return 0x00883A18;
#endif
}

void sound_and_pfx_interface::un_mash(generic_mash_header *header, void *owner, [[maybe_unused]] void *storage,
                                    generic_mash_data_ptrs *data)
{
#if STANDALONE_SYSTEM
    field_30 = field_34 = field_38 = field_3C = 0;
    field_28 = nullptr;
    field_2C = static_cast<entity_base *>(owner);
    field_28 = native_enx::load(data, this, field_2C);
    field_4 = reinterpret_cast<std::intptr_t>(owner);
    dynamic = false;
    field_C.field_0 = 0;
    field_10 = nullptr;
    field_20 = 0.5f;
    field_14 = ZEROVEC;
    field_24 = 0.0f;
    if (header->is_flagged(0x10)) {

        data->rebase_shared(4);
        field_10 = data->get_from_shared<shared_sound_interface_info>();
        *reinterpret_cast<std::uint8_t *>(&field_10->field_0) = 0;
        decode_vector(field_10->events, data, [data](sound_interface_event_info &event) {
            decode_event(event, data);
        });
        decode_vector(field_10->web_parameters, data, [data](std::array<std::uint32_t, 4> &entry) {
            for (auto &word : entry)
                word = *data->get_from_shared<std::uint32_t>();
        });
        decode_vector(field_10->groups, data, [data](web_sound_params &entry) {

            entry.category = string_hash{int(*data->get_from_shared<uint32_t>())};
            entry.launch_group = string_hash{int(*data->get_from_shared<uint32_t>())};
            entry.travel_group = string_hash{int(*data->get_from_shared<uint32_t>())};
            entry.impact_group = string_hash{int(*data->get_from_shared<uint32_t>())};
            entry.travel_factor = *data->get_from_shared<float>();
            entry.field_14 = *data->get_from_shared<uint32_t>();
            entry.field_18 = *data->get_from_shared<uint32_t>();
            entry.field_18 = *data->get_from_shared<uint32_t>();
        });
        ++field_10->references;
    }
    auto &interfaces = var<_std::vector<sound_interface *> *>(0x0095A6A4);
    if (interfaces == nullptr)
        interfaces = new _std::vector<sound_interface *>;
    interfaces->push_back(this);
#else
    generic_interface::un_mash(header, owner, storage, data);
#endif
}

web_sound_params *sound_and_pfx_interface::get_web_sound_params(string_hash name)
{
    return field_10 != nullptr ? field_10->get_web_sound_params(name) : nullptr;
}

void sound_and_pfx_interface::release_ifc()
{
#if STANDALONE_SYSTEM
    remove_sound_interface(this);
    release_native_sound_emitter(this);
    if (field_10 != nullptr) {
        --field_10->references;
        if ((*reinterpret_cast<std::uint8_t *>(&field_10->field_0)) != 0) {
            if (field_10->references == 0)
                release_owned_shared_info(field_10);
        } else {
            release_events(field_10->events);
        }
        field_10 = nullptr;
    }
    if (field_28 != nullptr) {
        native_enx::release(field_28, this);
        field_28 = nullptr;
    }
#else
    pfx_interface::release_ifc();
#endif
}

void sound_interface::copy(const sound_interface &source)
{

    if (field_10) {
        --field_10->references;
        if (*reinterpret_cast<uint8_t *>(&field_10->field_0)) {
            if (!field_10->references)
                release_owned_shared_info(field_10);
        } else {
            release_events(field_10->events);
        }
    }
    field_10 = source.field_10;
    if (field_10) {
        ++field_10->references;
        if (field_10->events.from_mash() && field_10->events.is_shared())
            ++*(reinterpret_cast<uint32_t *>(field_10->events.m_data) - 1);
    }
}
