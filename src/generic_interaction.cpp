#include "generic_interaction.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(generic_interaction, 0x54u);

generic_interaction::generic_interaction(const resource_key &resource_str, interaction_type_enum a3, string_hash a4)
    : interaction(a3, a4)
{
    assert(resource_str.get_type() == RESOURCE_KEY_TYPE_AI_INTERACTION);

    THISCALL(0x004D5DB0, this, &resource_str, a3, a4);
}
