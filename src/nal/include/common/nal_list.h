#pragma once

#include <fixedstring.h>
#include <variable.h>

#include <tl_instance_bank.h>

struct nalInitList {

    struct vtbl {
        void * Register;
    };

    std::intptr_t m_vtbl;
    void *field_4;

    nalInitList();

    static void * & head;

    /* virtual */ tlInstanceBank::Node *Register() /* = 0 */;
};

struct nalInitListAnimType : nalInitList {
    tlFixedString field_8;
    std::intptr_t anim_vtbl_ptr;
    std::intptr_t skel_vtbl_ptr;

    nalInitListAnimType(const char *str,
                        std::intptr_t anim_vtbl_ptr_arg,
                        std::intptr_t skel_vtbl_ptr_arg);

    /* virtual */ tlInstanceBank::Node * _Register() /* override */;
};

struct nalComponentInitList : nalInitList {
    const char *field_8;
    void *field_C;

    nalComponentInitList(const char *str, void *a3);

    /* virtual */ tlInstanceBank::Node * _Register() /* override */;
};

//0x0078AE10
extern void nalInitListInit();

extern void nalInitList_patch();
