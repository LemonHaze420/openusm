#pragma once

#include "scene_anim.h"

#include "float.hpp"
#include "nfl_system.h"

struct nalBaseSkeleton;
struct tlFixedString;

struct nalClientSceneAnim;

struct nalStreamInstance : nalSceneAnimInstance {
    int field_1C;
    nalSceneAnimCallback m_callback;
    void *field_24;
    nflFileID field_28;
    nalSceneAnim *field_2C;
    int Size;
    int field_34;
    nalBaseSkeleton **field_38;
    int field_3C;
    int field_40;
    int BufferSize;
    char *field_48[2];
    int field_50[2];
    int field_58;
    int field_5C;
    int CurrentOffset;
    int field_64;
    int field_68;
    nflRequestID m_requestID;
    int field_70;
    int field_74;
    uint8_t field_78;
    uint8_t field_79;

    void *operator new(size_t size);

    void operator delete(void *, size_t);

    void AdvanceStream();

    ~nalStreamInstance();

    bool IsReady() const;

    bool Advance(Float dt);
};

extern nalStreamInstance *create_stream_instance(uint32_t a1, uint32_t a2, uint32_t a3, [[maybe_unused]] int a4,
                                                 nalClientSceneAnim *(*p_cb)(const tlFixedString &, void *), void *a6);
