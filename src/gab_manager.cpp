#include "gab_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "trace.h"
#include "variables.h"

VALIDATE_SIZE(gab_database, 0x14);

VALIDATE_SIZE(gab_archetype, 0x18);

VALIDATE_SIZE(gab_expression, 0x18);

#if !STANDALONE_SYSTEM

_std::list<gab_history_entry> *& s_gab_history = var<_std::list<gab_history_entry> *>(0x00968514);

gab_database *& s_gab_database = var<gab_database *>(0x00968510);

#else

_std::list<gab_history_entry> *& s_gab_history = []() -> auto & {
    static _std::list<gab_history_entry> * s_gab_history1 {};
    return s_gab_history1;
}();

gab_database *& s_gab_database = []() -> auto & {
    static gab_database * s_gab_database1 {};
    return s_gab_database1;
}();

#endif

void gab_expression::unmash(mash_info_struct *a1, void *) {
    TRACE("gab_expression::unmash");

    a1->unmash_class_in_place(this->field_0, nullptr);
    a1->unmash_class_in_place(this->field_8, nullptr);
}

void gab_archetype::unmash(mash_info_struct *a1, void *a2) {
    TRACE("gab_archetype::unmash");

    if constexpr (1) {
        a1->unmash_class_in_place(this->field_4, nullptr);
    } else {
        void (__fastcall *func)(void *, void *edx, mash_info_struct *, void *) = CAST(func, 0x005DBCA0);
        func(this, nullptr, a1, a2);
    }
}

gab_database::gab_database(from_mash_in_place_constructor *a2) : field_0(a2) {
    if constexpr (1) {
    } else {
        THISCALL(0x005E0E80, this, a2);
    }
}

void gab_database::destruct_mashed_class() {
    THISCALL(0x005E0860, this);
}

void gab_database::unmash(mash_info_struct *a1, void *a3)
{
    TRACE("gab_database::unmash");

    if constexpr (1) {
        a1->unmash_class_in_place(this->field_0, this);
    } else {
        THISCALL(0x005DF4A0, this, a1, a3);
    }
}

void gab_manager::create_inst() {
    CDECL_CALL(0x005D7E20);
}

void gab_manager::delete_inst() {
    delete s_gab_history;
    s_gab_history = nullptr;
}

void gab_manager::frame_advance(Float a1) {
    CDECL_CALL(0x005D1DA0, a1);
}

void gab_manager::set_gab_database(gab_database *a1) {
    s_gab_database = a1;
}

gab_database *gab_manager::get_gab_database() {
    return s_gab_database;
}
