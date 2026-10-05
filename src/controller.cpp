#include "controller.h"

#include "common.h"
#include "memory.h"
#include "trace.h"
#include "utility.h"

controller::controller()
{
#if STANDALONE_SYSTEM
    field_4 = true;
    field_5 = false;
#endif
}

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

#if STANDALONE_SYSTEM
namespace {
void __fastcall native_controller_kill(controller *self, void *)
{
    self->kill();
}

void __fastcall native_controller_resurrect(controller *self, void *)
{
    self->resurrect();
}


void __fastcall native_controller_handle_input(controller *, void *, int)
{
}

bool __fastcall native_controller_false(controller *, void *)
{
    return false;
}

bool __fastcall native_controller_true(controller *, void *)
{
    return true;
}
}

void controller::initialize_native_vtable(std::intptr_t *table, std::intptr_t destroy,
                                          std::intptr_t advance, bool mouselook)
{
    table[0] = destroy;
    table[1] = advance;
    table[2] = reinterpret_cast<std::intptr_t>(native_controller_kill);
    table[3] = reinterpret_cast<std::intptr_t>(native_controller_resurrect);
    table[4] = reinterpret_cast<std::intptr_t>(native_controller_handle_input);
    table[5] = reinterpret_cast<std::intptr_t>(native_controller_false);
    table[6] = reinterpret_cast<std::intptr_t>(native_controller_true);
    table[7] = reinterpret_cast<std::intptr_t>(mouselook ? native_controller_true
                                                     : native_controller_false);
    table[8] = reinterpret_cast<std::intptr_t>(native_controller_false);
    table[9] = reinterpret_cast<std::intptr_t>(native_controller_false);
    table[10] = reinterpret_cast<std::intptr_t>(native_controller_false);
}
#endif
