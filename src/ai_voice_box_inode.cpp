#include "ai_voice_box_inode.h"

#include "common.h"
#include "base_ai_core.h"
#include "memory.h"
#include "actor.h"
#include "damage_interface.h"
#include "gab_manager.h"
#include "sound_and_pfx_interface.h"
#include "sound_source.h"
#include "vtbl.h"

#include <new>
#include <algorithm>
#include <array>

namespace {

constexpr uint32_t team_hashes[] = {
    to_hash("SPIDERMAN"), to_hash("VENOM"), to_hash("BOSS"),
    to_hash("NONATTACK_BOSS"), to_hash("SHIELD"), to_hash("POLICE"),
    to_hash("GANG_SKULLS"), to_hash("GANG_HELLIONS"), to_hash("GANG_FTB"),
    to_hash("GANG_SKINHEAD"), to_hash("GANG_MERC"), to_hash("GANG_SRK"),
    to_hash("TRASK"), to_hash("CIVILIAN"), to_hash("PEDESTRIAN"),
};
}

VALIDATE_SIZE(ai::voice_box_inode, 0x44);
VALIDATE_SIZE(ai_lip_sync, 0x50);
VALIDATE_SIZE(ai::speech_request, 0x20);
VALIDATE_OFFSET(ai::voice_box_inode, lip_sync, 0x28);
VALIDATE_OFFSET(ai::voice_box_inode, teams, 0x30);
VALIDATE_OFFSET(ai::voice_box_inode, speech_requests, 0x40);

std::set<ai::voice_box_inode *> *ai::voice_box_inode::speaking_voice_boxes = nullptr;
std::set<ai::voice_box_inode *> *ai::voice_box_inode::live_voice_boxes = nullptr;

#if STANDALONE_SYSTEM
namespace {
void __fastcall voice_mashed_destruct(ai::voice_box_inode *node) { node->_destruct_mashed_class(); }
void __fastcall voice_unmash(ai::voice_box_inode *node, void *, mash_info_struct *info, void *owner)
{
    node->_unmash(info, owner);
}
ai::voice_box_inode *__fastcall voice_delete(ai::voice_box_inode *node, void *, unsigned char flags)
{
    node->~voice_box_inode();
    if ((flags & 1) != 0)
        mem_dealloc(node, sizeof(*node));
    return node;
}
int __fastcall voice_type(const ai::voice_box_inode *) { return 456; }
bool __fastcall voice_subclass(const ai::voice_box_inode *, void *, int type) { return type == 537 || type == 573; }
bool __fastcall voice_needs_advance(const ai::voice_box_inode *) { return true; }
void __fastcall voice_advance(ai::voice_box_inode *node, void *, Float elapsed) { node->_frame_advance(elapsed); }
void __fastcall voice_activate(ai::voice_box_inode *node, void *, ai::ai_core *core) { node->_activate_voice(core); }
int __fastcall voice_size(const ai::voice_box_inode *) { return sizeof(ai::voice_box_inode); }
}

void *ai::voice_box_inode::native_vtable()
{
    static std::array<void *, 12> table = [] {
        std::array<void *, 12> result;
        std::copy_n(static_cast<void **>(info_node::native_vtable()), result.size(), result.begin());
        result[0x00 / 4] = reinterpret_cast<void *>(&voice_mashed_destruct);
        result[0x04 / 4] = reinterpret_cast<void *>(&voice_unmash);
        result[0x08 / 4] = reinterpret_cast<void *>(&voice_delete);
        result[0x0C / 4] = reinterpret_cast<void *>(&voice_type);
        result[0x10 / 4] = reinterpret_cast<void *>(&voice_subclass);
        result[0x18 / 4] = reinterpret_cast<void *>(&voice_needs_advance);
        result[0x1C / 4] = reinterpret_cast<void *>(&voice_advance);
        result[0x20 / 4] = reinterpret_cast<void *>(&voice_activate);
        result[0x2C / 4] = reinterpret_cast<void *>(&voice_size);
        return result;
    }();
    return table.data();
}
#else
void *ai::voice_box_inode::native_vtable() { return reinterpret_cast<void *>(0x0087DDD8); }
#endif

ai_lip_sync::ai_lip_sync(ai::voice_box_inode *voice)
    : owner(voice), resource(nullptr), morph(nullptr), morph_name{}
{

    queued_sounds.m_data = nullptr;
    queued_sounds.m_max_size = 0;
    queued_delays.m_data = nullptr;
    queued_delays.m_max_size = 0;
}

ai_lip_sync::~ai_lip_sync()
{

    if (!queued_delays.is_pointer_in_mash_image(queued_delays.m_data))
        delete[] queued_delays.m_data;
    if (!queued_sounds.is_pointer_in_mash_image(queued_sounds.m_data))
        delete[] queued_sounds.m_data;
}

ai::voice_box_inode::voice_box_inode()
    : info_node(), current_sound(0), pending_sound(), teams{}
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    initialize_voice(mash::ALLOCATED);
}

ai::voice_box_inode::voice_box_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), current_sound(0), pending_sound(constructor), teams{}
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    initialize_voice(mash::FROM_MASH);
}

ai::voice_box_inode::~voice_box_inode()
{

    finalize_voice(mash::ALLOCATED);
}

void ai::voice_box_inode::initialize_voice(mash::allocation_scope scope)
{

    if (scope == mash::FROM_MASH) {
        lip_sync = ::new (mem_alloc(sizeof(ai_lip_sync))) ai_lip_sync(this);
        if (speaking_voice_boxes == nullptr)
            speaking_voice_boxes = new std::set<voice_box_inode *>;
        speech_requests = ::new (mem_alloc(sizeof(*speech_requests))) _std::list<speech_request *>;

        if (my_param_block.param_array != nullptr) {
            for (int team = 0; team != 15; ++team) {
                auto *parameter = my_param_block.param_array->common_find_data(
                    string_hash{static_cast<int>(team_hashes[team])});
                if (parameter != nullptr)
                    teams[team] = parameter->m_union.i != 0;
            }
        }
    } else {
        current_sound = sound_instance_id(0);
        current_priority = -1;
        flags = 0;
        pending_sound = string_hash(0);
        lip_sync = nullptr;
        speech_requests = nullptr;
    }
    if (live_voice_boxes == nullptr)
        live_voice_boxes = new std::set<voice_box_inode *>;
    live_voice_boxes->insert(this);
}

void ai::voice_box_inode::_unmash(mash_info_struct *info, void *context)
{
    info_node::_unmash(info, context);
    pending_sound.unmash(info, this);
}

void ai::voice_box_inode::_activate_voice(ai_core *core)
{
    field_8 = core;
    field_C = core->field_64;
}

void ai::voice_box_inode::clear_speech_requests()
{
    for (auto *request : *speech_requests) {
        if (request != nullptr) {
            request->~speech_request();
            mem_dealloc(request, sizeof(*request));
        }
    }
    speech_requests->clear();
}

void ai::voice_box_inode::finalize_voice(mash::allocation_scope scope)
{

    if (scope == mash::FROM_MASH) {
        if (lip_sync != nullptr) {
            lip_sync->~ai_lip_sync();
            mem_dealloc(lip_sync, sizeof(*lip_sync));
        }
        clear_speech_requests();
        speech_requests->~list();
        mem_dealloc(speech_requests, sizeof(*speech_requests));
    }
    if (speaking_voice_boxes != nullptr)
        speaking_voice_boxes->erase(this);
    live_voice_boxes->erase(this);
    if (live_voice_boxes->empty()) {
        delete live_voice_boxes;
        live_voice_boxes = nullptr;
    }
}

void ai::voice_box_inode::_destruct_mashed_class()
{
    finalize_voice(mash::FROM_MASH);
    pending_sound.destruct_mashed_class();
    info_node::_destruct_mashed_class();
}

bool ai::voice_box_inode::is_any_voice_box_speaking_by_team(
    int team, const voice_box_inode *except)
{
    if (speaking_voice_boxes == nullptr)
        return false;
    static const string_hash team_id{static_cast<int>(to_hash("team"))};
    for (const auto *voice : *speaking_voice_boxes) {
        if (voice != except &&
            voice->field_8->field_50.get_pb_hash(team_id).source_hash_code == team_hashes[team])
            return true;
    }
    return false;
}

void ai::voice_box_inode::shut_up()
{
    if ((flags & 1) != 0) {
        lip_sync->stop_all();
        flags &= ~1;
    } else if (auto *sound = current_sound.get_sound_instance_ptr()) {
        sound->stop();
    }
    clear_speech_requests();
    if (speaking_voice_boxes != nullptr)
        speaking_voice_boxes->erase(this);
}

bool ai::voice_box_inode::service_speech_request(const speech_request &request)
{


    for (int team = 0; team != 15; ++team) {
        if (request.excluded_teams[team] && is_any_voice_box_speaking_by_team(team, this))
            return request.interruption != 2;
    }

    if ((flags & 1) != 0 || current_sound.get_sound_instance_ptr() != nullptr) {
        if (request.interruption == 2)
            return false;
        if (request.priority == 2 && current_priority == 2)
            return false;
        if ((request.interruption != 0 || request.priority <= current_priority) &&
            (request.interruption != 1 || request.priority < current_priority))
            return true;

        if ((flags & 1) != 0) {
            lip_sync->stop_all();
            flags &= ~1;
        } else if (auto *sound = current_sound.get_sound_instance_ptr()) {
            sound->stop();
        }
    }

    current_priority = request.priority;
    auto *sound_interface = field_C->my_sound_and_pfx_interface;
    switch (request.source_type) {
    case 0:
        if (lip_sync->play(request.sound)) {
            pending_sound = request.sound;
            flags |= 1;
        } else {
            current_sound = sound_instance::play_and_add(
                sound_interface, request.sound, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
        }
        break;
    case 1:
        current_sound = sound_interface->play_sound_grp(
            request.sound, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
        break;
    case 2: {
        static const string_hash speaker_id{static_cast<int>(to_hash("speaker_id"))};
        const char *speaker = nullptr;
        if (my_param_block.param_array != nullptr) {
            const auto *parameter = my_param_block.param_array->common_find_data(speaker_id);
            speaker = parameter->m_union.str;
        }
        const auto source = gab_manager::calc_gab_source(speaker, request.sound);
        if (source.is_valid())
            current_sound = sound_interface->play_sound(
                source, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
        break;
    }
    }
    return true;
}

bool ai::voice_box_inode::can_gab() const
{
    static const string_hash speaker_id{int(to_hash("speaker_id"))};
    return my_param_block.param_array != nullptr &&
        my_param_block.param_array->common_find_data(speaker_id) != nullptr;
}

bool ai::voice_box_inode::say_gab(string_hash sound, int interruption, int priority,
                                 const unsigned char *excluded_teams)
{
    auto *request = ::new (mem_alloc(sizeof(speech_request))) speech_request;
    request->sound = sound;
    request->source_type = 2;
    request->interruption = interruption;
    request->priority = priority;
    std::copy_n(excluded_teams != nullptr ? excluded_teams : teams, 15, request->excluded_teams);
    speech_requests->push_back(request);
    const bool was_speaking = (flags & 1) != 0 || current_sound.get_sound_instance_ptr() != nullptr;
    frame_advance(0.0f);
    return !was_speaking && ((flags & 1) != 0 || current_sound.get_sound_instance_ptr() != nullptr);
}

void ai::voice_box_inode::_frame_advance(Float elapsed_seconds)
{


    auto *owner = field_8->field_64;
    const auto alive = reinterpret_cast<bool(__fastcall *)(actor *, void *)>(
        get_vfunc(owner->m_vtbl, 0x50));
    if (!alive(owner, nullptr)) {
        const auto has_damage = reinterpret_cast<bool(__fastcall *)(actor *, void *)>(
            get_vfunc(owner->m_vtbl, 0x114));
        bool subdued = false;
        if (has_damage(owner, nullptr)) {
            const auto get_damage = reinterpret_cast<damage_interface *(__fastcall *)(actor *, void *)>(
                get_vfunc(owner->m_vtbl, 0x118));
            const auto *damage = get_damage(owner, nullptr);
            subdued = damage->field_21C.field_0[0] > EPSILON &&
                      damage->field_1FC.field_0[0] < EPSILON;
        }
        if (!subdued) {
            shut_up();
            return;
        }
    }

    lip_sync->frame_advance(elapsed_seconds);
    if (lip_sync->sound_start_due() && (flags & 1) != 0) {
        flags &= ~1;
        current_sound = sound_instance::play_and_add(
            field_C->my_sound_and_pfx_interface, pending_sound,
            1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
    }
    if ((flags & 1) == 0 && !speech_requests->empty()) {
        auto *request = speech_requests->front();
        if (service_speech_request(*request)) {
            speech_requests->pop_front();
            request->~speech_request();
            mem_dealloc(request, sizeof(*request));
        }
    }

    if ((flags & 1) != 0 || current_sound.get_sound_instance_ptr() != nullptr)
        speaking_voice_boxes->insert(this);
    else if (speaking_voice_boxes != nullptr)
        speaking_voice_boxes->erase(this);
}

void ai::voice_box_inode::sub_6D7E10(const char *a2)
{
    static const string_hash speaker_id{to_hash("speaker_id")};

    this->my_param_block.set_pb_fixedstring(speaker_id, a2, true);
}
