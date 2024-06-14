#include "ghetto_mash_file_header.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "utility.h"
#include "trace.h"

#include <cstring>

VALIDATE_SIZE(ghetto_mash_file_header, 8);

ghetto_mash_file_header::ghetto_mash_file_header(mash::allocation_scope a2, const char *a3, int a4) {
    this->initialize(a2, a3, a4);
}

void ghetto_mash_file_header::initialize(mash::allocation_scope a2, const char *a3, int a4)
{
    if (a2 == mash::ALLOCATED)
    {
        if (a3 != nullptr) {
            std::strcpy(this->field_0, a3);
        }

        this->field_4 = a4;
    }
}

void ghetto_mash_file_header::unmash(mash_info_struct *, void *) {
    TRACE("ghetto_mash_file_header::unmash");
    ;
}

bool ghetto_mash_file_header::validate(const char *a2, int a3) const
{
    bool result;
    if constexpr (1) {
        result = (std::strncmp(this->field_0, a2, 3u) == 0 && this->field_4 == a3);
    } else {
        bool (__fastcall *func)(const void *, void *edx, const char *, int) = CAST(func, 0x00420650);
        result = func(this, nullptr, a2, a3);
    }

    return result;
}

void ghetto_mash_file_header_patch() {
    {
        FUNC_ADDRESS(address, &ghetto_mash_file_header::validate);
        REDIRECT(0x00550EE9, address);
    }
}
