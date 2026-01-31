#include "worldly_resource_handler.h"

#include "common.h"
#include "memory.h"
#include "vtbl.h"

VALIDATE_SIZE(worldly_resource_handler, 0x10);

void * worldly_resource_handler::operator new(std::size_t sz) {
    return mem_alloc(sz);
}

void worldly_resource_handler::operator delete(void *ptr, std::size_t sz) {
    return mem_dealloc(ptr, sz);
}

bool worldly_resource_handler::handle(eBehavior behavior, limited_timer *a5) {
    bool (__fastcall *func)(void *, int, eBehavior, limited_timer *) =
        CAST(func, get_vfunc(m_vtbl, 0x4));

    return func(this, 0, behavior, a5);
}
