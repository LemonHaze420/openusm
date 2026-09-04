#pragma once

struct actor;
struct dangler;
struct entity_base;
struct polytube;

namespace ai {

struct SpidermanLocoSwingBack {
    int field_0;
    dangler *web_dangler;
    int field_8;
    char field_C;

    //0x0045D2C0
    SpidermanLocoSwingBack();

    //0x00438EC0
    ~SpidermanLocoSwingBack();

    //0x00481650
    void init(polytube *a2, actor *a3, entity_base *a4);
};
}  // namespace ai
