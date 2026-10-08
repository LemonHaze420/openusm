#include "window_manager.h"

Var<window_manager *> window_manager::instance{0x00966008};

window_manager::~window_manager() = default;

void create_window_handle()
{
    window_manager::instance() = new window_manager{};
}
