#include "daynight.h"

#include "city_lights.h"
#include "city_gradients.h"
#include "func_wrapper.h"
#include "game.h"
#include "ngl.h"
#include "trace.h"
#include "parse_generic_mash.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"

void daynight::frame_advance(Float)
{
    TRACE("daynight::frame_advance");

    constexpr float dawn_begin = 18000.0f;
    constexpr float dawn_end = 21600.0f;
    constexpr float dusk_begin = 64800.0f;
    constexpr float dusk_end = 68400.0f;
    constexpr float transition_rate = 1.0f / 3600.0f;

    const float time = g_game_ptr->get_script_game_clock_timer();
    current_time() = time;
    if (time < dawn_begin || time >= dusk_end) {
        glow_intensity() = 1.0f;
    } else if (time <= dawn_end) {
        glow_intensity() = 1.0f - (time - dawn_begin) * transition_rate;
    } else if (time < dusk_begin) {
        glow_intensity() = 0.0f;
    } else {
        glow_intensity() = (time - dusk_begin) * transition_rate;
    }

    update_shadow_settings();
    if (lights() != nullptr) {
        nglMatrix unused_a{};
        nglMatrix unused_b{};
        lights()->update(time, unused_a, unused_b);
    }
}

void daynight::update_shadow_settings()
{
#if STANDALONE_SYSTEM
    shadow_multiplier() = 1.0f;
#else
    CDECL_CALL(0x0051BCA0);
#endif
}

void daynight::init()
{
    TRACE("daynight::init");

    if constexpr (1) {
        assert(!initialized());

        initialized() = true;

        auto *common_partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_COMMON);
        assert(common_partition != nullptr);

        assert(common_partition->get_pack_slots().size() == 1);

        auto *common_slot = common_partition->get_pack_slots().at(0u);
        assert(common_slot != nullptr);

        resource_key a2{string_hash{"glob"}, (resource_key_type)38};
        int mash_data_size;
        auto *gradient_image = (char *)common_slot->get_resource(a2, &mash_data_size, nullptr);
        assert(gradient_image != nullptr);
        parse_generic_object_mash(gradients(), gradient_image, nullptr, nullptr, nullptr, 0u, 0u, nullptr);

        resource_key v10{string_hash{"glob"}, (resource_key_type)37};
        auto *light_image = (char *)common_slot->get_resource(v10, &mash_data_size, nullptr);
        assert(light_image != nullptr);

        parse_generic_object_mash(lights(), light_image, nullptr, nullptr, nullptr, 0u, 0u, nullptr);
    } else {
        CDECL_CALL(0x00550690);
    }
}

void daynight::kill()
{
    lights() = nullptr;
    gradients() = nullptr;
    initialized() = false;
}

void daynight_patch()
{
    REDIRECT(0x0055C88B, daynight::init);
}
