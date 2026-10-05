#include "signaller.h"

#include "common.h"
#include "entity_mash.h"
#include "func_wrapper.h"
#include "conglom.h"

VALIDATE_SIZE(signaller, 0x48);

signaller::signaller(bool a2) : entity_base(a2)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[1];
#endif
}

signaller::signaller(const string_hash &a2, uint32_t a3, bool a4) : entity_base(a2, a3, a4)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[1];
#endif
}

signaller::~signaller() = default;

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
    entity_base::release_mem();
}

void signaller::set_occluded_last_frame_all()
{
    ++occlusion_status_base;
}

void signaller::set_occluded_last_frame(bool occluded)
{
    field_44 = occlusion_status_base - int(occluded);
}

bool signaller::get_occluded_last_frame() const
{
    const signaller *root = this;
    while (root->is_flagged(0x8000))
        root = static_cast<const signaller *>(root->get_conglom_owner());
    return root->field_44 != occlusion_status_base;
}
