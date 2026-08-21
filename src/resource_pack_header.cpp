#include "resource_pack_header.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "utility.h"
#include "variables.h"

#include <cassert>

VALIDATE_SIZE(resource_pack_header, 0x2Cu);

resource_pack_header::resource_pack_header() : field_0()
{
    this->field_14 = 0;
    this->directory_offset = 0;
    this->res_dir_mash_size = 0;
    this->field_20 = 0;
    this->field_24 = 0;
    this->field_28 = 0;
}

void resource_pack_header::clear()
{
    this->field_0 = {};

    this->field_14 = 0;
    this->directory_offset = 0;
    this->res_dir_mash_size = 0;
    this->field_20 = 0;
    this->field_24 = 0;
    this->field_28 = 0;
}

bool resource_pack_header::verify(resource_key a2) const
{
    if constexpr (1) {
        //sp_log("0x%08X %d", a2.field_0, a2.m_type);

        static constexpr resource_versions v20 = {RESOURCE_PACK_VERSION,
                                                  RESOURCE_ENTITY_MASH_VERSION,
                                                  RESOURCE_NONENTITY_MASH_VERSION,
                                                  RESOURCE_AUTO_MASH_VERSION,
                                                  RESOURCE_RAW_MASH_VERSION};

        bool v29 = false, v28 = false;

        if (this->field_0.field_0 < v20.field_0 || this->field_0.field_4 < v20.field_4 ||
            this->field_0.field_8 < v20.field_8 || this->field_0.field_C < v20.field_C ||
            this->field_0.field_10 < v20.field_10) {
            v28 = true;
        } else if (this->field_0.field_0 > v20.field_0 || this->field_0.field_4 > v20.field_4 ||
                   this->field_0.field_8 > v20.field_8 || this->field_0.field_C > v20.field_C ||
                   this->field_0.field_10 > v20.field_10) {
            v29 = true;
        }

        if (v29) {
            auto v18 = v20.to_string();
            auto v19 = this->field_0.to_string();

            auto v8 = a2.m_hash.sub_501E80();
            auto v2 = v8.to_string();
            sp_log("Error. The pack file %s (v%s) is newer than this executable code (v%s).",
                   v2,
                   v19.c_str(),
                   v18.c_str());
#ifndef TARGET_XBOX
            assert(0);
#endif

            return false;
        } else if (v28) {
            auto v18 = v20.to_string();
            auto v19 = this->field_0.to_string();

            auto v10 = a2.m_hash.sub_501E80();
            auto v2 = v10.to_string();

            sp_log("Error. The pack file %s (v%s) is older than this executable code (v%s).",
                   v2,
                   v19.c_str(),
                   v18.c_str());

#ifndef TARGET_XBOX
            assert(0);
#endif

            return false;
        } else {
            if (this->field_14 & 1) {
                auto v14 = a2.m_hash.sub_501E80();
                auto v5 = v14.to_string();

                sp_log("Error. The pack file %s has sync testing ON, but this executable has sync "
                       "testing OFF.",
                       v5);
            }
        }

#ifdef TARGET_XBOX
        assert(0 && "Load amalgapak from xbox");
#endif

        return true;
    } else {
        return (bool)THISCALL(0x00537DA0, this, a2);
    }
}

void resource_pack_header_patch()
{
    {
        FUNC_ADDRESS(address, &resource_pack_header::verify);
        REDIRECT(0x0053E1D2, address);
    }
}
