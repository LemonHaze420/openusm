#pragma once

#include "actor_interface.h"

#include "float.hpp"
#include "fixedstring.h"
#include "variable.h"

#include <vector.hpp>

#include <cstdint>

struct actor;
struct generic_mash_header;
struct generic_mash_data_ptrs;

struct facial_expression_interface : actor_interface {
    actor *owner_actor;
    fixedstring<8> field_10;
    bool field_30;

    struct {
        int field_0 {-1};
        float field_4[3] {};
        int field_10 {0};
    } field_34[3];
    int field_70;
    int field_74;
    int field_78;

    //0x006D1670
    facial_expression_interface(actor *a1);

    bool is_dynamic() const;

    //0x006C9940
    static void frame_advance_all_facial_expression_ifc(Float a1);

    //0x006CFE50
    void add_to_facial_expression_ifc_list();

    //0x006D1760
    void un_mash(generic_mash_header *, actor *a3, void *, generic_mash_data_ptrs *);

    //virtual
    const char * get_ifc_type_str() const;

    static inline Var<_std::vector<facial_expression_interface> *> all_facial_expression_interfaces{
        0x0096C9AC};
};

extern void facial_expression_interface_patch();
