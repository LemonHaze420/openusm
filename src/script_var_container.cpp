#include "script_var_container.h"

#include "func_wrapper.h"
#include "common.h"
#include "parse_generic_mash.h"
#include "script_manager.h"
#include "trace.h"

#include <cassert>

VALIDATE_SIZE(script_var_container, 0x1Cu);

constexpr auto SCRIPT_VAR_FLAG_UN_MASHED = 4u;

void script_var_container::un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4, void *)
{
    assert(( flags & SCRIPT_VAR_FLAG_UN_MASHED ) == 0);
    this->un_mash(a2, a3, a4);
}

void __fastcall sub_599380(
        void *a1,
        void *,
        generic_mash_header *,
        script_var_container *a3,
        void *,
        generic_mash_data_ptrs *a5)
{

    struct {
        uint8_t *field_0;
        int field_4;
    } *self = static_cast<decltype(self)>(a1);

    a5->rebase(4);

    int v6 = *a5->get<int>();

    self->field_0 = a5->get<uint8_t>(v6);

    int addr = ( ((a3->flags & 2) != 0)
            ? (int) script_manager::get_game_var_address(self->field_4)
            : (int) script_manager::get_shared_var_address(self->field_4)
            );

    self->field_4 = addr;
}

void script_var_container::un_mash(generic_mash_header *header, void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("script_var_container::un_mash");

    if constexpr (1) {
        assert(((int)header) % 4 == 0);

        this->script_var_block.un_mash(header, this, a4);

        a4->rebase(4);

        a4->rebase(4);

        struct type {
            char field_0[8];
        };

        this->script_var_to_addr = CAST(this->script_var_to_addr, a4->get<type>(this->field_10));

        a4->rebase(4);

        for ( auto i = 0; i < this->field_10; ++i ) {
            assert(((int)header) % 4 == 0);

            sub_599380(
                this->script_var_to_addr[i],
                nullptr,
                header,
                this,
                this->script_var_to_addr[i],
                a4);
        }

        this->flags |= 4u;
    } else {
        THISCALL(0x00599440, this, header, a3, a4);
    }
}

void script_var_container::destroy()
{
    THISCALL(0x005A3370, this);
}

int script_var_container::save_script_var_buffer(char *a2)
{
    TRACE("script_var_container::save_script_var_buffer");

    if ( a2 != nullptr ) {
        auto v4 = this->script_var_block.size();
        auto *buffer = this->script_var_block.get_buffer();
        memcpy(a2, buffer, v4);
    }

    return this->script_var_block.size();
}

int script_var_container::load_script_var_buffer(char *a2)
{
    TRACE("script_var_container::load_script_var_buffer");

    if ( a2 != nullptr )
    {
        auto v4 = this->script_var_block.size();
        auto *buffer = this->script_var_block.get_buffer();
        memcpy(buffer, a2, v4);
    }

    return this->script_var_block.size();
}

void *script_var_container::get_script_var_address(const char *a2, script_library_class **a3) {
    return (void *) THISCALL(0x005A0520, this, a2, a3);
}

char *script_var_container::get_address(int offset) {
    TRACE("script_var_container::get_address", std::to_string(offset).c_str());

    assert(this->script_var_block.get_buffer() != nullptr);
    assert(script_var_block.is_address_in_buffer( int( script_var_block.get_buffer() ) + offset ));
    return (this->script_var_block.get_buffer() + offset);
}
