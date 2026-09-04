#pragma once

#include "variable.h"
#include "nfl_system.h"

struct nslBankLoad {
    int field_0;
    int field_4;
    nflRequestID rid;
    int field_C;
    nflFileID field_10;
    int field_14;
    struct internal {
        int field_0;
        char field_4;
        char field_5;
        char field_6;
        char field_7;
        int field_8;
        char field_C;
        int field_10;
        int field_14;
        void *field_18;
        int field_1C;
        int field_20;
        int field_24[7];
        int field_40;
        void *field_44;
    } *field_18;
    int field_1C;
    int field_20;
    int field_24;
};

extern int nslBankLoad_Update(nslBankLoad *a1);

extern void nslBankLoad_Init(nslBankLoad *a1, unsigned int a2, nflFileID a3, unsigned int a4);

extern Var<nslBankLoad> nsl_bankLoad;
