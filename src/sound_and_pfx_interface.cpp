#include "sound_and_pfx_interface.h"

#include "common.h"

VALIDATE_SIZE(sound_and_pfx_interface, 0x40);

web_sound_params *shared_sound_interface_info::get_web_sound_params(string_hash a2)
{
    if constexpr (0) {
    } else {
        web_sound_params *(__fastcall * func)(void *, void *, string_hash a2) = CAST(func, 0x00564670);
        return func(this, nullptr, a2);
    }
}

sound_and_pfx_interface::sound_and_pfx_interface() {}

web_sound_params *sound_and_pfx_interface::get_web_sound_params(string_hash a1)
{
    if (this->field_10 != nullptr) {
        auto *v6 = this->field_10->get_web_sound_params(a1);
        return v6;
    } else {
        return nullptr;
    }
}

void sound_and_pfx_interface::release_ifc()
{
    pfx_interface::release_ifc();
}
