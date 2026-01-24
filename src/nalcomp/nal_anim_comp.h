#pragma once

#include <nal_skeleton.h>

#include "component_id.h"
#include <float.hpp>

#include <cstdint>

struct BaseComponent;
struct CharComponentBase;
struct nalBasePose;

namespace nalComp {

struct nalCompPose;

extern void Blend(nalCompPose &a1, Float a2, const nalCompPose &src0, const nalCompPose &src1);

struct nalCompSkeleton : nalBaseSkeleton {

    int field_48;
    int field_4C[6];
    int m_iNumComponents;
    int field_68;
    int field_6C;
    struct {
        int m_name;
        BaseComponent *m_component;
        int field_8;

        bool sub_671D5F(char a2) const
        {
            return ((1 << a2) & this->field_8) != 0;
        }
    } *field_70;

    char *field_74;
    char *field_78;

    auto GetNumComponents() const {
        return this->m_iNumComponents;
    }

    void CopyPose(nalCompPose &a1, const nalCompPose &a2);

    //virtual
    void VirtualCopyPose(nalBasePose *a1, const nalBasePose *a2);

    //virtual
    void VirtualBlend(
            nalBasePose *a1,
            Float a2,
            const nalBasePose *a3,
            const nalBasePose *a4);

    int ConvertCompIxToPoseIx(uint32_t a2) const;

    int GetComponentPoseDataOffset(uint32_t a2) const;

    char * GetCompDefaultPoseData(int iCompIx) const;

    char * GetCompPerSkelDataInt(int iCompIx) const;

    ComponentId GetComponentId(int a3);

    int GetCompIxFromName(nalComp::ComponentId a2) const;

    CharComponentBase * GetComponent(int iCompIx);

    CharComponentBase * GetComponent(int iCompIx) const;

    int GetName(int iCompIx) const;

    bool _DoesComponentHavePoseTrackData(int a2) const;

    //virtual
    //0x00732000
    bool DoesComponentHavePoseTrackData(int a2) const;

    void UnMash(void *a2, BaseComponent **a3, unsigned int iNumComponents);

    void ReMash(void *a2);
};

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
    nalComp::nalCompSkeleton *field_30;
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
