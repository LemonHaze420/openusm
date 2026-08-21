#include "nal_instance.h"

#include "common.h"
#include "func_wrapper.h"
#include "nal_anim_comp.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <vector.hpp>

VALIDATE_SIZE(nalComp::nalCompInstance, 0x20u);

nalComp::nalCompInstance::nalCompInstance(nalComp::nalCompAnim *a2, nalComp::nalCompSkeleton *a3)
    : nalBaseInstance(bit_cast<nalAnimClass<nalAnyPose> *>(a2), a3)
{
    if constexpr (1) {
        static vtbl g_vtbl = {
            func_address(&finalize),
            func_address(&_VirtualGetPose),
            func_address(&_BuildDirectMapping),
            func_address(&_BuildSkelRemapping),
            func_address(&_BuildEmptyPoseArray),
            func_address(&_BuildPerInstData),
        };
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x008AA370;
    }
    this->field_14 = nullptr;
    this->field_18 = 0;
    this->field_1C = nullptr;
}

void nalComp::nalCompInstance::ConstructInstance()
{
    TRACE("nalComp::nalCompInstance::ConstructInstance");

    auto *Anim = this->GetAnim();
    auto *Skeleton = Anim->GetSkeleton();
    if (Skeleton == this->GetSkeleton()) {
        this->BuildDirectMapping();
    } else {
        this->BuildSkelRemapping();
    }

    this->BuildPerInstData();
}

nalComp::nalCompInstance::~nalCompInstance()
{
    this->m_vtbl = 0x008AA370;

    auto *v2 = this->GetAnim();
    auto *SkeletonFromInstance = this->GetSkeleton();

    for (int i = 0; i < this->field_18; ++i) {
        auto *v5 = &this->field_14[i];
        if (v5->field_8 != -1 && v5->field_11) {
            auto *component = SkeletonFromInstance->GetComponent(v5->field_0);
            if (v5->field_10) {
                auto *CompPerAnimDataInt = v2->GetCompPerAnimDataInt(v5->field_8);
                auto *CompPerSkelDataInt = SkeletonFromInstance->GetCompPerSkelDataInt(v5->field_0);
                auto Name = SkeletonFromInstance->GetName(v5->field_0);
                component->DestroyPerInstData(v5->field_C, Name, CompPerSkelDataInt, CompPerAnimDataInt);
            } else {
                auto *Skeleton = v2->GetSkeleton();
                auto v23 = Skeleton->GetComponentId(v5->field_8);
                auto *v14 = v2->GetCompPerAnimDataInt(v5->field_8);
                auto *v12 = SkeletonFromInstance->GetCompPerSkelDataInt(v5->field_0);
                component->DestroyPerInstData(v5->field_C, v23.field_0, v12, v14);
            }
        }
    }

    tlMemFree(this->field_14);
    this->field_14 = nullptr;

    tlMemFree(this->field_1C);
    this->field_1C = nullptr;
}

nalComp::nalCompSkeleton *nalComp::nalCompInstance::GetSkeleton()
{
    return bit_cast<nalCompSkeleton *>(this->field_C);
}

nalComp::nalCompAnim *nalComp::nalCompInstance::GetAnim()
{
    return bit_cast<nalCompAnim *>(this->field_10);
}

void nalComp::nalCompInstance::_VirtualGetPose(Float a1, Float a2, nalBasePose *a3, const nalBasePose *a4)
{
    TRACE("nalComp::nalCompInstance::VirtualGetPose");

    const nalComp::nalCompPose *v5 = nullptr;
    if (a4 != nullptr) {
        v5 = (const nalComp::nalCompPose *)&a4[-1];
    }

    if (a3 != nullptr) {
        this->GetPose(a1, a2, (nalComp::nalCompPose *)&a3[-1], v5);
    } else {
        this->GetPose(a1, a2, nullptr, v5);
    }
}

void nalComp::nalCompInstance::_BuildDirectMapping()
{
    TRACE("nalCompInstance::BuildDirectMapping");

    if constexpr (1) {
        auto *Skeleton = this->GetSkeleton();
        auto NumComponents = Skeleton->GetNumComponents();
        this->field_18 = 0;
        for (int i = 0; i < NumComponents; ++i) {
            if (Skeleton->DoesComponentHavePoseTrackData(i)) {
                ++this->field_18;
            }
        }

        this->field_14 = CAST(this->field_14, tlMemAlloc(20 * this->field_18, 8, 0));
        int v7 = 0;
        for (int iCompIx = 0; iCompIx < NumComponents; ++iCompIx) {
            if (Skeleton->DoesComponentHavePoseTrackData(iCompIx)) {
                this->field_14[v7].field_8 = iCompIx;
                this->field_14[v7].field_4 = Skeleton->ConvertCompIxToPoseIx(iCompIx);
                this->field_14[v7].field_0 = iCompIx;
                this->field_14[v7].field_10 = 1;

                auto *Anim = this->GetAnim();
                this->field_14[v7].field_11 = Anim->DoesComponentAddToPose(iCompIx);
                ++v7;
            }
        }

        this->BuildEmptyPoseArray();
    } else {
        void(__fastcall * func)(void *) = CAST(func, 0x00736F70);
        func(this);
    }
}

void nalComp::nalCompInstance::BuildDirectMapping()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this);
}

void nalComp::nalCompInstance::_BuildSkelRemapping()
{
    TRACE("nalComp::nalCompInstance::BuildSkelRemapping");

    if constexpr (0) {
        _std::vector<nalComp::nalCompInstance::Internal> v81{};
        auto numComponents = this->GetSkeleton()->GetNumComponents();

        for (int i = 0; i < numComponents; ++i) {
            if (this->GetSkeleton()->DoesComponentHavePoseTrackData(i)) {
                auto *Skeleton = this->GetSkeleton();

                Internal v78;
                v78.field_0 = i;
                v78.field_4 = Skeleton->ConvertCompIxToPoseIx(i);
                v78.field_8 = -1;
                v78.field_10 = false;
                v78.field_11 = false;

                v81.push_back(v78);
            }
        }

        _std::vector<int> v78{};

        auto *v66 = this->GetAnim()->GetSkeleton();
        for (int i = 0; i < v81.size(); ++i) {
            auto &v9 = v81[i];
            auto *v5 = this->GetSkeleton();
            auto v12 = v5->GetComponentId(v9.field_0);
            auto v14 = v66->GetCompIxFromName(v12);
            if (v14 != -1) {
                v9.field_10 = true;
                v9.field_8 = v14;

                v9.field_11 = this->GetAnim()->DoesComponentAddToPose(v14);
                v78.push_back(v14);
            }
        }

        for (int j = 0; j < v81.size(); ++j) {
            auto &item = v81[j];
            auto *v73 = this->GetSkeleton()->GetComponent(item.field_0);
            if (!item.field_10) {
                for (int m = 0; m < v66->GetNumComponents(); ++m) {
                    if (this->GetAnim()->DoesComponentAddToPose(m)) {
                        int n;
                        for (n = 0; n < v78.size(); ++n) {
                            if (m == v78[n]) {
                                break;
                            }
                        }

                        if (n < v78.size())
                            break;

                        auto v54 = v66->GetComponent(m)->GetType();
                        auto v51 = v66->GetName(m);

                        auto v52 = this->GetSkeleton()->GetName(item.field_0);
                        if (v73->WillMapToComponentData(v52, v51, v54)) {
                            v78.push_back(m);

                            item.field_8 = m;
                            item.field_11 = true;
                        }
                    }
                }
            }
        }

        for (auto it = v81.begin(); it != v81.end();) {
            if (it->field_8 == -1) {
                ++it;
                //v81->(iter.m_ptr);
            } else {
                ++it;
            }
        }

        this->field_18 = v81.size();
        this->field_14 = static_cast<nalComp::nalCompInstance::Internal *>(tlMemAlloc(20 * this->field_18, 8u, 0));

        for (int v63 = 0; v63 < this->field_18; ++v63) {
            auto &v7 = v81[v63];
            auto &v65 = this->field_14[v63];
            std::memcpy(&v7, &v65, sizeof(nalComp::nalCompInstance::Internal));
        }

        this->BuildEmptyPoseArray();
    } else {
        void(__fastcall * func)(void *) = CAST(func, 0x0073EB50);
        func(this);
    }
}

void nalComp::nalCompInstance::BuildSkelRemapping()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    func(this);
}

void nalComp::nalCompInstance::_BuildEmptyPoseArray()
{
    TRACE("nalCompInstance::BuildEmptyPoseArray");

    _std::vector<int> v4{};
    for (int i = 0; i < this->field_18; ++i) {
        auto v2 = this->field_14[i].field_8;
        auto *Anim = this->GetAnim();
        if (!Anim->DoesComponentAddToPose(v2))
            v4.push_back(this->field_14[i].field_4);
    }
}

void nalComp::nalCompInstance::BuildEmptyPoseArray()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x10));
    func(this);
}

void nalComp::nalCompInstance::_BuildPerInstData()
{
    TRACE("nalCompInstance::BuildPerInstData");

    if constexpr (0) {
        auto dwSize = 0;
        auto a2 = 1;
        int *v56 = new int[this->field_18];
        auto *Skeleton = this->GetSkeleton();
        auto *Anim = this->GetAnim();
        for (int i = 0; i < this->field_18; ++i) {
            auto *v52 = &this->field_14[i];
            if (v52->field_11) {
                auto *Component_0 = Skeleton->GetComponent(v52->field_0);
                auto v41 = !v52->field_10;
                auto CompAnimTrackData = Anim->GetCompAnimTrackData(v52->field_8);
                auto CompPerAnimDataInt = Anim->GetCompPerAnimDataInt(v52->field_8);
                auto CompDefaultPoseData = Skeleton->GetCompDefaultPoseData(v52->field_0);
                auto v23 = v52->field_8;
                auto v2 = Anim->GetSkeleton();
                auto CompPerSkelDataInt = v2->GetCompPerSkelDataInt(v23);
                auto v19 = Skeleton->GetCompPerSkelDataInt(v52->field_0);
                auto Name = Skeleton->GetName(v52->field_0);
                int v51 = Component_0->GetSizeOfPerInstData(
                    Name, v19, CompPerSkelDataInt, CompDefaultPoseData, CompPerAnimDataInt, CompAnimTrackData, v41);
                auto v46 = Skeleton->GetComponent(v52->field_0);

                auto v42 = !v52->field_10;
                auto v38 = Anim->GetCompAnimTrackData(v52->field_8);
                auto v34 = Anim->GetCompPerAnimDataInt(v52->field_8);
                auto v30 = Skeleton->GetCompDefaultPoseData(v52->field_0);
                auto v25 = v52->field_8;
                auto v5 = Anim->GetSkeleton();
                auto v26 = v5->GetCompPerSkelDataInt(v25);
                auto v20 = Skeleton->GetCompPerSkelDataInt(v52->field_0);
                auto v6 = Skeleton->GetName(v52->field_0);
                int v50 = int(v46->GetAlignOfPerInstData(v6, v20, v26, v30, v34, v38, v42));
                if (!dwSize && v51) {
                    a2 = v50;
                }

                if (v51) {
                    auto func = [](int a1, int a2) -> int {
                        return ~(a2 - 1) & (a1 + a2 - 1);
                    };

                    auto v7 = func(dwSize, v50);
                    v56[i] = v7;
                    dwSize = v51 + v7;
                } else {
                    v56[i] = -1;
                }
            } else {
                v56[i] = -1;
            }
        }

        this->field_1C = CAST(this->field_1C, tlMemAlloc(dwSize, a2, 0));
        auto *v49 = (char *)this->field_1C;
        for (int j = 0; j < this->field_18; ++j) {
            if (v56[j] == -1) {
                this->field_14[j].field_C = nullptr;
            } else {
                this->field_14[j].field_C = &v49[v56[j]];
                auto v43 = this->field_14[j].field_0;
                auto v8 = this->GetSkeleton();
                auto v47 = v8->GetComponent(v43);
                auto v44 = !this->field_14[j].field_10;
                auto v39 = this->field_14[j].field_8;
                auto v10 = this->GetAnim();
                auto v40 = v10->GetCompAnimTrackData(v39);
                auto v35 = this->field_14[j].field_8;
                auto v11 = this->GetAnim();
                auto v36 = v11->GetCompPerAnimDataInt(v35);
                auto v31 = this->field_14[j].field_0;
                auto v12 = this->GetSkeleton();
                auto v32 = v12->GetCompDefaultPoseData(v31);
                auto v27 = this->field_14[j].field_8;
                auto v13 = this->GetAnim();
                auto v14 = v13->GetSkeleton();
                auto v28 = v14->GetCompPerSkelDataInt(v27);
                auto v21 = this->field_14[j].field_0;
                auto v15 = this->GetSkeleton();
                auto v22 = v15->GetCompPerSkelDataInt(v21);
                auto v17 = this->field_14[j].field_0;
                auto *v16 = this->GetSkeleton();
                auto v18 = v16->GetName(v17);
                v47->BuildPerInstData(this->field_14[j].field_C, v18, v22, v28, v32, v36, v40, v44);
            }
        }

        delete[] (v56);
    } else {
        void(__fastcall * func)(void *) = CAST(func, 0x00737130);
        func(this);
    }
}

void nalComp::nalCompInstance::BuildPerInstData()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x14));
    func(this);
}


void nalComp::nalCompInstance::GetPose(Float a2, Float a3, nalComp::nalCompPose *a4, const nalComp::nalCompPose *a5)
{
    TRACE("nalComp::nalCompInstance::GetPose");

    if constexpr (1) {
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
                    [[maybe_unused]] auto v28 = v12->GetComponentId(v27);
                    auto v25 = v30->field_8;
                    auto animTrackData = anim->GetCompAnimTrackData(v25);
                    auto v23 = v30->field_8;
                    auto animDataInt = anim->GetCompPerAnimDataInt(v23);
                    auto v20 = v30->field_0;
                    auto *Skeleton = anim->GetSkeleton();
                    auto skelDataInt = Skeleton->GetCompPerSkelDataInt(v20);
                    auto v18 = v30->field_0;
                    auto v16 = this->GetSkeleton();
                    auto v19 = v16->GetName(v18);
                    auto v17 = a4->GetComponentPoseData(v30->field_0);
                    v29->CalcPoseDataRemapped(v17,
                                              v19,
                                              a2,
                                              a3,
                                              anim,
                                              skelDataInt,
                                              bit_cast<void *>(animDataInt),
                                              bit_cast<void *>(animTrackData),
                                              v30->field_C);
                }
            }
        }
    } else {
        THISCALL(0x00733EA0, this, a2, a3, a4, a5);
    }
}

void nalCompInstance_patch()
{
    {
        auto address = func_address(&nalComp::nalCompInstance::ConstructInstance);
        REDIRECT(0x005FB604, address);
        REDIRECT(0x00744E34, address);
    }

    {
        SET_JUMP(0x0073E1A0, func_address(&nalComp::nalCompInstance::_BuildEmptyPoseArray));
    }
}
