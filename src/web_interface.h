#pragma once

#include "float.hpp"
#include "mstring.h"
#include "mvector.h"
#include "string_hash.h"

struct actor;
struct entity;

struct web_info_nugget {
    actor *my_actor;
    float field_4;
    float field_8;
    float field_C;
    float field_10;
    entity *m_web_entity;
    string_hash field_18;
    bool field_1C;

    web_info_nugget(mString a2, Float a3, Float a4);

    //0x004ED7D0
    void create_web_entity();

    //0x004ED790
    void destroy_web_entity();

    //0x004ED8C0
    void frame_advance(Float a2);
};

struct web_interface {
    enum eWebInterfaceFlags {
    };

    mVector<web_info_nugget> field_0;
    actor *my_actor;
    int field_18;
    int16_t field_1C;

    web_interface(actor *a2);

    void * operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    bool is_flagged(web_interface::eWebInterfaceFlags a2) const {
        return (a2 & this->field_1C) != 0;
    }

    //0x004D1C60
    void insert_in_web_targets_list();

    void frame_advance(Float a2);

    //0x004CF670
    void set_my_actor(actor *a2);

    //0x004D1C90
    void add_web_nugget(web_info_nugget *nugget);

    //0x004F2AC0
    static void frame_advance_all_web_interfaces(Float a1);

    static inline auto & m_all_web_interfaces = var<mVector<web_interface>>(0x0095B9B4);
};

extern void web_interface_patch();
