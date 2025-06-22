#pragma once

#include <cstdint>

struct BaseComponent;

namespace nalComp {

struct nalCompSkeleton;

struct nalCompPose {
    std::intptr_t m_vtbl;
    const nalCompSkeleton *field_4;
    void *m_pTheData;
    int field_C;

    auto GetSkeleton() const {
        return this->field_4;
    }

    nalCompPose(const nalCompSkeleton *);

    nalCompPose & operator=(const nalComp::nalCompPose *a2);

    void CopyPoseDataNoFree(const void *a2);

    void * GetComponentPoseData(uint32_t a2);

    //virtual
    void * GetComponentPoseData(uint32_t a2) const;

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

extern nalCompPose *&pTempStuff;

} // namespace nalComp
