#pragma once

#include "mstring.h"
#include "string_hash.h"
#include "so_data_block.h"
#include "float.hpp"
#include "msimpletemplates_guts.h"

#include <list.hpp>
#include <set.hpp>

#include <set>

#include <cstdio>

struct chunk_file;
struct vm_executable;
struct vm_thread;
struct script_instance;
struct script_executable;
struct generic_mash_header;
struct generic_mash_data_ptrs;

inline constexpr auto SCRIPT_OBJECT_FLAG_GLOBAL = (1u << 0);
inline constexpr auto SCRIPT_OBJECT_FLAG_EXTERNAL = (1u << 2);

struct vm_symbol {
    mString field_0;
    mString field_C;
    mString field_18;
    mString field_24;
    int field_30;
    int field_34;
    bool field_38;

    void read(chunk_file *file);
};

struct script_object {
    string_hash name;
    script_executable *parent;
    script_instance *global_instance;

    struct debug_info_t {
        string_hash field_0;
        _std::list<vm_symbol> field_4;
        _std::list<vm_symbol> field_10;

        ~debug_info_t() {
            void (__fastcall *func)(void *) = CAST(func, 0x005B7BE0);
            func(this);
        }
    };

    debug_info_t *debug_info;
    so_data_block static_data;
    int data_blocksize;
    vm_executable **funcs;
    int total_funcs;
    int field_28;
    simple_list<script_instance> *instances;
    uint32_t flags;

    script_object();

    //0x005AF6C0
    ~script_object();

    bool is_external_object() const {
        return (this->flags & SCRIPT_OBJECT_FLAG_EXTERNAL) != 0;
    }

    bool is_global_object() const {
        return (this->flags & SCRIPT_OBJECT_FLAG_GLOBAL) != 0;
    }

    auto *get_parent() {
        return parent;
    }

    char *get_static_data_buffer() const {
        return this->static_data.get_buffer();
    }

    int get_static_data_size() const {
        return this->static_data.size();
    }

    auto &get_name() const {
        return name;
    }

    int get_constructor_parmsize();

    //0x005A0750
    void constructor_common();

    //0x005AD9A0
    void destructor_common();

    //0x005AF3E0
    void destroy();

    //0x005AF320
    void create_destructor_instances();

    //0x005AF750
    void run(bool a2);

    bool has_threads() const;

    void sub_5AB420();

    void dump_threads_to_file(FILE *a2);

    //0x005AB120
    script_instance *add_instance(string_hash a2, char *a3, vm_thread **a4);

    //0x005AB200
    script_instance *add_instance(string_hash a2, vm_executable *a3);

    //0x005ADC60
    void remove_instance(script_instance *a2);

    script_instance *sub_5AB030(string_hash a2, int a3);

    //0x0059ECC0
    void add(script_instance *a2);

    //0x00599530
    void link(const script_executable *a2);

    //0x005AB350
    void un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5);

    //0x005AAEF0
    void create_auto_instance(Float arg0);

    vm_executable *get_func(int);

    //0x0058EF80
    int find_func(string_hash a2) const;

    static void read(chunk_file *file, script_object *so);

    struct function {
        script_object *field_0;
        string_hash field_4;
        int field_8;
        int field_C;
    };

    static inline Var<function[20]> function_cache {0x00966D10};
};

struct script_instance_callback_reason_t {
    int field_0;
};

struct script_instance {
    simple_list<script_instance>::vars_t simple_list_vars;
    string_hash name;
    so_data_block data;
    simple_list<vm_thread> threads;
    vm_executable *field_28;
    script_object *parent;
    uint32_t flags;
    void (* m_callback)(script_instance_callback_reason_t, script_instance *, vm_thread *, void *);
    _std::set<void *> field_38;

    //0x005AAA40
    script_instance(string_hash a2,
        int size,
        unsigned int a4);

    auto &get_name() const {
        return name;
    }

    auto get_parent()
    {
        return this->parent;
    }

    char *get_buffer()
    {
        return this->data.get_buffer();
    }

    //0x005AAE60
    simple_list<vm_thread>::iterator delete_thread(
        simple_list<vm_thread>::iterator a3);

    void dump_threads_to_file(FILE *a2);

    void run(bool a2);

    //0x0059EC70
    void run_callbacks(
        script_instance_callback_reason_t a2,
        vm_thread *a3);

    //0x005AF500
    void build_parameters();

    bool has_threads() const;

    //0x005AAC20
    vm_thread *add_thread(const vm_executable *a2);

    vm_thread *add_thread(const vm_executable *a1, const char *a2);

    //0x005ADB80
    void massacre_threads(const vm_executable *a2, const vm_thread *a3);

    //0x005AD8D0
    void kill_thread(const vm_executable *a2, const vm_thread *a3);

    //0x005A33F0
    int *register_callback(
        void (*p_cb)(script_instance_callback_reason_t, script_instance *, vm_thread *, void *),
        void *a3);

};

extern void script_instance_patch();
