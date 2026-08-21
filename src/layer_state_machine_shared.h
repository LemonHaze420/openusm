#pragma once

#include "state_machine_shared.h"

#include "als_layer_types.h"

struct from_mash_in_place_constructor;

namespace als {
struct layer_state_machine_shared : state_machine_shared {
    int field_40;
    layer_types field_44;

    layer_state_machine_shared();

    layer_state_machine_shared(from_mash_in_place_constructor *);

    //virtual
    void _unmash(mash_info_struct *a1, void *a3);

    //0x004931E0
    //virtual
    int _get_virtual_type_enum() const;

    //virtual
    layer_types _get_layer_id() const;

    //virtual
    int _get_mash_sizeof() const;

    //virtual
    void _set_layer_id(layer_types a2);
};
}  // namespace als

extern void als_layer_state_machine_shared_patch();
