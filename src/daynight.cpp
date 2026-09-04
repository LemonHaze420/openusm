#include "daynight.h"

#include "city_lights.h"
#include "city_gradients.h"
#include "func_wrapper.h"
#include "trace.h"
#include "parse_generic_mash.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"

void daynight::frame_advance(Float a1)
{
    TRACE("daynight::frame_advance");

    CDECL_CALL(0x00527F80, a1);
}

void daynight::update_shadow_settings()
{
    CDECL_CALL(0x0051BCA0);
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
