#include "fixedstring.h"

#include "common.h"
#include "log.h"
#include "string_hash.h"
#include "tlresource_location.h"

#include <cassert>
#include <ctype.h>
#include <string.h>

VALIDATE_SIZE(fixedstring<8>, 0x20);

tlFixedString::tlFixedString(const char *a1) : tlFixedString() {
    this->m_hash = to_hash(a1);

    strncpy(this->field_4, a1, 27u);
    strlwr(this->field_4);
}

int tlFixedString::compare(const tlFixedString &a2) {
    for (auto i = 0; i < 8; ++i) {
        auto func = [](const void *self) -> const uint32_t * {
            //
            return static_cast<const uint32_t *>(self);
        };

        auto *v2 = func(this);
        if (v2[i] != func(&a2)[i]) {
            return func(&a2)[i] < v2[i] ? 1 : -1;
        }
    }
    return 0;
}

bool tlFixedString::operator==(const tlFixedString &a2) const {
    for (int i = 0; i < 8; ++i) {
        auto func = [](const void *self) -> const int * {
            //
            return static_cast<const int *>(self);
        };

        auto *v2 = func(this);
        if (v2[i] != func(&a2)[i]) {
            return false;
        }
    }

    return true;
}

bool tlFixedString::operator!=(const tlFixedString &a2) const {
    for (auto i = 0u; i < 8u; ++i) {
        auto func = [](const void *self) -> const int * {
            //
            return static_cast<const int *>(self);
        };

        auto *v2 = func(this);
        if (v2[i] != func(&a2)[i]) {
            return true;
        }
    }

    return false;
}

tlFixedString *tlFixedString::operator=(const tlFixedString &a1) {
    for (auto i = 0u; i < 8u; ++i) {
        auto func = [](const void *self) -> const int * {
            //
            return static_cast<const int *>(self);
        };

        auto func1 = [](void *self) -> int * {
            //
            return static_cast<int *>(self);
        };

        auto *v2 = func(&a1);
        func1(this)[i] = v2[i];
    }
    return this;
}

int compare_tlFixedString_tlresource_location(uint32_t *a1, tlresource_location *a2) {
    int result;
    if (a2->name.source_hash_code == *a1) {
        result = 0;
    } else {
        result = (a2->name.source_hash_code < *a1 ? 1 : -1);
    }

    return result;
}
