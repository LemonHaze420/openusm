#include "character_anim_inst.h"

#include "character_pose_skel.h"
#include "common.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

#include <cassert>
#include <algorithm>
#include <cmath>

VALIDATE_SIZE(nalChar::nalCharInstance, 0x20u);

static constexpr auto NAL_CHAR_VERSION = 0x10003;

namespace nalChar {
#if !STANDALONE_SYSTEM
int &nalCharAnim::vtbl_ptr = var<int>(0x0096A84C);
#endif
}  // namespace nalChar

void nalChar::nalCharInstance::finalize(bool a2)
{
    this->~nalCharInstance();
    if ((a2 & 1) != 0) {
        ::operator delete(this);
    }
}

nalChar::nalCharInstance::nalCharInstance(nalChar::nalCharAnim *a2, nalChar::nalCharSkeleton *a3)
    : nalCompInstance(a2, a3)
{
    if constexpr (1) {
        static vtbl g_vtbl = {
            func_address(&nalCharInstance::finalize),
            func_address(&nalCharInstance::_VirtualGetPose),
            func_address(&nalCompInstance::_BuildDirectMapping),
            func_address(&nalCompInstance::_BuildSkelRemapping),
            func_address(&nalCompInstance::_BuildEmptyPoseArray),
            func_address(&nalCharInstance::_BuildPerInstData),
        };
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00891FF4;
    }

    this->ConstructInstance();
}

nalChar::nalCharInstance::~nalCharInstance()
{
    TRACE("nalCharInstance::~nalCharInstance");

    this->m_vtbl = 0x00891FF4;

    auto *SkeletonFromInstance = this->GetSkeleton();
    auto *Anim = bit_cast<nalComp::nalCompAnim *>(this->field_10);

    for (int v17 = 0; v17 < this->field_18; ++v17) {
        auto *v5 = &this->field_14[v17];
        if (v5->field_8 != -1 && v5->field_11) {
            auto v6 = v5->field_0;

            auto *ComponentFromInstance = SkeletonFromInstance->GetComponent(v6);
            if (v5->field_10) {
                auto *CompPerSkelDataInt = SkeletonFromInstance->GetCompPerSkelDataInt(v6);
                auto Name = SkeletonFromInstance->GetName(v6);
                auto *CompPerAnimDataInt = (const void *)Anim->GetCompPerAnimDataInt(v5->field_8);
                ComponentFromInstance->DestroyPerInstData(v5->field_C, Name, CompPerSkelDataInt, CompPerAnimDataInt);
            } else {
                auto *Skeleton = Anim->GetSkeleton();
                auto v11 = v5->field_8;
                auto *component = static_cast<CharComponentBase *>(Skeleton->GetComponent(v11));
                auto *CompPerSkelDataInt = SkeletonFromInstance->GetCompPerSkelDataInt(v6);
                auto toName = Skeleton->GetName(v11);
                auto fromName = SkeletonFromInstance->GetName(v6);
                auto *v15 = Anim->GetCompPerAnimDataInt(v11);
                component->DestroyRemapPerInstData(v5->field_C, fromName, toName, component, CompPerSkelDataInt, v15);
            }
        }
    }

    tlMemFree(this->field_14);
    this->field_14 = nullptr;

    tlMemFree(this->field_1C);
    this->field_1C = nullptr;

    this->field_18 = 0;
}

void nalChar::nalCharInstance::_VirtualGetPose(Float a1, Float a2, nalBasePose *a3, const nalBasePose *a4)
{
    TRACE("nalChar::nalCharInstance::VirtualGetPose");

    const nalCharPose *v5 = nullptr;
    if (a4 != nullptr) {
        v5 = (const nalCharPose *)&a4[-1];
    }

    nalCharPose *v6 = nullptr;
    if (a3 != nullptr) {
        v6 = (nalCharPose *)&a3[-1];
    }

    this->GetPose(a1, a2, v6, v5);
}

void nalChar::nalCharInstance::_BuildPerInstData()
{
    TRACE("nalCharInstance::BuildPerInstData");

    if constexpr (1) {
        auto dwSize = 0;
        auto a2 = 1;

        auto func = [](int a1, int a2) -> int {
            return ~(a2 - 1) & (a1 + a2 - 1);
        };

        std::vector<int> v77(this->field_18);

        auto *Skeleton = this->GetSkeleton();
        auto *Anim = this->GetAnim();

        for (int i = 0; i < this->field_18; ++i) {
            auto *v73 = &this->field_14[i];
            auto *v72 = Skeleton->GetComponent(v73->field_0);
            if (v73->field_11 && (!v72->GetDomain() || v73->field_10)) {
                auto v54 = !v73->field_10;
                auto v48 = Anim->GetCompAnimTrackData(v73->field_8);
                auto CompPerAnimDataInt = Anim->GetCompPerAnimDataInt(v73->field_8);
                auto v33 = Skeleton->GetCompDefaultPoseData(v73->field_0);
                auto v24 = v73->field_8;
                auto *v2 = Anim->GetSkeleton();
                auto v25 = v2->GetCompPerSkelDataInt(v24);
                auto v21 = Skeleton->GetCompPerSkelDataInt(v73->field_0);
                auto Name = Skeleton->GetName(v73->field_0);
                auto v71 = v72->GetSizeOfPerInstData(Name, v21, v25, v33, CompPerAnimDataInt, v48, v54);

                auto v55 = !v73->field_10;
                auto v49 = Anim->GetCompAnimTrackData(v73->field_8);
                auto v43 = Anim->GetCompPerAnimDataInt(v73->field_8);
                auto v34 = Skeleton->GetCompDefaultPoseData(v73->field_0);
                auto v26 = v73->field_8;
                auto *v5 = Anim->GetSkeleton();
                auto v27 = v5->GetCompPerSkelDataInt(v26);
                auto v22 = Skeleton->GetCompPerSkelDataInt(v73->field_0);
                auto v6 = Skeleton->GetName(v73->field_0);
                auto v70 = v72->GetAlignOfPerInstData(v6, v22, v27, v34, v43, v49, v55);
                if (!dwSize && v71) {
                    a2 = v70;
                }

                if (v71) {
                    auto v7 = func(dwSize, v70);
                    v77[i] = v7;
                    dwSize = v71 + v7;
                } else {
                    v77[i] = -1;
                }
            } else if (v73->field_11) {
                auto *v69 = Anim->GetSkeleton();
                auto *v68 = v69->GetComponent(v73->field_8);
                auto v56 = Anim->GetCompAnimTrackData(v73->field_8);
                auto v50 = Anim->GetCompPerAnimDataInt(v73->field_8);
                auto v44 = Skeleton->GetCompDefaultPoseData(v73->field_0);
                auto v35 = v73->field_8;
                auto *v8 = Anim->GetSkeleton();
                auto v36 = v8->GetCompPerSkelDataInt(v35);
                auto v28 = Skeleton->GetCompPerSkelDataInt(v73->field_0);
                auto v18 = v69->GetName(v73->field_8);
                auto v16 = Skeleton->GetName(v73->field_0);
                auto v67 = v72->GetRemapSizeOfPerInstData(v16, v18, v68, v28, v36, v44, v50, v56);

                auto v57 = Anim->GetCompAnimTrackData(v73->field_8);
                auto v51 = Anim->GetCompPerAnimDataInt(v73->field_8);
                auto v45 = Skeleton->GetCompDefaultPoseData(v73->field_0);
                auto v37 = v73->field_8;
                auto v9 = Anim->GetSkeleton();
                auto v38 = v9->GetCompPerSkelDataInt(v37);
                auto v29 = Skeleton->GetCompPerSkelDataInt(v73->field_0);
                auto v19 = v69->GetName(v73->field_8);
                auto v17 = Skeleton->GetName(v73->field_0);
                auto v66 = v72->GetRemapAlignOfPerInstData(v17, v19, v68, v29, v38, v45, v51, v57);
                if (!dwSize && v67) {
                    a2 = v66;
                }

                if (v67) {
                    auto v10 = func(dwSize, v66);
                    v77[i] = v10;
                    dwSize = v67 + v10;
                } else {
                    v77[i] = -1;
                }
            } else {
                v77[i] = -1;
            }
        }

        this->field_1C = tlMemAlloc(dwSize, a2, 0);
        auto *v65 = (char *)this->field_1C;
        for (int j = 0; j < this->field_18; ++j) {
            if (v77[j] == -1) {
                this->field_14[j].field_C = nullptr;
            } else {
                auto *v63 = &this->field_14[j];
                auto *component = Skeleton->GetComponent(v63->field_0);
                this->field_14[j].field_C = &v65[v77[j]];
                if (v63->field_10) {
                    auto bIsRemapped = !v63->field_10;
                    auto *v52 = Anim->GetCompAnimTrackData(v63->field_8);
                    auto *v46 = Anim->GetCompPerAnimDataInt(v63->field_8);
                    auto *compDefaultPoseData = Skeleton->GetCompDefaultPoseData(v63->field_0);
                    auto *skeletonFromAnim = Anim->GetSkeleton();
                    auto *compPerSkelDataIntFromAnim = skeletonFromAnim->GetCompPerSkelDataInt(v63->field_8);
                    auto *compPerSkelDataInt = Skeleton->GetCompPerSkelDataInt(v63->field_0);
                    auto name = Skeleton->GetName(v63->field_0);

                    component->BuildPerInstData(v63->field_C,
                                                name,
                                                compPerSkelDataInt,
                                                compPerSkelDataIntFromAnim,
                                                compDefaultPoseData,
                                                v46,
                                                v52,
                                                bIsRemapped);
                } else {
                    auto *skeletonFromAnim = Anim->GetSkeleton();
                    auto *v60 = skeletonFromAnim->GetComponent(v63->field_8);
                    auto *v59 = Anim->GetCompAnimTrackData(v63->field_8);
                    auto *v53 = Anim->GetCompPerAnimDataInt(v63->field_8);
                    auto *v47 = Skeleton->GetCompDefaultPoseData(v63->field_0);
                    auto *v41 = skeletonFromAnim->GetCompPerSkelDataInt(v63->field_8);
                    auto *v32 = Skeleton->GetCompPerSkelDataInt(v63->field_0);
                    auto nameFromAnim = skeletonFromAnim->GetName(v63->field_8);
                    auto name = Skeleton->GetName(v63->field_0);
                    component->BuildRemapPerInstData(v63->field_C, name, nameFromAnim, v60, v32, v41, v47, v53, v59);
                }
            }
        }

    } else {
        void(__fastcall * func)(void *) = CAST(func, 0x005F08A0);
        func(this);
    }
}

nalChar::nalCharSkeleton *nalChar::nalCharInstance::GetSkeleton()
{
    return bit_cast<nalChar::nalCharSkeleton *>(this->field_C);
}

nalChar::nalCharAnim *nalChar::nalCharInstance::GetAnim()
{
    return bit_cast<nalChar::nalCharAnim *>(nalCompInstance::GetAnim());
}

void nalChar::nalCharInstance::GetPose(Float a2, Float a3, nalChar::nalCharPose *a4, const nalChar::nalCharPose *a5)
{
    TRACE("nalChar::nalCharInstance::GetPose");

    if constexpr (1) {
        if ((GetAnim()->field_34 & 1) == 0) {
            a2 = std::clamp(a2.value, 0.0f, 1.0f);
            a3 = std::clamp(a3.value, 0.0f, 1.0f);
        }
        *a4 = *a5;
        for (int i = 0; i < this->field_18; ++i) {
            auto *v30 = &this->field_14[i];
            if (v30->field_8 != -1 && v30->field_11) {
                auto v26 = v30->field_0;
                auto *v5 = this->GetSkeleton();
                auto *v29 = v5->GetComponent(v26);
                if (v30->field_10) {
                    auto v24 = v30->field_8;
                    auto *v7 = this->GetAnim();
                    auto animTrackData = v7->GetCompAnimTrackData(v24);
                    auto v22 = v30->field_8;
                    auto animDataInt = v7->GetCompPerAnimDataInt(v22);
                    auto v21 = v30->field_0;
                    auto *v8 = this->GetSkeleton();
                    auto skelDataInt = v8->GetCompPerSkelDataInt(v21);
                    auto v19 = v30->field_0;
                    auto *v9 = this->GetSkeleton();
                    auto v20 = v9->GetName(v19);
                    auto v10 = a4->GetComponentPoseData(v30->field_0);

                    v29->CalcPoseDataDirect(v10,
                                            v20,
                                            a2,
                                            a3,
                                            v7,
                                            skelDataInt,
                                            bit_cast<void *>(animDataInt),
                                            bit_cast<void *>(animTrackData),
                                            v30->field_C);
                } else {
                    auto v27 = v30->field_8;
                    auto anim = this->GetAnim();
                    auto *v12 = anim->GetSkeleton();
                    auto componentId = v12->GetComponentId(v27);
                    auto v25 = v30->field_8;
                    auto animTrackData = anim->GetCompAnimTrackData(v25);
                    auto v23 = v30->field_8;
                    auto animDataInt = anim->GetCompPerAnimDataInt(v23);

                    auto defaultPoseData = v12->GetCompDefaultPoseData(v30->field_8);
                    auto skelDataInt = v12->GetCompPerSkelDataInt(v30->field_8);
                    auto v20 = v30->field_0;
                    auto *Skeleton = this->GetSkeleton();
                    auto skelDataInt1 = Skeleton->GetCompPerSkelDataInt(v20);
                    auto v18 = v30->field_0;
                    auto v16 = this->GetSkeleton();
                    auto v19 = v16->GetName(v18);
                    auto v17 = a4->GetComponentPoseData(v30->field_0);
                    v29->CalcPoseDataRemapped(v17,
                                              v19,
                                              a2,
                                              a3,
                                              anim,
                                              skelDataInt1,
                                              componentId.field_0,
                                              componentId.field_4,
                                              skelDataInt,
                                              defaultPoseData,
                                              bit_cast<void *>(animDataInt),
                                              bit_cast<void *>(animTrackData),
                                              v30->field_C);
                }
            }
        }
    } else {
        THISCALL(0x005F0E10, this, a2, a3, a4, a5);
    }
}

nalChar::nalCharAnim::nalCharAnim()
{
    if constexpr (1) {
        static void *g_vtbl[]{
            nullptr,
            func_address(&nalCharAnim::_Process),
            func_address(&nalCharAnim::_Release),
            func_address(&nalCharAnim::_CheckVersion),
            func_address(&nalCharAnim::_VirtualCreateInstance),
            func_address(&nalCompAnim::_GetPerAnimDataFromComponentIx),
            func_address(&nalCompAnim::_GetPerAnimUserDataInt),
            func_address(&nalCompAnim::_UnMash),
            func_address(&nalCompAnim::_ReMash),
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00891FD0;
    }
}

void nalChar::nalCharAnim::_Process()
{
    TRACE("nalCharAnim::Process");

    assert(Version == NAL_CHAR_VERSION && "Panel animation version mismatch, must be reconverted");

    this->UnMash(this);
}

void nalChar::nalCharAnim::_Release()
{
    this->ReMash(this);
}

bool nalChar::nalCharAnim::_CheckVersion() const
{
    TRACE("nalCharAnim::CheckVersion");

    return this->Version == NAL_CHAR_VERSION;
}

nalChar::nalCharInstance *nalChar::nalCharAnim::CreateInstance(nalChar::nalCharSkeleton *a2)
{
    auto *result = new nalCharInstance(this, a2);
    return result;
}

nalComp::nalCompInstance *nalChar::nalCharAnim::_VirtualCreateInstance(nalBaseSkeleton *a1)
{
    TRACE("nalCharAnim::VirtualCreateInstance");

    return this->CreateInstance(bit_cast<nalCharSkeleton *>(a1));
}

void *nalChar::nalCharAnim::GetPerAnimDataByName(CharComponentBase::Names a2)
{
    TRACE("nalCharAnim::GetPerAnimDataByName");

    auto *v3 = this->GetSkeleton();
    int CompIxByName = v3->GetCompIxByName(a2);
    if (CompIxByName == -1) {
        return nullptr;
    }

    auto *CompPerAnimDataInt = (const void *)this->GetCompPerAnimDataInt(CompIxByName);
    auto *v9 = this->field_30->GetComponent(CompIxByName);
    return v9->ApplyPublicPerAnimDataOffset(a2, CompPerAnimDataInt);
}

void nalChar::nalCharAnim::ComputeFrameValues(float &a2, uint32_t &a3, uint32_t &a4, float &a5, Float a6) const
{
    TRACE("nalChar::nalCharAnim::ComputeFrameValues");

    if constexpr (1) {
        auto v7 = a6 * this->field_38;

        a2 = v7;
        auto a2a = this->field_34 & 1;
        float v9{};
        bool v10{};
        if (a2a) {
            v9 = this->field_50;
            v10 = this->field_50 < 0;
        } else {
            auto v16 = this->field_50 - 1;
            v9 = v16;
            v10 = v16 < 0;
        }

        if (v10) {
            v9 += flt_86F860;
        }

        auto v17 = v9 / this->field_38;
        if (equal<float>(a6, 1.0f) && equal<float>(this->field_38, 0.0f)) {
            a3 = 0;
            a4 = 0;
            a5 = 0.0f;
            a2 = 0.0f;
            return;
        }

        a3 = std::ceil(v7 * v17);
        auto v12 = v17 * a2;
        int a3a = v12;
        float v13 = a3a;
        if (a3a < 0) {
            v13 += flt_86F860;
        }

        a5 = v12 - v13;
        if (equal<int>(std::ceil(v17 * a2), int(v17 * a2))) {
            ++a3;
            a5 = 0.0f;
        }

        if (a6 >= 1.0f && !a2a) {
            a5 = 1.0f;
        }

        if (a2a) {
            a3 %= this->GetTotalFrames();
        } else {
            uint32_t v14 = this->GetTotalFrames() - 1;
            a3 = std::min(a3, v14);
        }

        auto v15 = a3;
        if (v15) {
            a4 = v15 - 1;
        } else if (a2a) {
            a4 = this->GetTotalFrames() - 1;
        } else {
            assert(0 && "Somehow, a Character Animation has a curr frame of 0 and is not looping.");
            a4 = 0;
        }
    } else {
        void(__fastcall * func)(const void *, void *edx, float *, uint32_t *, uint32_t *, float *, Float) =
            CAST(func, 0x005F06B0);
        func(this, nullptr, &a2, &a3, &a4, &a5, a6);
    }
}

void nalCharInstance_patch()
{
    {
        FUNC_ADDRESS(address, &nalChar::nalCharAnim::_VirtualCreateInstance);
        set_vfunc(0x00891FE0, address);
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharAnim::GetPerAnimDataByName);
        SET_JUMP(0x005F0840, address);
    }
}
