#pragma once

#include <cstdint>

#include <float.hpp>

struct BaseComponent {
    std::intptr_t m_vtbl;
    //virtual ~BaseComponent() = default;

    //virtual
    int GetType() { return 0; }

    //virtual
    void * ApplyPublicPerSkelDataOffset(uint32_t a1, void *a2) const;

    //virtual
    void * ApplyPublicPerAnimDataOffset(uint32_t a1, const void *) const;

    //virtual
    void * GetSizeOfPerInstData(
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool a8);

    //virtual
    void BlendPoseData(void *a1, uint32_t a2, Float a3,const void *a4, const void *a5);

    //virtual
    void SkelPoseProcess(uint32_t a1, void *a2, void *a3) const;

    //virtual
    void PoseDataFree(uint32_t , void *) const;
};

