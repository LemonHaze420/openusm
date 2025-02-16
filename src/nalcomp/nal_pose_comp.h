#pragma once

#include <cstdint>

struct BaseComponent;

namespace nalComp {

struct nalCompSkeleton;

struct nalCompPose {
    std::intptr_t m_vtbl;
    const nalCompSkeleton *field_4;
    void *field_8;
    int field_C;

    auto GetSkeleton() const {
        return this->field_4;
    }

    nalCompPose(const nalCompSkeleton *);

    void * GetComponentPoseData(uint32_t a2);

    //virtual
    void * GetComponentPoseData(uint32_t a2) const;

    //virtual
    void FreePoseData();

    //virtual
    void ComponentFreePoseData();
};

} // namespace nalComp
