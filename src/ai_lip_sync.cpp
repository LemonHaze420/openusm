#include "ai_voice_box_inode.h"

#include "actor.h"
#include "common.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_morph.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "filespec.h"

#include <cstring>
#include <new>
#include <functional>


struct VisemeStream {
    int viseme_count;
    int phoneme_count;
    int frame_count;
    int frames_per_second;
    float sound_start_delay;
    float *weights;

    bool apply(nglMesh *mesh, nglMorphSet *morph, float elapsed) const;
};

VALIDATE_SIZE(VisemeStream, 0x18);
VALIDATE_SIZE(ai_lip_sync, 0x50);


bool VisemeStream::apply(nglMesh *mesh, nglMorphSet *morph, float elapsed) const
{
    const auto frame = static_cast<int32_t>(static_cast<int64_t>(static_cast<double>(frames_per_second) * elapsed));
    if (frame >= frame_count)
        return true;

    const float *sample = weights + frame * viseme_count;
    nglMorphFrame frames[15];
    nglMorphEntry entries[15];
    uint32_t count = 0;
    for (int index = 0; index < viseme_count; ++index) {
        if (std::not_equal_to<float>{}(sample[index], 0.0f)) {
            new (&frames[count]) nglMorphFrame{morph->Frames + index + 1};
            entries[count] = {sample[index], &frames[count]};
            ++count;
        }
    }
    if (count != 0)
        nglBlendMorphs(mesh, count, entries);
    return false;
}

bool ai_lip_sync::sound_start_due() const
{
    return resource != nullptr && resource->sound_start_delay < elapsed;
}


bool ai_lip_sync::play(string_hash sound)
{
    constexpr auto viseme_resource_type = static_cast<resource_key_type>(50);
    const resource_key key{sound, viseme_resource_type};
    int size = 0;
    resource_manager::push_resource_context(owner->get_actor()->get_resource_context());
    auto *stream = reinterpret_cast<VisemeStream *>(resource_manager::get_resource(key, &size, nullptr));
    resource_manager::pop_resource_context();
    if (stream == nullptr) {
        resource_manager::push_resource_context(
            resource_manager::get_best_context(static_cast<resource_partition_enum>(3)));
        stream = reinterpret_cast<VisemeStream *>(resource_manager::get_resource(key, &size, nullptr));
        resource_manager::pop_resource_context();
        if (stream == nullptr)
            return false;
    }
    auto *actor = owner->get_actor();
    if (resource != nullptr)
        actor->field_90.end_buffering();
    stream->viseme_count = 15;
    stream->phoneme_count = 11;
    stream->weights = nullptr;
    resource = stream;
    stream->weights = reinterpret_cast<float *>(stream + 1);
    actor->field_90.start_buffering(2);
    morph = actor->get_morph(tlFixedString{morph_name}, true);
    if (morph == nullptr)
        return false;

    auto *mesh = actor->get_mesh();
    for (uint32_t index = 0; index < mesh->NSections; ++index) {
        if (mesh->Sections[index].Section->VertexDef == nullptr)
            morph->Frames[0].field_8[index].field_4 = 0;
    }
    elapsed = 0.0f;
    return true;
}


void ai_lip_sync::frame_advance(float elapsed_seconds)
{
    for (int index = 0; index < queued_sounds.m_size;) {
        const double remaining = static_cast<double>(queued_delays.m_data[index]) - elapsed_seconds;
        queued_delays.m_data[index] = static_cast<float>(remaining);
        if (!(remaining < 0.0)) {
            ++index;
            continue;
        }

        const string_hash sound = queued_sounds.m_data[index];
        if (!play(sound))
            sound.to_string();
        const int sounds_after = queued_sounds.m_size - index - 1;
        if (sounds_after != 0)
            std::memmove(
                queued_sounds.m_data + index, queued_sounds.m_data + index + 1, sounds_after * sizeof(string_hash));
        --queued_sounds.m_size;
        const int delays_after = queued_delays.m_size - index - 1;
        if (delays_after != 0)
            std::memmove(queued_delays.m_data + index, queued_delays.m_data + index + 1, delays_after * sizeof(float));
        --queued_delays.m_size;
    }


    if (resource == nullptr)
        return;
    elapsed += elapsed_seconds;
    if (morph == nullptr)
        return;
    if (resource->apply(owner->get_actor()->get_mesh(), morph, elapsed)) {
        resource = nullptr;
        owner->get_actor()->field_90.end_buffering();
        morph = nullptr;
    }
}


void ai_lip_sync::stop_all()
{
    const int removed = queued_sounds.m_size;
    if (removed != 0) {
        const int delays_remaining = queued_delays.m_size - removed;
        if (delays_remaining != 0)
            std::memmove(queued_delays.m_data, queued_delays.m_data + removed, delays_remaining * sizeof(float));
        queued_sounds.m_size = 0;
        queued_delays.m_size = delays_remaining;
    }
    if (resource == nullptr)
        return;
    resource = nullptr;
    owner->get_actor()->field_90.end_buffering();
    morph = nullptr;
}

void ai_lip_sync::set_morph_name(const mString &name)
{
    const filespec path{name};
    std::memset(morph_name, 0, sizeof(morph_name));
    std::memcpy(morph_name, path.m_name.c_str(), std::strlen(path.m_name.c_str()));
}

void ai_lip_sync::queue(string_hash sound, float delay)
{
    queued_sounds.push_back(sound);
    queued_delays.push_back(delay);
}
