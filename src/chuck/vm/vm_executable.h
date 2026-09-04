#pragma once

#include "mstring.h"
#include "string_hash.h"

#include <list.hpp>

#include <set>

struct chunk_file;

struct script_executable;
struct script_object;

struct generic_mash_header;
struct generic_mash_data_ptrs;

inline constexpr auto VM_EXECUTABLE_FLAG_STATIC = 1u;
inline constexpr auto VM_EXECUTABLE_FLAG_LINKED = 2u;
inline constexpr auto VM_EXECUTABLE_FLAG_FROM_MASH = 4u;
inline constexpr auto VM_EXECUTABLE_FLAG_UN_MASHED = 8u;

struct vm_executable_param_symbol {};

struct vm_executable {
    script_object *owner;
    string_hash fullname;
    string_hash name;
    int parms_stacksize;
    uint16_t *buffer;
    int buffer_len;
    struct debug_info_t {
        mString field_0;
        int field_10;
        _std::list<vm_executable_param_symbol> field_14;
        void **parameters;
        int field_24;

        debug_info_t()
        {
            field_10 = -1;
            parameters = nullptr;
            field_24 = 0;
        }

    } *debug_info;
    uint32_t flags;
    int field_20;

    vm_executable(script_object *so);

    //0x005AF310
    ~vm_executable();

    void *operator new(size_t);

    void operator delete(void *, size_t);

    void destroy();

    int get_size() const
    {
        return buffer_len;
    }

    auto &get_name() const
    {
        return name;
    }

    auto &get_fullname() const
    {
        return fullname;
    }

    const auto *get_start() const
    {
        return buffer;
    }

    bool is_static() const
    {
        return (this->flags & VM_EXECUTABLE_FLAG_STATIC) != 0;
    }

    bool is_linked() const
    {
        return (this->flags & VM_EXECUTABLE_FLAG_LINKED) != 0;
    }

    bool is_from_mash() const
    {
        return (this->flags & VM_EXECUTABLE_FLAG_FROM_MASH) != 0;
    }

    bool is_un_mashed() const
    {
        return (this->flags & VM_EXECUTABLE_FLAG_UN_MASHED) != 0;
    }

    int get_parms_stacksize() const
    {
        return this->parms_stacksize;
    }

    auto *get_owner() const
    {
        return this->owner;
    }

    void un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5);

    void link(const script_executable &a2);

    void link_un_mash(const script_executable &a2);

    static void write(chunk_file *file, const vm_executable *x, const std::set<string_hash> &);

    static void read(chunk_file *file, vm_executable *x);

    static inline auto &resolve_signal_callback = var<void (*)(const char *, uint32_t *)>(0x00965F08);
};

extern void vm_executable_patch();
