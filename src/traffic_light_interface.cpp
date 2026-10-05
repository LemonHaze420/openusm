#include "traffic_light_interface.h"

#include "actor.h"

#include "common.h"
#include "memory.h"
#include "parse_generic_mash.h"
#include "traffic_signal_mgr.h"
#include "variables.h"

#include <array>

#if STANDALONE_SYSTEM
namespace {
void *__fastcall destroy_traffic_light(traffic_light_interface *self, void *, unsigned char flags)
{
    self->~traffic_light_interface();
    if ((flags & 1) != 0)
        mem_dealloc(self, sizeof(*self));
    return self;
}


bool __fastcall traffic_property_unsupported(traffic_light_interface *, void *, std::uintptr_t, std::uintptr_t, bool)
{
    return false;
}

void __fastcall unmash_traffic_light(traffic_light_interface *self, void *, generic_mash_header *header, void *owner,
                                     void *data, generic_mash_data_ptrs *cursor)
{
    self->actor_interface::un_mash(header, owner, data, cursor);
    self->field_C = *cursor->get<int>();
}

const char *__fastcall traffic_light_type(traffic_light_interface *, void *)
{
    return "traffic_light";
}

void __fastcall release_traffic_light(traffic_light_interface *self, void *)
{
    traffic_signal_mgr::remove_traffic_light(self->my_actor);
}

std::intptr_t traffic_light_vtable()
{
    static const std::array<std::intptr_t, 10> table{
        reinterpret_cast<std::intptr_t>(&destroy_traffic_light),
        reinterpret_cast<std::intptr_t>(&traffic_property_unsupported),
        reinterpret_cast<std::intptr_t>(&traffic_property_unsupported),
        reinterpret_cast<std::intptr_t>(&traffic_property_unsupported),
        reinterpret_cast<std::intptr_t>(&traffic_property_unsupported),
        reinterpret_cast<std::intptr_t>(&traffic_property_unsupported),
        reinterpret_cast<std::intptr_t>(&traffic_property_unsupported),
        reinterpret_cast<std::intptr_t>(&unmash_traffic_light),
        reinterpret_cast<std::intptr_t>(&traffic_light_type),
        reinterpret_cast<std::intptr_t>(&release_traffic_light),
    };
    return reinterpret_cast<std::intptr_t>(table.data());
}
}  // namespace
#endif

VALIDATE_SIZE(traffic_light_interface, 0x10);

traffic_light_interface::traffic_light_interface(actor *a2) : actor_interface(a2)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = traffic_light_vtable();
#else
    this->m_vtbl = 0x00883048;
#endif
    this->field_C = 0;
}

traffic_light_interface::~traffic_light_interface()
{
    if (!g_generating_vtables)
        traffic_signal_mgr::remove_traffic_light(my_actor);
    my_actor = nullptr;
}

void *traffic_light_interface::operator new(std::size_t sz)
{
    return mem_alloc(sz);
}

void traffic_light_interface::operator delete(void *ptr, std::size_t sz)
{
    return mem_dealloc(ptr, sz);
}
