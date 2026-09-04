#pragma once

#include <cstdint>

struct tlFixedString;

struct tlHashString {
    uint32_t field_0;

    tlHashString();

    tlHashString(uint32_t v) : field_0(v) {}

    auto GetHash() const
    {
        return this->field_0;
    }

    bool operator==(const tlHashString &a2) const;

    bool operator==(const tlFixedString &a2) const;

    int compare(const tlHashString &a2) const;

    //0x0074A500
    const char *c_str() const;
};
