#pragma once

#include <config.h>

#include "variable.h"

struct txSlotPool {
    struct Slot {
        Slot *field_0;
        Slot *field_4;
        int m_id;
    };

    Slot *field_0;
    Slot field_4;
    Slot field_10;
    int m_size;
    int m_count;
    uint32_t max_value;
};

inline auto &nsl_sourcePool = var<txSlotPool>(0x0097DAAC);
inline auto &nsl_emitterPool = var<txSlotPool>(0x0097DADC);

//0x0079F760
extern int txSlotNew(txSlotPool *a1);

//0x0079F690
extern int txSlotIndex(txSlotPool *a1, int a2);

//0x0079F7D0
extern void txSlotFree(txSlotPool *a1, int a2);

//0x0079F5E0
extern bool txSlotPoolInit(txSlotPool *a1, void *entries, uint32_t count, uint32_t size);

//0x0079F830
extern int txSlotFirst(txSlotPool *a1);

//0x0079F850
extern int txSlotNext(txSlotPool *a1, int a2);

//0x0079FA80
extern int txTime();
