#include "signaller.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(signaller, 0x48);

signaller::signaller(bool a2) : entity_base(a2) {}

signaller::signaller(const string_hash &a2, uint32_t a3, bool a4) : entity_base(a2, a3, a4) {}

signaller::~signaller()
{
    THISCALL(0x004F9120, this);
}

bool signaller::sub_48AE20()
{
    bool result = THISCALL(0x0048AE20, this);
    return result;
}

int signaller::get_entity_size()
{
    return 72;
}

void signaller::release_mem()
{
    THISCALL(0x004F9130, this);
}
