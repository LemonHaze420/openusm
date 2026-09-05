#include "sound_instance_id.h"

#include "common.h"

#include "func_wrapper.h"

VALIDATE_SIZE(sound_instance_slot, 0x54);

#if STANDALONE_SYSTEM
static sound_instance_slot s_sound_instance_slot_storage[128]{};
static sound_instance_slot *s_sound_instance_slot_storage_ptr = s_sound_instance_slot_storage;
sound_instance_slot *&s_sound_instance_slots = s_sound_instance_slot_storage_ptr;
#else
sound_instance_slot *&s_sound_instance_slots = var<sound_instance_slot *>(0x0095C830);
#endif

void sound_instance::stop()
{
    THISCALL(0x0053E6F0, this);
}

sound_instance_id sub_60B960(string_hash a2, Float a3, Float a4)
{
    sound_instance_id result;
    CDECL_CALL(0x0060B960, &result, a2, a3, a4);

    return result;
}

sound_instance *sound_instance_id::get_sound_instance_ptr()
{
#if STANDALONE_SYSTEM

    const auto handle = this->field_0;
    const auto slot_index = static_cast<uint16_t>(handle & 0xffffu);
    const auto generation = static_cast<uint16_t>(handle >> 16);
    if (generation == 0 || slot_index >= 128 || s_sound_instance_slots == nullptr) {
        return nullptr;
    }

    auto *slot = &s_sound_instance_slots[slot_index];
    if (slot->field_50 != generation) {
        return nullptr;
    }

    return reinterpret_cast<sound_instance *>(slot);
#else
    return (sound_instance *)THISCALL(0x0050EF00, this);
#endif
}
