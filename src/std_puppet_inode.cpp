#include "std_puppet_inode.h"

#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "base_state.h"
#include "common.h"
#include <algorithm>
#include <array>

namespace ai {

VALIDATE_SIZE(std_puppet_inode, 0x20);

namespace {
void *__fastcall delete_puppet(std_puppet_inode *self, void *, unsigned flags)
{
    self->~std_puppet_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(std_puppet_inode));
    return self;
}
unsigned __fastcall puppet_type(std_puppet_inode *, void *) { return 315; }
bool __fastcall puppet_subclass(std_puppet_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall puppet_needs_advance(std_puppet_inode *, void *) { return true; }
void __fastcall advance_puppet(std_puppet_inode *self, void *, Float dt) { self->frame_advance(dt); }
void __fastcall activate_puppet(std_puppet_inode *self, void *, ai_core *core) { self->activate(core); }
int __fastcall puppet_size(std_puppet_inode *, void *) { return sizeof(std_puppet_inode); }
}

void *std_puppet_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 12> callbacks{};
        std::copy_n(static_cast<void **>(info_node::native_vtable()), callbacks.size(), callbacks.begin());
        callbacks[2] = reinterpret_cast<void *>(&delete_puppet);
        callbacks[3] = reinterpret_cast<void *>(&puppet_type);
        callbacks[4] = reinterpret_cast<void *>(&puppet_subclass);
        callbacks[6] = reinterpret_cast<void *>(&puppet_needs_advance);
        callbacks[7] = reinterpret_cast<void *>(&advance_puppet);
        callbacks[8] = reinterpret_cast<void *>(&activate_puppet);
        callbacks[11] = reinterpret_cast<void *>(&puppet_size);
        return callbacks;
    }();
    return table.data();
}

std_puppet_inode::std_puppet_inode() : info_node(), field_1C(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[315]);
    my_param_block.set_pb_int(string_hash{"puppet_generic_in_service"}, 0, true);
}

std_puppet_inode::std_puppet_inode(from_mash_in_place_constructor *tag) : info_node(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[315]);
    my_param_block.set_pb_int(string_hash{"puppet_generic_in_service"}, 0, true);
}

void std_puppet_inode::unmash(mash_info_struct *info, void *data)
{
    info_node::_unmash(info, data);
}

void std_puppet_inode::activate(ai_core *core)
{
    info_node::_activate(core);
    field_1C = false;
}


void std_puppet_inode::set_current_state(string_hash state)
{
    static const string_hash current_state{static_cast<int>(to_hash("current_state"))};
    my_param_block.set_pb_hash(current_state, state, true);
}


void std_puppet_inode::frame_advance(Float)
{
    const auto *machine = get_core()->my_locomotion_machine;
    if (machine != nullptr && machine->get_curr_state() != nullptr &&
        machine->has_default_transition(static_cast<mash::virtual_types_enum>(316), false)) {
        set_current_state(machine->get_curr_state()->get_name());
        field_1C = false;
    } else {

        if (!field_1C) {
            set_current_state(string_hash{0});
        }
        field_1C = true;
    }
}

}
