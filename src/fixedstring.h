#pragma once

#include <cstdint>

#include "utility.h"

template<int ndwords = 2>
struct fixedstring {

    static inline constexpr auto MAX_CHARS = ndwords * 4;

    unsigned value[ndwords];

    fixedstring() = default;

    fixedstring(const char *str)
    {
        assert(str != nullptr);

        auto len = strlen(str);

        assert(len <= MAX_CHARS && "String too big for fixed size.");

        for (auto  i = 0u; i < ndwords; ++i )
        {
            this->value[i] = 0;
        }

        std::memcpy(this->value, str, len);
    }

    inline bool operator==( const fixedstring<ndwords> & rhs ) const
    {
        for (int i = 0; i < ndwords; ++i)
        {
            if (value[i] != rhs.value[i])
            {
                return false;
            }
        }

        return true;
    }

    const char *to_string() const
    {
        return bit_cast<const char *>(&this->value);
    }
};

struct tlFixedString {
    uint32_t m_hash;
    char field_4[28];

    tlFixedString() = default;

    tlFixedString(const tlFixedString &) = default;

    //0x004018D0
    tlFixedString(const char *a1);

    auto GetHash() const {
        return this->m_hash;
    }

    const char *to_string() const {
        return field_4;
    }

    const char * c_str() const {
        return field_4;
    }

    tlFixedString *operator=(const tlFixedString &a1);

    int compare(const tlFixedString &a2) const;

    bool operator==(const tlFixedString &a2) const;

    bool operator!=(const tlFixedString &a2) const;
};

struct tlresource_location;

//0x00501C30
extern int compare_tlFixedString_tlresource_location(uint32_t &a1, tlresource_location &a2);
