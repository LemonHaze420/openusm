#include "enum_anim_key.h"

#include "anim_record.h"
#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"

VALIDATE_SIZE(anim_key, 0x4);
VALIDATE_SIZE(enum_anim_key, 0x8);

enum_anim_key::enum_anim_key(enum_anim_key::key_enum a2)
{
    this->m_vtbl = 0x00873908;
    this->field_4 = a2;
}

anim_key::anim_key()
{
    this->m_vtbl = 0x008738E8;
}

int anim_key::get_compare_value(const anim_key *a2)
{
    int(__fastcall * func)(anim_key *, void *, const anim_key *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this, nullptr, a2);
}

int anim_key::get_mash_sizeof()
{
    int(__fastcall * func)(anim_key *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    return func(this);
}

int anim_key::compare(anim_key *&a1, anim_record *&a2)
{
    return a1->get_compare_value(a2->my_key);
}
