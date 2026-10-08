#pragma once

#include "fixedstring.h"
#include "nal_anim.h"
#include "nal_system.h"

struct nalClientSceneAnim;
struct nalSceneAnimInstance;

using nalSceneAnimCallback = nalClientSceneAnim *(*)(const tlFixedString &, void *);

struct nalSceneAnimSegment {
    nalSceneAnimSegment *next;
    int field_4;
    nalAnimClass<nalAnyPose> *anim;
};

struct nalSceneAnim {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    tlFixedString field_10;
    int field_30;
    nalSceneAnimSegment *field_34;
    int field_38;
    int field_3C;
    tlFileBuf field_40;
    int field_4C;
    tlFixedString field_50;

    static tlFixedString *get_string(nalSceneAnim *a1)
    {
        return &a1->field_10;
    }

    nalSceneAnimInstance *CreateInstance(nalSceneAnimCallback callback, void *parameter);
};

struct nalClientSceneAnim {
    struct vtable {
        nalAnimClass<nalAnyPose>::nalInstanceClass *(__fastcall *CreateInstance)(nalClientSceneAnim *, void *,
                                                                                 nalAnimClass<nalAnyPose> *);
        void(__fastcall *Advance)(nalClientSceneAnim *, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *, Float,
                                  Float, Float, Float);
        void(__fastcall *Render)(nalClientSceneAnim *, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *, Float);
        void(__fastcall *Release)(nalClientSceneAnim *, void *);
    } *m_vtbl;
};

struct nalSceneAnimInstance {
    struct client_anim {
        nalClientSceneAnim *field_0;
        nalAnimClass<nalAnyPose> *field_4;
        nalAnimClass<nalAnyPose>::nalInstanceClass *field_8;
        client_anim *field_C;
    };
    struct vtable {
        void *(__fastcall *Destroy)(nalSceneAnimInstance *, void *, unsigned);
        bool(__fastcall *IsReady)(const nalSceneAnimInstance *, void *);
        bool(__fastcall *Advance)(nalSceneAnimInstance *, void *, Float);
    } *m_vtbl;
    nalSceneAnim *field_4;
    client_anim *field_8;
    client_anim *field_C;
    float field_10;
    float field_14;
    nalSceneAnimSegment *field_18;

    ~nalSceneAnimInstance();
    void Destroy();
    bool Advance(Float dt);
    bool IsReady() const;
    bool IsFinished() const;
    void AddClientAnim(nalClientSceneAnim *client, nalAnimClass<nalAnyPose> *anim);
    void Render() const;
};

struct nalStaticInstance : nalSceneAnimInstance {
    nalSceneAnimSegment *field_1C;

    nalStaticInstance(nalSceneAnim *scene, nalSceneAnimCallback callback, void *parameter);
    bool Advance(Float dt);
};

extern void scene_anim_patch();
