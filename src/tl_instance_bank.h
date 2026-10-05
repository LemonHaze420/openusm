#pragma once

#include "fixedstring.h"
#include "variable.h"

struct tlHashString;

struct tlInstanceBank {
    struct Node {
        tlFixedString field_0;
        void *field_20;
        int field_24;
        Node *field_28[1];
    };

    Node *field_0;
    Node *field_4;
    int field_8;
    int field_C;
    int field_10;

    //0x00749FF0
    tlInstanceBank();

    //0x0074A410
    ~tlInstanceBank();

    void Release();

    //0x0074A470
    void Init();

    //0x0074A1C0
    int Delete(const tlFixedString &a2);

    //0x0074A070
    Node *Insert(const tlFixedString &a1, void *a3);

    int sub_74A020();

    Node *NewNodeOfLevel(int a1);

    Node *Search(const tlFixedString &a2);

    //0x0074A310
    Node *Search(const tlHashString &a2);
};

extern void tlInstanceBank_patch();
