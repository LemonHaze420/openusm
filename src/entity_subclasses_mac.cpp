#include "entity_subclasses_mac.h"

#include "func_wrapper.h"
#include "item.h"

#include <cassert>

//0x00601AB0
template <>
item *to_pointer(
    const vhandle_type<item, vhandle_type<actor, vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>>>>
        *a1)
{
    if (!a1->field_0.field_0) {
        return nullptr;
    }

    auto *my_ent = static_cast<item *>(a1->field_0.get_volatile_ptr());

    assert(my_ent != nullptr && "Dangling item pointer!  (You asked for streaming, and this comes with the package)");

    assert(my_ent != nullptr && my_ent->is_an_item());

    return my_ent;
}
