#pragma once

#include "variable.h"

enum nslBankID {
    NSL_BANK_ID_INVALID = -1,
};

struct nslBank {
    nslBankID field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    struct {
        char field_0[0x18];
        char field_20[4];
        int field_24[25];
        int field_88;
    } *field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
};

extern nslBank (&nsl_banks)[1];

inline auto &nsl_workGuard = var<char *>(0x0097DB18);
inline auto &nsl_workGuard1 = var<char *>(0x0097DB1C);

//0x00798550
extern void nslFreeBank(nslBankID a1);

//0x007984D0
extern int nslGetBankState(nslBankID);

extern void nslAramFree(unsigned int a1);

extern void nslUpdate();

extern float *nsl_GetMaster();

extern float *nsl_GetListener();

extern void nsl_patch();
