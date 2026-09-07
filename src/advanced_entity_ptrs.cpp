#include "advanced_entity_ptrs.h"

#include "common.h"
#include "parse_generic_mash.h"
#include <func_wrapper.h>

VALIDATE_SIZE(movement_info, 0x58);
VALIDATE_SIZE(advanced_entity_ptrs, 0x14);
VALIDATE_OFFSET(advanced_entity_ptrs, mi, 0xC);

advanced_entity_ptrs::~advanced_entity_ptrs()
{
    THISCALL(0x004EF1C0, this);
}

void advanced_entity_ptrs::un_mash(generic_mash_header *header,
                                    actor *arg4,
                                    void *object,
                                    generic_mash_data_ptrs *data)
{
#if STANDALONE_SYSTEM
    (void)arg4;
    (void)object;
    (void)header;
    const bool has_render_data = this->field_8 != nullptr;

    this->coninfo = nullptr;
    this->ignore_col_ents = nullptr;
    this->field_8 = nullptr;
    this->mi = nullptr;
    this->my_script = nullptr;

    if (has_render_data) {
        data->rebase(4);
        this->field_8 = data->get<render_data>();
    }

#else
    THISCALL(0x004CFCE0, this, header, arg4, object, data);
#endif
}
