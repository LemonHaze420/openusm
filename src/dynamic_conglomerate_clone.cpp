#include "dynamic_conglomerate_clone.h"

#include "common.h"
#include "oldmath_po.h"
#include "variable.h"

po *dynamic_conglomerate_clone::get_member_abs_po(entity_base *)
{
    static Var<po> stru_960A38{0x00960A38};

    return &stru_960A38();
}
