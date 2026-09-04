#pragma once

#include "entity_base_vhandle.h"
#include "fixed_pool.h"
#include "msimpletemplates.h"
#include "opcodes.h"
#include "script_library_class.h"
#include "signaller.h"
#include "vm_stack.h"

#include <variable.h>

#include <vector.hpp>

struct mString;
struct script_instance;
struct vm_executable;

struct vm_thread {
    union argument_t {
        vm_num_t val;
        vm_str_t str;
        short word;
        char *sdr;
        script_library_class::function *lfr;
        vm_executable *sfr;
        unsigned binary;
    };

    struct internal_t {
        string_hash field_0;
        char *field_4;
    };

    enum flags_t {
        SUSPENDED = 0x0001,
        SUSPENDABLE = 0x0002,
    };

    simple_list<vm_thread *>::vars_t simple_list_vars;
    script_instance *inst;
    const vm_executable *ex;
    vm_thread *field_14;
    int field_18;

private:
    int flags;

public:
    vm_stack dstack;
    const uint16_t *PC;
    const uint16_t *field_1B0;
    float field_1B4;
    _std::vector<unsigned short const *> PC_stack;
    _std::vector<internal_t> field_1C8;
    script_library_class::function::entry_t entry;
    void *field_1DC;
    int field_1E0;
    int field_1E4;

    //0x005A5420
    vm_thread(script_instance *a2, const vm_executable *a3);

    //0x005A5500
    vm_thread(script_instance *a2, const vm_executable *a3, void *a4);

    //0x005A55E0
    ~vm_thread();

    void *operator new(size_t size);

    void operator delete(void *);

    const vm_executable *get_running_executable() const;

    script_instance *get_instance()
    {
        return inst;
    }

    const vm_executable *get_executable() const
    {
        return this->ex;
    }

    auto &get_data_stack()
    {
        return this->dstack;
    }

    bool is_flagged(flags_t f) const
    {
        return (f & this->flags) != 0;
    }

    void set_flag(flags_t a2, bool a3);

    void set_suspended(bool a2);

    bool is_suspended() const
    {
        return this->is_flagged(SUSPENDED);
    }

    //0x005996C0
    char *install_temp_string(const char *a1);

    //0x005ADD00
    bool run();

    //0x005A56F0
    void push_PC();

    //0x005A0C10
    void pop_PC();

    void slf_error(const mString &a2);

    //0x00599710
    void raise_event(const vm_thread::argument_t &a2, opcode_arg_t arg_type);

    //0x0058F960
    void raise_all_event(const vm_thread::argument_t &a2, opcode_arg_t arg_type);

    //0x0058F890
    void create_event_callback(const vm_thread::argument_t &a2, bool a3);

    //0x0058F900
    void create_static_event_callback(const vm_thread::argument_t &a2, bool a3);

    //0x005AB670
    void spawn_sub_thread(const vm_thread::argument_t &a2);

    //0x005AB710
    void spawn_parallel_thread(const vm_thread::argument_t &a2);

    //0x0058F7E0
    bool call_script_library_function(const vm_thread::argument_t &a2, const uint16_t *a3);

    static void register_callbacks(void (*a1)(vm_thread *, string_hash, vhandle_type<signaller>, vm_executable *,
                                              char *, bool),
                                   void (*a2)(vm_thread *, string_hash, vhandle_type<signaller>),
                                   void (*a3)(vm_thread *, string_hash), int (*a4)(uint32_t, uint32_t),
                                   void (*a5)(vm_thread *, string_hash));

    static inline auto &string_registers = var<char[64][256]>(0x00961940);

    static Var<fixed_pool> pool;

    static inline auto &add_signal_callback_callback =
        var<void (*)(vm_thread *, string_hash, vhandle_type<signaller>, vm_executable *, char *, bool)>(0x00965F10);

    static inline auto &raise_signal_callback =
        var<void (*)(vm_thread *, string_hash, vhandle_type<signaller>)>(0x00965F14);

    static inline auto &raise_all_signal_callback = var<void (*)(vm_thread *, string_hash)>(0x00965F18);

    static inline Var<int> id_counter{0x00965F0C};
};

extern void vm_thread_patch();
