#include "native_enx.h"

#include "native_pfx.h"
#include "pfx_interface.h"
#include "sound_and_pfx_interface.h"
#include "mash_info_struct.h"
#include "mash_virtual_base.h"
#include "fixedstring.h"
#include "event.h"
#include "event_manager.h"
#include "event_type.h"
#include "event_recipient_entry.h"
#include "script.h"
#include "vm_thread.h"
#include "actor.h"
#include "wds.h"
#include "memory.h"
#include "slab_allocator.h"
#include "nslbank.h"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <new>
#include <string>
#include <vector>

namespace native_enx {
namespace {
struct Object {
    intptr_t table;
};
struct Vector {
    unsigned image_size;
    int count;
    Object **data;
    unsigned capacity;
    bool owns_elements;
    unsigned char padding[3];
};
struct ActionList {
    intptr_t table;
    int callback;
    Vector actions;
};
struct Graph {
    intptr_t table;
    unsigned callbacks[6];
    ActionList common;
};
struct ExtendedGraph {
    Graph base;
    Vector lists;
};
struct DataSlot {
    intptr_t table;
    unsigned image_size;
    tlFixedString name;
    unsigned hash;
    aeps::UpdateStruct *updater;
    float fade;
    unsigned flags;
};
struct EventSlot {
    DataSlot base;
    unsigned field_38[2];
};
struct SoundSlot {
    DataSlot base;
    float delay, lifetime;
    unsigned sound;
    unsigned limit;
    sound_instance_id instance;
    intptr_t bank_owner;
};
struct ScriptSlot {
    DataSlot base;
    float delay, lifetime;
    tlFixedString start, stop;
    vm_thread *thread;
    unsigned field_84;
    script_instance *instance;
    entity_base_vhandle owner, other;
    unsigned field_94;
};
struct PfxSlot {
    DataSlot base;
    native_pfx::Instance *instance;
    unsigned field_3C[9];
    float delay, lifetime, scale;
    unsigned field_6C;
};
using ActionInfo = aeps::ActionInfoStruct;
static_assert(sizeof(Vector) == 20);
static_assert(sizeof(ActionList) == 28);
static_assert(sizeof(Graph) == 56);
static_assert(sizeof(ExtendedGraph) == 76);
static_assert(sizeof(DataSlot) == 56);
static_assert(sizeof(EventSlot) == 64);
static_assert(sizeof(SoundSlot) == 80);
static_assert(sizeof(ScriptSlot) == 152);
static_assert(sizeof(PfxSlot) == 112);
static_assert(sizeof(ActionInfo) == 44);
static_assert(offsetof(DataSlot, updater) == 0x2C);

constexpr unsigned sizes[] = {28, 76, 76, 56, 76, 64, 112, 152, 152, 80};
void *tables[10][20]{};

template <class R, class... Args>
R call(void *object, unsigned offset, Args... args)
{
    auto **table = reinterpret_cast<void **>(static_cast<Object *>(object)->table);
    auto fn = reinterpret_cast<R(__fastcall *)(void *, void *, Args...)>(table[offset / 4]);
    return fn(object, nullptr, args...);
}
unsigned class_id(void *object)
{
    return call<unsigned>(object, 0xC);
}
unsigned list_count(unsigned type)
{
    return type == 1 ? 22 : type == 2 ? 19 : 6;
}

void destroy_vector(Vector &vector)
{
    auto inside = [&](const void *pointer) {
        auto value = reinterpret_cast<uintptr_t>(pointer);
        auto begin = reinterpret_cast<uintptr_t>(&vector);
        return value >= begin && value <= begin + vector.image_size;
    };
    if (vector.owns_elements) {
        for (int i = 0; i < vector.count; ++i) {
            if (vector.data[i]) {
                if (inside(vector.data[i]))
                    call<void>(vector.data[i], 0);
                else
                    call<void *>(vector.data[i], 8, 1);
            }
            vector.data[i] = nullptr;
        }
    }
    if (vector.data && !inside(vector.data)) {
        if (vector.capacity * 4 <= 0xB0)
            slab_allocator::deallocate(vector.data, nullptr);
        else
            ::operator delete(vector.data);
    }
    vector.data = nullptr;
    vector.count = 0;
    vector.capacity = 0;
    vector.image_size = 0;
}
void detach_updater(DataSlot *slot)
{
    if (auto *updater = slot->updater) {
        aeps::RemUpdater(updater);
        call<void *>(updater, 0, 1);
        slot->updater = nullptr;
    }
}
void __fastcall destruct(void *object, void *)
{
    const unsigned type = class_id(object);
    if (type == 0)
        destroy_vector(static_cast<ActionList *>(object)->actions);
    else if (type >= 1 && type <= 4) {
        auto *graph = static_cast<Graph *>(object);
        if (type != 3)
            destroy_vector(static_cast<ExtendedGraph *>(object)->lists);
        destroy_vector(graph->common.actions);
    } else
        detach_updater(static_cast<DataSlot *>(object));
}
void *__fastcall deleting_destruct(void *object, void *, int flags)
{
    destruct(object, nullptr);
    if (flags & 1)
        ::operator delete(object);
    return object;
}
void __fastcall updater_destruct(aeps::UpdateStruct *object, void *, int flags)
{
    if (flags & 1)
        mem_freealign(object);
}
void __fastcall updater_advance(aeps::UpdateStruct *object, void *, float time, int)
{
    object->advance(time);
}
void *updater_table[] = {reinterpret_cast<void *>(updater_destruct), reinterpret_cast<void *>(updater_advance)};

void unmash_vector(Vector &vector, mash_info_struct *mash, unsigned base_size, unsigned size_slot)
{
    if (vector.data) {
        vector.data = reinterpret_cast<Object **>(mash->read_from_buffer(vector.count * 4, 4));
        for (int i = 0; i < vector.count; ++i) {
            auto *object = mash->read_from_buffer(base_size, 0);
            vector.data[i] = reinterpret_cast<Object *>(object);
            mash_virtual_base::fixup_vtable(object);
            unsigned size = call<unsigned>(object, size_slot);
            mash->advance_buffer(size - base_size);
            call<void>(object, 4, mash, static_cast<void *>(nullptr));
        }
    }
    vector.image_size = static_cast<unsigned>(mash->mash_image_ptr[0] + mash->buffer_size_used[0] -
                                              reinterpret_cast<unsigned char *>(&vector));
}
void __fastcall unmash(void *object, void *, mash_info_struct *mash, void *)
{
    const unsigned type = class_id(object);
    if (type == 0)
        unmash_vector(static_cast<ActionList *>(object)->actions, mash, 56, 0x30);
    else if (type >= 1 && type <= 4) {
        auto *graph = static_cast<Graph *>(object);
        mash_virtual_base::fixup_vtable(&graph->common);
        call<void>(&graph->common, 4, mash, object);
        if (type != 3)
            unmash_vector(static_cast<ExtendedGraph *>(object)->lists, mash, 28, 0x20);
    }
}
void construct_vector(Vector &vector)
{
    for (int i = 0; i < vector.count; ++i)
        vector.data[i] = static_cast<Object *>(mash_virtual_base::construct_class_helper(vector.data[i]));
}
int __fastcall list_append(ActionList *list, void *, Object *element)
{
    auto &vector = list->actions;
    const auto begin = reinterpret_cast<uintptr_t>(&vector);
    const auto data = reinterpret_cast<uintptr_t>(vector.data);
    const bool mashed = data >= begin && data <= begin + vector.image_size;
    if (vector.count == static_cast<int>(vector.capacity) || mashed) {
        const unsigned capacity = 8 * (vector.count / 8) + 8;
        if (capacity > vector.capacity) {
            const unsigned bytes = 4 * capacity;
            auto **replacement =
                static_cast<Object **>(bytes > 0xB0 ? ::operator new(bytes) : slab_allocator::allocate(bytes, nullptr));
            if (vector.data) {
                std::memcpy(replacement, vector.data, 4 * vector.count);
                if (!mashed) {
                    if (4 * vector.capacity > 0xB0)
                        ::operator delete(vector.data);
                    else
                        slab_allocator::deallocate(vector.data, nullptr);
                }
            }
            vector.data = replacement;
            vector.capacity = capacity;
        }
    }
    vector.data[vector.count++] = element;
    return vector.count;
}

void __cdecl event_action(event *raised, entity_base_vhandle handle, void *context)
{
    auto *owner = handle.get_volatile_ptr();
    if (!owner)
        return;
    if (!context) {
        aeps::DoCallback(owner, 5, 0);
        return;
    }
    ActionInfo info;
    info.owner = owner;
    info.hash = raised->field_4.source_hash_code;
    call<void>(context, 0x1C, 5, reinterpret_cast<pfx_interface *>(owner->my_sound_and_pfx_interface), &info);
}
void load_list(ActionList &list, generic_mash_data_ptrs *data, pfx_interface *ifc, entity_base *owner,
               bool register_events)
{
    for (int i = 0; i < list.actions.count; ++i) {
        auto *slot = reinterpret_cast<DataSlot *>(list.actions.data[i]);
        call<void>(slot, 0x18, data, ifc, owner);
        if (register_events && (slot->flags & 0x20)) {
            auto *type = event_manager::register_event_type(string_hash{static_cast<int>(slot->hash)}, false);
            if (type) {
                auto *recipient = type->create_recipient_entry(owner->my_handle);
                if (recipient)
                    recipient->add_callback(event_action, slot, false);
            }
        }
    }
}
void __fastcall graph_load(Graph *graph, void *, generic_mash_data_ptrs *data, pfx_interface *ifc, entity_base *owner)
{
    const unsigned type = class_id(graph);
    load_list(graph->common, data, ifc, owner, type != 4);
    if (type != 3) {
        auto &lists = reinterpret_cast<ExtendedGraph *>(graph)->lists;
        for (unsigned i = 0; i < list_count(type); ++i)
            load_list(*reinterpret_cast<ActionList *>(lists.data[i]), data, ifc, owner, false);
    }
}
void __fastcall pfx_load(PfxSlot *slot, void *, generic_mash_data_ptrs *data, pfx_interface *ifc, entity_base *)
{
    slot->instance = native_pfx::load(data, nullptr);
    native_pfx::attach_instance(ifc, slot->instance);
    slot->instance->flags |= 4;
    slot->instance->fade = slot->base.fade;
    if (slot->instance->effect) {
        auto *effect = reinterpret_cast<native_pfx::Effect *>(slot->instance->effect);
        effect->fade = slot->base.fade;
        if (effect->fade >= 1.0f && !(slot->instance->flags & 8))
            effect->retry = false;
    }
}
void __fastcall empty_load(void *, void *, generic_mash_data_ptrs *, pfx_interface *, entity_base *) {}
void __fastcall empty_action(void *, void *, int, pfx_interface *, ActionInfo *) {}
void __fastcall empty_update(void *, void *) {}

void __fastcall script_start(ScriptSlot *slot, void *)
{
    if (!slot->stop.field_4[0])
        detach_updater(&slot->base);
    auto *owner = slot->owner.get_volatile_ptr();
    auto *other = slot->other.get_volatile_ptr();
    const char *suffix = owner && other ? "(entity,entity)" : owner || other ? "(entity)" : "()";
    string_hash function{(std::string(slot->start.c_str()) + suffix).c_str()};
    slot->thread = owner || other
                       ? find_func_and_spawn_new_thread(reinterpret_cast<actor *>(owner ? owner : other), function)
                       : spawn_thread_for_func(function, script::get_gsoi());
    if (slot->thread) {
        if (owner)
            script::push_arg(owner);
        if (other)
            script::push_arg(other);
        script::exec_thread(false);
        slot->instance = slot->thread->inst;
    }
}
void __fastcall script_stop(ScriptSlot *slot, void *)
{
    if (!slot->instance || !slot->stop.field_4[0])
        return;
    string_hash function{(std::string(slot->stop.c_str()) + "()").c_str()};
    auto *owner = slot->owner.get_volatile_ptr();
    slot->thread = owner ? find_func_and_spawn_new_thread(reinterpret_cast<actor *>(owner), function)
                         : spawn_thread_for_func(function, script::get_gsoi());
    if (slot->thread)
        script::exec_thread(false);
    slot->instance = nullptr;
}
void __fastcall script_action(ScriptSlot *slot, void *, int, pfx_interface *, ActionInfo *info)
{
    const unsigned flags = slot->base.flags | info->flags;
    if (info->owner && (flags & 0x100000)) {
        auto *player = reinterpret_cast<actor *>(g_world_ptr->get_hero_ptr(0));
        if (!player || info->owner != reinterpret_cast<entity_base *>(player))
            return;
        auto *controller = player->m_player_controller;
        if (controller && !call<bool>(controller, 0x3C))
            return;
    }
    if (slot->base.updater) {
        if (flags & 1) {
            slot->base.updater->lifetime = info->lifetime;
            return;
        }
        script_stop(slot, nullptr);
        slot->base.updater->target = nullptr;
        detach_updater(&slot->base);
    }
    slot->delay = info->delay;
    slot->lifetime = info->lifetime;
    if (info->owner)
        slot->owner = info->owner->my_handle;
    if (info->other)
        slot->other = info->other->my_handle;
    auto *updater = static_cast<aeps::UpdateStruct *>(arch_memalign(16, sizeof(aeps::UpdateStruct)));
    updater->m_vtbl = reinterpret_cast<intptr_t>(updater_table);
    updater->elapsed = 0;
    updater->delay = slot->delay;
    updater->lifetime = slot->lifetime;
    updater->field_14 = true;
    updater->started = false;
    updater->target = reinterpret_cast<aeps::UpdateTarget *>(slot);
    slot->base.updater = updater;
    aeps::s_activeStructs().push_back(updater);
}

std::vector<void *> sound_banks;
void __fastcall sound_action(SoundSlot *slot, void *, int callback, pfx_interface *ifc, ActionInfo *info)
{
    const unsigned flags = slot->base.flags | info->flags;
    if ((flags & 0x20000000) && info->owner) {
        if (!slot->bank_owner && info->owner->get_flavor() == 10) {
            auto *resource = *reinterpret_cast<unsigned char **>(reinterpret_cast<unsigned char *>(info->owner) + 0x68);
            auto *pack = *reinterpret_cast<void **>(resource + 0x28);
            bool seen = false;
            for (auto *bank : sound_banks)
                if (bank == pack) {
                    seen = true;
                    break;
                }
            if (seen)
                slot->bank_owner = -1;
            else {
                sound_banks.push_back(pack);
                slot->bank_owner = reinterpret_cast<intptr_t>(slot);
            }
        }
        if (slot->bank_owner != reinterpret_cast<intptr_t>(slot))
            return;
    }
    if (callback == 4 || (callback == 3 && (flags & 0x10000000))) {
        if ((flags & 0x40) && stop_first_native_emitter_sound(ifc))
            return;
        if (auto *instance = slot->instance.get_sound_instance_ptr()) {
            const auto *wave = nslGetWave(instance->wave_id);
            if (!wave || !(wave->flags & 8))
                return;
            instance->stop();
        }
        slot->instance = sound_instance_id{0};
    } else {
        auto *instance = slot->instance.get_sound_instance_ptr();
        unsigned count;
        if (!slot->instance.field_0 || !instance ||
            (native_sound_emitter_count(instance->emitter_id, count) && count < slot->limit))
            slot->instance = ifc->play_sound_grp(string_hash{static_cast<int>(slot->sound)}, 1, 1, 1, -1, -1);
    }
}

bool spawn_slot(PfxSlot *slot, unsigned flags, float delay, float lifetime, float scale, const vector3d *position,
                entity_base *attach)
{
    if (attach)
        native_pfx::set_attached_owner(slot->instance, attach);
    if (position)
        native_pfx::set_position(slot->instance, *position);
    return native_pfx::spawn_action(
        slot->instance, flags | slot->base.flags, delay + slot->delay, lifetime + slot->lifetime, scale * slot->scale);
}
void __fastcall pfx_action(PfxSlot *slot, void *, int, pfx_interface *, ActionInfo *info)
{
    const unsigned flags = info->flags | slot->base.flags;
    if (flags & 2)
        return;
    if ((flags & 4) && info->other && info->other->is_an_entity_base() && info->other->has_sound_and_pfx_ifc()) {
        auto *ifc = reinterpret_cast<pfx_interface *>(info->other->my_sound_and_pfx_interface);
        if (ifc->field_28) {
            ActionInfo inherited;
            inherited.owner = info->other;
            inherited.other = info->owner;
            inherited.hash = info->hash;
            inherited.position = info->position;
            inherited.flags = 0x40000000;
            inherited.index = -1;
            inherited.delay = inherited.lifetime = 0;
            inherited.scale = 1;
            if (call<int>(ifc->field_28, 0x2C, &inherited) == 1)
                return;
        }
    }
    spawn_slot(slot, flags, info->delay, info->lifetime, info->scale, info->position, nullptr);
}
void perform_list(ActionList &list, int callback, pfx_interface *ifc, ActionInfo *info, bool filter_hash)
{
    if (list.callback != callback)
        return;
    for (int i = 0; i < list.actions.count; ++i) {
        auto *slot = reinterpret_cast<DataSlot *>(list.actions.data[i]);
        if (!filter_hash || slot->hash == info->hash)
            call<void>(slot, 0x1C, callback, ifc, info);
    }
}
void __fastcall graph_perform(Graph *graph, void *, int callback, pfx_interface *ifc, ActionInfo *info)
{
    const unsigned type = class_id(graph);
    if (type == 4) {
        if (!info || !info->mask)
            return;
        auto &lists = reinterpret_cast<ExtendedGraph *>(graph)->lists;
        for (unsigned i = 0; i < 6; ++i)
            if (info->mask & (1u << i))
                perform_list(*reinterpret_cast<ActionList *>(lists.data[i]), callback, ifc, info, false);
        perform_list(graph->common, callback, ifc, info, false);
    } else if (type != 3 && info->index != -1) {
        auto &lists = reinterpret_cast<ExtendedGraph *>(graph)->lists;
        perform_list(*reinterpret_cast<ActionList *>(lists.data[static_cast<unsigned short>(info->index)]),
                     callback,
                     ifc,
                     info,
                     true);
    } else
        perform_list(graph->common, callback, ifc, info, true);
}
bool __fastcall graph_has(Graph *graph, void *, unsigned callback)
{
    return graph->callbacks[callback] != 0;
}
unsigned __fastcall graph_mark(Graph *graph, void *, unsigned callback, unsigned char value)
{
    return graph->callbacks[callback] = value;
}
bool __fastcall graph_any(Graph *graph, void *)
{
    for (unsigned i = 0; i < 6; ++i)
        if (graph_has(graph, nullptr, i))
            return true;
    return false;
}
bool __fastcall graph_check(Graph *graph, void *, int callback, unsigned mask)
{
    if (graph->common.actions.count && graph->common.callback == callback)
        return true;
    const unsigned type = class_id(graph);
    if (type == 3)
        return false;
    auto &lists = reinterpret_cast<ExtendedGraph *>(graph)->lists;
    for (unsigned i = 0; i < list_count(type); ++i) {
        auto *list = reinterpret_cast<ActionList *>(lists.data[i]);
        if ((mask & (1u << i)) && list->actions.count && list->callback == callback)
            return true;
    }
    return false;
}
Vector &spawn_vector(Graph *graph, int index)
{
    return index == -1 ? graph->common.actions
                       : reinterpret_cast<ActionList *>(
                             reinterpret_cast<ExtendedGraph *>(graph)->lists.data[static_cast<unsigned short>(index)])
                             ->actions;
}
int spawn_matching(Graph *graph, unsigned hash, int index, entity_base *owner, unsigned flags, const vector3d *position,
                   float delay, float lifetime, float scale, entity_base *other = nullptr)
{
    if (!graph_has(graph, nullptr, 5))
        return 0;
    auto &vector = spawn_vector(graph, index);
    int result = 0;
    for (int i = 0; i < vector.count; ++i) {
        if (class_id(vector.data[i]) != 6)
            continue;
        auto *slot = reinterpret_cast<PfxSlot *>(vector.data[i]);
        if (slot->base.hash != hash)
            continue;
        unsigned combined = flags | slot->base.flags;
        auto *attach = index == -1 ? (combined & 8 ? owner : combined & 0x10 ? other : nullptr) : nullptr;
        result |= spawn_slot(slot, flags, delay, lifetime, scale, position, attach);
    }
    return result;
}
int __fastcall graph_spawn_info(Graph *graph, void *, ActionInfo *info)
{
    int index = class_id(graph) == 3 ? -1 : info->index;
    return spawn_matching(graph,
                          info->hash,
                          index,
                          info->owner,
                          info->flags,
                          info->position,
                          info->delay,
                          info->lifetime,
                          info->scale,
                          info->other);
}
int __fastcall graph_spawn_index(Graph *graph, void *, unsigned hash, unsigned short index, entity_base *owner,
                                 unsigned flags)
{
    return spawn_matching(graph, hash, class_id(graph) == 3 ? -1 : index, owner, flags, nullptr, 0, 0, 1);
}
int __fastcall graph_spawn_position(Graph *graph, void *, unsigned hash, int, const vector3d *position,
                                    entity_base *owner, unsigned flags)
{
    return spawn_matching(graph, hash, -1, owner, flags, position, 0, 0, 1);
}
int __fastcall graph_spawn(Graph *graph, void *, unsigned hash, entity_base *owner, unsigned flags)
{
    return spawn_matching(graph, hash, -1, owner, flags, nullptr, 0, 0, 1);
}
int __fastcall empty_spawn5(void *, void *, int, int, const void *, int, int)
{
    return 0;
}
int __fastcall empty_spawn4(void *, void *, int, int, int, int)
{
    return 0;
}
int __fastcall empty_spawn3(void *, void *, int, int, int)
{
    return 0;
}
int __fastcall empty_spawn_info(void *, void *, ActionInfo *)
{
    return 0;
}

template <unsigned N>
unsigned __fastcall id(void *, void *)
{
    return N;
}
template <unsigned N>
unsigned __fastcall size(void *, void *)
{
    return sizes[N];
}
template <unsigned N>
bool __fastcall subclass(void *, void *, unsigned base)
{
    if constexpr (N == 0)
        return base == 556 || base == 573;
    if constexpr (N == 1 || N == 2)
        return base == 3 || base == 558 || base == 573;
    if constexpr (N >= 1 && N <= 4)
        return base == 558 || base == 573;
    if constexpr (N == 6)
        return base == 557 || base == 573;
    return base == 555 || base == 557 || base == 573;
}
template <unsigned N>
bool __fastcall is_or_subclass(void *object, void *, unsigned base)
{
    return base == N || subclass<N>(object, nullptr, base);
}
template <unsigned N>
void init_table()
{
    auto &table = tables[N];
    table[0] = reinterpret_cast<void *>(destruct);
    table[1] = reinterpret_cast<void *>(unmash);
    table[2] = reinterpret_cast<void *>(deleting_destruct);
    table[3] = reinterpret_cast<void *>(id<N>);
    table[4] = reinterpret_cast<void *>(subclass<N>);
    table[5] = reinterpret_cast<void *>(is_or_subclass<N>);
    if constexpr (N == 0) {
        table[6] = reinterpret_cast<void *>(list_append);
        table[7] = reinterpret_cast<void *>(id<1>);
        table[8] = reinterpret_cast<void *>(size<N>);
    } else if constexpr (N >= 1 && N <= 4) {
        table[6] = reinterpret_cast<void *>(graph_load);
        table[7] = reinterpret_cast<void *>(empty_spawn5);
        table[8] = N == 4 ? reinterpret_cast<void *>(empty_spawn5) : reinterpret_cast<void *>(graph_spawn_position);
        table[9] = N == 4 ? reinterpret_cast<void *>(empty_spawn4) : reinterpret_cast<void *>(graph_spawn_index);
        table[10] = N == 4 ? reinterpret_cast<void *>(empty_spawn3) : reinterpret_cast<void *>(graph_spawn);
        table[11] = N == 4 ? reinterpret_cast<void *>(empty_spawn_info) : reinterpret_cast<void *>(graph_spawn_info);
        table[12] = reinterpret_cast<void *>(graph_check);
        table[13] = reinterpret_cast<void *>(graph_perform);
        table[14] = reinterpret_cast<void *>(graph_mark);
        table[15] = reinterpret_cast<void *>(graph_has);
        table[16] = reinterpret_cast<void *>(graph_has);
        table[17] = reinterpret_cast<void *>(graph_any);
        table[18] = reinterpret_cast<void *>(id < N == 1 ? 3 : N == 2 ? 4 : N == 3 ? 2 : 1 >);
        table[19] = reinterpret_cast<void *>(size<N>);
    } else {
        table[6] = reinterpret_cast<void *>(empty_load);
        table[7] = reinterpret_cast<void *>(empty_action);
        table[8] = reinterpret_cast<void *>(empty_update);
        table[9] = reinterpret_cast<void *>(empty_update);
        table[10] = reinterpret_cast<void *>(empty_update);
        table[11] = reinterpret_cast<void *>(id < N == 5 ? 2 : N == 6 ? 5 : N == 8 ? 4 : 3 >);
        table[12] = reinterpret_cast<void *>(size<N>);
        if constexpr (N == 6) {
            table[6] = reinterpret_cast<void *>(pfx_load);
            table[7] = reinterpret_cast<void *>(pfx_action);
        } else if constexpr (N == 8) {
            table[7] = reinterpret_cast<void *>(script_action);
            table[8] = reinterpret_cast<void *>(script_start);
            table[9] = reinterpret_cast<void *>(script_stop);
        } else if constexpr (N == 9)
            table[7] = reinterpret_cast<void *>(sound_action);
    }
}
}  // namespace

void *vtable(unsigned type)
{
    static const bool initialized = [] {
        init_table<0>();
        init_table<1>();
        init_table<2>();
        init_table<3>();
        init_table<4>();
        init_table<5>();
        init_table<6>();
        init_table<8>();
        init_table<9>();
        return true;
    }();
    (void)initialized;
    if (type == 7)
        return native_pfx::instance_mash_vtable();
    assert(type < 10);
    return tables[type];
}
unsigned size(unsigned type)
{
    assert(type < 10);
    return sizes[type];
}
void *construct(unsigned type, void *storage, unsigned *class_size)
{
    assert(type < 10);
    if (class_size)
        *class_size = sizes[type];
    if (!storage)
        return nullptr;
    if (type == 7)
        return native_pfx::construct_instance(storage);
    static_cast<Object *>(storage)->table = reinterpret_cast<intptr_t>(vtable(type));
    if (type == 0)
        construct_vector(static_cast<ActionList *>(storage)->actions);
    else if (type >= 1 && type <= 4) {
        auto *graph = static_cast<Graph *>(storage);
        construct(0, &graph->common, nullptr);
        if (type != 3)
            construct_vector(static_cast<ExtendedGraph *>(storage)->lists);
    } else {
        static_cast<DataSlot *>(storage)->updater = nullptr;
        if (type == 6)
            static_cast<PfxSlot *>(storage)->instance = nullptr;
        else if (type == 8) {
            auto *slot = static_cast<ScriptSlot *>(storage);
            slot->field_84 = 0;
            slot->thread = nullptr;
            slot->owner.field_0 = 0;
            slot->other.field_0 = 0;
            slot->instance = nullptr;
        } else if (type == 9) {
            auto *slot = static_cast<SoundSlot *>(storage);
            slot->limit = 1;
            slot->instance = sound_instance_id{0};
            slot->bank_owner = 0;
        }
    }
    return storage;
}
void *load(generic_mash_data_ptrs *data, pfx_interface *ifc, entity_base *owner)
{
    for (unsigned i = 0; i < 8; ++i)
        if (data->field_4[i] != 0xA2)
            return nullptr;
    data->field_4 += 8;
    mash_info_struct mash{data->field_0, 0x10000};
    auto *root = mash.read_from_buffer(28, 0);
    mash_virtual_base::fixup_vtable(root);
    mash.advance_buffer(call<unsigned>(root, 0x4C) - 28);
    call<void>(root, 4, &mash, static_cast<void *>(nullptr));
    auto *result = mash_virtual_base::construct_class_helper(root);
    data->field_0 += mash.buffer_size_used[0];
    data->rebase(8);
    if (result)
        call<void>(result, 0x18, data, ifc, owner);
    data->rebase(8);
    data->rebase_shared(8);
    return result;
}
void release(void *root, pfx_interface *ifc)
{
    native_pfx::release_attached_instances(ifc);
    if (root)
        call<void>(root, 0);
}
}  // namespace native_enx
