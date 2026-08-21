#pragma once

#include "component_id.h"
#include <nal_skeleton.h>

#include <cstdint>

struct BaseComponent;
struct CharComponentBase;

namespace nalComp {

struct nalCompSkeleton;

struct nalCompPose {
    std::intptr_t m_vtbl;
    const nalCompSkeleton *field_4;
    void *m_pTheData;
    int field_C;

    auto GetSkeleton() const
    {
        return this->field_4;
    }

    nalCompPose(const nalCompSkeleton *);

    nalCompPose &operator=(const nalComp::nalCompPose *a2);

    void CopyPoseDataNoFree(const void *a2);

    void *_GetComponentPoseData(uint32_t a2);

    //virtual
    void *GetComponentPoseData(uint32_t a2);

    void *_GetComponentPoseData(uint32_t a2) const;

    //virtual
    void *GetComponentPoseData(uint32_t a2) const;

    int _GetPoseDataSize();

    //virtual
    //0x00734420
    int GetPoseDataSize();

    //virtual
    //0x00734430
    int GetPoseDataAlign();

    //virtual
    //0x00731E90
    void AllocPoseData();

    //virtual
    //0x00737800
    void CopyPoseData(void *a2);

    //virtual
    //0x00731EC0
    void DirectCopyPoseData(const void *a2);

    //virtual
    void FreePoseData();

    //virtual
    //0x00737820
    void InitializePoseDataFromSkel();

    //virtual
    void ComponentFreePoseData();
};

struct nalCompSkeleton : nalBaseSkeleton {
    int field_5C;
    int field_60;
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

    auto GetNumComponents() const
    {
        return this->m_iNumComponents;
    }

    void CopyPose(nalCompPose &a1, const nalCompPose &a2);

    //virtual
    void VirtualCopyPose(nalBasePose *a1, const nalBasePose *a2);

    //virtual
    void VirtualBlend(nalBasePose *a1, Float a2, const nalBasePose *a3, const nalBasePose *a4);

    int ConvertCompIxToPoseIx(uint32_t a2) const;

    int GetComponentPoseDataOffset(uint32_t a2) const;

    char *GetCompDefaultPoseData(int iCompIx) const;

    char *GetCompPerSkelDataInt(int iCompIx) const;

    ComponentId GetComponentId(int a3);

    int GetCompIxFromName(nalComp::ComponentId a2) const;

    CharComponentBase *GetComponent(int iCompIx);

    CharComponentBase *GetComponent(int iCompIx) const;

    int GetName(int iCompIx) const;

    bool _DoesComponentHavePoseTrackData(int a2) const;

    //virtual
    //0x00732000
    bool DoesComponentHavePoseTrackData(int a2) const;

    void _UnMash(void *a2, BaseComponent **a3, unsigned int iNumComponents);

    //virtual
    void UnMash(void *a2, BaseComponent **a3, unsigned int iNumComponents);

    void ReMash(void *a2);
};

extern void Blend(nalCompPose &a1, Float a2, const nalCompPose &src0, const nalCompPose &src1);

extern nalComp::nalCompPose *&pTempStuff;

}  // namespace nalComp
