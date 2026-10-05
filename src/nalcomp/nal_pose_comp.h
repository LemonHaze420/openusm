#pragma once

#include "component_id.h"
#include <nal_skeleton.h>

#include <cstdint>

struct BaseComponent;
struct CharComponentBase;
struct nalMatrix4x4;

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

    nalCompPose &operator=(const nalCompPose &a2);

    void CopyPoseDataNoFree(const void *a2);

    void *_GetComponentPoseData(uint32_t a2);

    //virtual
    void *GetComponentPoseData(uint32_t a2);

    void *_GetComponentPoseData(uint32_t a2) const;

    //virtual
    void *GetComponentPoseData(uint32_t a2) const;

    int _GetPoseDataSize() const;

    //virtual
    //0x00734420
    int GetPoseDataSize() const;

    //virtual
    //0x00734430
    int GetPoseDataAlign() const;

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
    struct ComponentEntry {
        int m_name;
        BaseComponent *m_component;
        uint32_t m_flags;

        //0x00671D5F
        bool hasFlag(uint8_t a2) const
        {
            return ((1 << a2) & this->m_flags) != 0;
        }
    };

    int field_5C;
    int m_boneMatrixCount;
    int m_iNumComponents;
    int m_poseDataAlign;
    int m_poseDataSize;
    ComponentEntry *m_components;

    char *m_pPerSkelDir;
    char *m_pDirectory;

    auto GetNumComponents() const
    {
        return this->m_iNumComponents;
    }

    int GetBoneMatrixCount() const;

    //0x00734500
    void GetBoneMatrices(const nalCompPose *a2, nalMatrix4x4 *a3) const;

    //0x007345A0
    void GetTrajectoryUpdate(const nalComp::nalCompPose *a2, nalPositionOrientation *a3);

    //virtual
    int _VirtualGetBoneMatrixCount() const;

    //0x005FB520
    //virtual
    void _VirtualGetBoneMatrices(const nalBasePose *a1, nalMatrix4x4 *a2) const;

    //0x005FB540
    //virtual
    void _VirtualGetTrajectoryUpdate(const nalBasePose *a2, nalPositionOrientation *a3);

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

    BaseComponent *GetComponent(int iCompIx);

    BaseComponent *GetComponent(int iCompIx) const;

    int GetName(int iCompIx) const;

    void *_GetPerSkelDataFromComponent(ComponentId id) const;
    void *GetPerSkelDataFromComponent(ComponentId id) const;

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
