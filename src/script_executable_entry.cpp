#include "script_executable_entry.h"

#include "common.h"

VALIDATE_SIZE(script_executable_entry_key, 0x10);
VALIDATE_SIZE(script_executable_entry, 0x14);

using pair = std::pair<script_executable_entry_key, script_executable_entry>;

VALIDATE_SIZE(pair, 0x24);

bool script_executable_entry_key::operator<(const script_executable_entry_key &a1) const
{
    bool result = false;
    if (this->field_0 == a1.field_0 && this->field_8 <= a1.field_8) {
        result = true;
    } else {
        result = (this->field_0 <= a1.field_0);
    }

    return result;
}
