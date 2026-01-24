#pragma once

#include <float.hpp>

#include <cstdint>

struct nalBasePose;

namespace nalComp {

struct nalCompSkeleton;

struct nalCompAnim {
    std::intptr_t m_vtbl;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    uint32_t field_2C;
    nalCompSkeleton *field_30;
    int field_34;
    float field_38;
    int field_3C;
    int *field_40;
    int *field_44;
    int field_48;

    void * GetCompPerAnimDataInt(int iCompIx);

    //0x00731E00
    void * GetCompAnimTrackData(int iCompIx);

    auto * GetSkeleton() {
        return this->field_30;
    }

    bool DoesComponentAddToPose(int32_t iCompIx);
};

}

extern void nalCompAnim_patch();
