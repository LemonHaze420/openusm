#pragma once

#include "float.hpp"
#include "mvector.h"
#include "variable.h"

#include <vector.hpp>

struct interaction;
struct actor;
struct from_mash_in_place_constructor;
struct mash_info_struct;

struct interactable_interface {
    actor *field_0;
    mVector<interaction> field_4;
    bool field_18;

    //0x004ED630
    interactable_interface(actor *act);

    //0x004ED5C0
    interactable_interface(from_mash_in_place_constructor *a2);

    //0x004DAFB0
    static void add_interface_to_list(interactable_interface *who_to_add);

    //0x004CAC60
    void frame_advance(Float a2);

    void update_registrations();

    void sub_4DAE90(actor *a2);

    void unmash(mash_info_struct *a2, void *);

    //0x004D1C10
    static void frame_advance_all(Float a1);

    static Var<_std::vector<interactable_interface *>> all_ifcs;
};

extern void interactable_interface_patch();
