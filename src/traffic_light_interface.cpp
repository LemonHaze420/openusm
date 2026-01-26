#include "traffic_light_interface.h"

#include "common.h"
#include "memory.h"

VALIDATE_SIZE(traffic_light_interface, 0x10);

traffic_light_interface::traffic_light_interface(
        actor *a2) : actor_interface(a2)
{
    this->m_vtbl = 0x00883048;
    this->field_C = 0;
}

void * traffic_light_interface::operator new(std::size_t sz)
{
    return mem_alloc(sz);
}

void traffic_light_interface::operator delete(void *ptr, std::size_t sz)
{
    return mem_dealloc(ptr, sz);
}
