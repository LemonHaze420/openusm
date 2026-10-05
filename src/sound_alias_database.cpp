#include "sound_alias_database.h"

#include "func_wrapper.h"
#include "utility.h"
#include "common.h"
#include "mash_info_struct.h"
#include "variables.h"

#include <cassert>

VALIDATE_SIZE(sound_alias, 0x20);
VALIDATE_SIZE(sound_alias_database, 0x14);

#if !STANDALONE_SYSTEM

sound_alias_database *&s_sound_alias_database = var<sound_alias_database *>(0x0095C854);

#else

sound_alias_database *&s_sound_alias_database = []() -> auto & {
    static sound_alias_database *s_sound_alias_database1{};
    return s_sound_alias_database1;
}();

#endif

sound_alias::sound_alias(from_mash_in_place_constructor *a2) : field_0(a2), field_4(a2) {}

sound_alias_database::sound_alias_database(from_mash_in_place_constructor *a2) : field_0(a2) {}

void sound_alias_database::destruct_mashed_class()
{
    this->field_0.destruct_mashed_class();
}

void sound_alias_database::unmash(mash_info_struct *a1, void *a3)
{
    a1->unmash_class_in_place(this->field_0, a3);
}

sound_alias *sound_alias_database::get_sound_alias(string_hash hash)
{
#if STANDALONE_SYSTEM
    auto first = 0;
    auto count = field_0.m_size;
    while (count > 0) {
        const auto step = count / 2;
        const auto index = first + step;
        const auto candidate = field_0.m_data[index]->field_0.source_hash_code;
        if (candidate < hash.source_hash_code) {
            first = index + 1;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    if (first < field_0.m_size &&
        field_0.m_data[first]->field_0.source_hash_code == hash.source_hash_code) {
        return field_0.m_data[first];
    }
    return nullptr;
#else
    return reinterpret_cast<sound_alias *>(THISCALL(0x005C9E50, this, hash));
#endif
}

void sound_alias_database_patch()
{
    FUNC_ADDRESS(address, &sound_alias_database::get_sound_alias);
    REDIRECT(0x005204D3, address);
}
