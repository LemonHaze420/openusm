#include "entity_trigger.h"

#include "common.h"

VALIDATE_SIZE(entity_trigger, 0x68);

entity_trigger::entity_trigger(string_hash a2, entity_base *a3, Float a4) : trigger(a2)
{
    this->m_vtbl = 0x0088A240;
    this->field_58 = a3->get_my_vhandle();
    this->field_48 = a4;

    auto v4 = a3->get_id();
    auto *v5 = v4.to_string();
    mString v9{v5};

    auto *v6 = v9.c_str();
    sp_log("constructing entity trigger tied to %s", v6);
}

entity_base *entity_trigger::get_ent()
{
    auto *ent = this->field_58.get_volatile_ptr();
    if (ent != nullptr) {
        return ent;
    }

    this->field_58.field_0 = 0;
    return nullptr;
}
