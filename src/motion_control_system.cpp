#include "motion_control_system.h"

#include "memory.h"

motion_control_system::motion_control_system() {}

void *motion_control_system::operator new(size_t size)
{
    return mem_alloc(size);
}
