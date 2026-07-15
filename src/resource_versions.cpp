#include "resource_versions.h"

mString resource_versions::to_string() const {
    mString result = mString{mString::fmtd{0},
                             "%d.%d.%d.%d.%d",
                             this->field_0,
                             this->field_4,
                             this->field_8,
                             this->field_C,
                             this->field_10};
    return result;
}

resource_versions expected_resource_versions(_nlPlatformEnum platform)
{
    if (platform == NL_PLATFORM_XBOX) {
        return XBOX_V14_RESOURCE_VERSIONS;
    }

    return {RESOURCE_PACK_VERSION,
            RESOURCE_ENTITY_MASH_VERSION,
            RESOURCE_NONENTITY_MASH_VERSION,
            RESOURCE_AUTO_MASH_VERSION,
            RESOURCE_RAW_MASH_VERSION};
}

bool supports_xbox_version(const resource_versions &versions)
{
    return versions.field_0 == XBOX_V14_RESOURCE_VERSIONS.field_0 &&
           versions.field_4 == XBOX_V14_RESOURCE_VERSIONS.field_4 &&
           versions.field_8 == XBOX_V14_RESOURCE_VERSIONS.field_8 &&
           versions.field_C == XBOX_V14_RESOURCE_VERSIONS.field_C &&
           versions.field_10 == XBOX_V14_RESOURCE_VERSIONS.field_10;
}
