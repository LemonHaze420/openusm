#include "hashstring.h"

#include "fixedstring.h"

#include <cstdio>

tlHashString::tlHashString() {}

int tlHashString::compare(const tlHashString &a2) const
{
    if (this->field_0 == a2.field_0) {
        return 0;
    }

    return ((a2.field_0 < this->field_0) ? 1 : -1);
}

bool tlHashString::operator==(const tlHashString &a2) const
{
    return a2.field_0 == this->field_0;
}

bool tlHashString::operator==(const tlFixedString &a2) const
{
    return this->field_0 == a2.GetHash();
}

const char *tlHashString::c_str() const
{
    static char byte_15D6574[12];
    sprintf(byte_15D6574, "0x%08x", this->field_0);
    return byte_15D6574;
}
