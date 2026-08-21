#include "controller.h"

#include "memory.h"
#include "trace.h"
#include "utility.h"

controller::controller() {}

void *controller::operator new(size_t size)
{
    return mem_alloc(size);
}

void controller::kill()
{
    TRACE("controller::kill");

    this->field_4 = false;
}

void controller::resurrect()
{
    TRACE("controller::resurrect");

    this->field_4 = true;
}

bool controller::is_controller() const
{
    return true;
}

void controller_patch()
{
    {
        FUNC_ADDRESS(address, &controller::kill);
        SET_JUMP(0x0055E7C0, address);
    }

    {
        FUNC_ADDRESS(address, &controller::resurrect);
        SET_JUMP(0x0055E7D0, address);
    }
}
