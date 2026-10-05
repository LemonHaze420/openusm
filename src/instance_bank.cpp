#include "instance_bank.h"

#include "colmesh.h"
#include "func_wrapper.h"


static_assert(sizeof(instance_bank<cg_mesh>) == 0x18);
static_assert(offsetof(instance_bank<cg_mesh>::entry, object) == 0x10);

template <>
void instance_bank<cg_mesh>::purge()
{
    if constexpr (STANDALONE_SYSTEM) {
        for (auto *item : entries) {
            delete item->object;
            delete item;
        }
        entries.clear();
        instances.clear();
    } else {
        THISCALL(0x0056DDE0, this);
    }
}
