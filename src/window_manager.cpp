#include "window_manager.h"

#include "func_wrapper.h"

Var<window_manager *> window_manager::instance{0x00966008};

window_manager::~window_manager()
{
    THISCALL(0x005B24E0, this);
}

void create_window_handle()
{
    window_manager::instance() = new window_manager{};
}
