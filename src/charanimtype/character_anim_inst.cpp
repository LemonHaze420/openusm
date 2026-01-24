#include "character_anim_inst.h"

#include "character_pose_skel.h"
#include "common.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

VALIDATE_SIZE(nalChar::nalCharInstance, 0x20u);

void nalChar::nalCharInstance::finalize(bool a2)
{
    this->~nalCharInstance();
    if ( (a2 & 1) != 0 ) {
        delete(this);
    }
}

void * nalChar::nalCharInstance::operator new(size_t size)
{
    return tlMemAlloc(size, 8u, 0);
}

void nalChar::nalCharInstance::operator delete(void *ptr)
{
    tlMemFree(ptr);
}

nalChar::nalCharInstance::nalCharInstance(
        nalChar::nalCharAnim *a2,
        nalChar::nalCharSkeleton *a3) : nalCompInstance(a2, a3)
{
    this->m_vtbl = 0x00891FF4;
    this->ConstructInstance();
}

nalChar::nalCharInstance::~nalCharInstance()
{
    TRACE("nalCharInstance::~nalCharInstance");

    this->m_vtbl = 0x00891FF4;

    auto *SkeletonFromInstance = this->GetSkeleton();
    auto *Anim = bit_cast<nalComp::nalCompAnim *>(this->field_10);

    for (int v17 = 0; v17 < this->field_18; ++v17)
    {
        auto *v5 = &this->field_14[v17];
        if ( v5->field_8 != -1 && v5->field_11 )
        {
            auto v6 = v5->field_0;

            auto *ComponentFromInstance = SkeletonFromInstance->GetComponent(v6);
            if ( v5->field_10 )
            {
                auto *CompPerSkelDataInt = SkeletonFromInstance->GetCompPerSkelDataInt(v6);
                auto Name = SkeletonFromInstance->GetName(v6);
                auto *CompPerAnimDataInt = (const void *) Anim->GetCompPerAnimDataInt(v5->field_8);
                ComponentFromInstance->DestroyPerInstData(
                    v5->field_C,
                    Name,
                    CompPerSkelDataInt,
                    CompPerAnimDataInt
                );
            }
            else
            {
                auto *Skeleton = (nalComp::nalCompSkeleton *)this->GetSkeleton();
                auto v11 = v5->field_8;
                auto *component = bit_cast<CharComponentBase *>(Skeleton->field_70[v11].m_component);
                auto *CompPerSkelDataInt = SkeletonFromInstance->GetCompPerSkelDataInt(v6);
                auto toName = Skeleton->GetName(v11);
                auto fromName = SkeletonFromInstance->GetName(v6);
                auto *v15 = (const void *) Anim->GetCompPerAnimDataInt(v11);
                component->DestroyRemapPerInstData(
                    v5->field_C,
                    fromName,
                    toName,
                    component,
                    CompPerSkelDataInt,
                    v15
                );
            }
        }
    }

    tlMemFree(this->field_14);
    this->field_14 = nullptr;

    tlMemFree(this->field_1C);
    this->field_1C = nullptr;

    this->field_18 = 0;
}

void nalChar::nalCharInstance::VirtualGetPose(
        Float a1,
        Float a2,
        nalBasePose *a3,
        const nalBasePose *a4)
{
    TRACE("nalChar::nalCharInstance::VirtualGetPose");

    const nalCharPose *v5 = nullptr;
    if ( a4 != nullptr ) {
        v5 = (const nalCharPose *)&a4[-1];
    }

    nalCharPose *v6 = nullptr;
    if (a3 != nullptr) {
        v6 = (nalCharPose *) &a3[-1];
    }

    this->GetPose(a1, a2, v6, v5);
}

void nalChar::nalCharInstance::BuildPerInstData()
{
    auto dwSize = 0;
    auto a2 = 1;

    auto func = [](int a1, int a2) -> int
    {
          return ~(a2 - 1) & (a1 + a2 - 1);
    };

    int *v77 = new int[this->field_18];

    auto *Skeleton = this->GetSkeleton();
    auto *v76 = Skeleton;

    auto *Anim = this->GetAnim();
    auto *v75 = Anim;
    for ( int i = 0; i < this->field_18; ++i )
    {
        auto *v73 = &this->field_14[i];
        auto *v72 = v76->GetComponent(v73->field_0);
        if ( v73->field_11
                && (!v72->GetDomain() || v73->field_10) )
        {
            auto v54 = !v73->field_10;
            auto v48 = Anim->GetCompAnimTrackData(v73->field_8);
            auto CompPerAnimDataInt = v75->GetCompPerAnimDataInt(v73->field_8);
            auto v33 = Skeleton->GetCompDefaultPoseData(v73->field_0);
            auto v24 = v73->field_8;
            auto v2 = v75->GetSkeleton();
            auto v25 = v2->GetCompPerSkelDataInt(v24);
            auto v21 = v76->GetCompPerSkelDataInt(v73->field_0);
            auto Name = v76->GetName(v73->field_0);
            auto v71 = v72->GetSizeOfPerInstData(
                    Name,
                    v21,
                    v25,
                    v33,
                    CompPerAnimDataInt,
                    v48,
                    v54);

            auto v55 = !v73->field_10;
            auto v49 = Anim->GetCompAnimTrackData(v73->field_8);
            auto v43 = v75->GetCompPerAnimDataInt(v73->field_8);
            auto v34 = Skeleton->GetCompDefaultPoseData(v73->field_0);
            auto v26 = v73->field_8;
            auto *v5 = v75->GetSkeleton();
            auto v27 = v5->GetCompPerSkelDataInt(v26);
            auto v22 = v76->GetCompPerSkelDataInt(v73->field_0);
            auto v6 = v76->GetName(v73->field_0);
            auto v70 = v72->GetAlignOfPerInstData(
                    v6,
                    v22,
                    v27,
                    v34,
                    v43,
                    v49,
                    v55);
            if ( !dwSize && v71 ) {
                a2 = v70;
            }

            if ( v71 )
            {
                auto v7 = func(dwSize, v70);
                v77[i] = v7;
                dwSize = v71 + v7;
            }
            else
            {
                v77[i] = -1;
            }
        }
        else if ( v73->field_11 )
        {
            auto v69 = v75->GetSkeleton();
            auto v68 = v69->GetComponent(v73->field_8);
            auto v56 = Anim->GetCompAnimTrackData(v73->field_8);
            auto v50 = v75->GetCompPerAnimDataInt(v73->field_8);
            auto v44 = Skeleton->GetCompDefaultPoseData(v73->field_0);
            auto v35 = v73->field_8;
            auto v8 = v75->GetSkeleton();
            auto v36 = v8->GetCompPerSkelDataInt(v35);
            auto v28 = Skeleton->GetCompPerSkelDataInt(v73->field_0);
            auto v18 = v69->GetName(v73->field_8);
            auto v16 = v76->GetName(v73->field_0);
            auto v67 = v72->GetRemapSizeOfPerInstData(
                    v16,
                    v18,
                    v68,
                    v28,
                    v36,
                    v44,
                    v50,
                    v56);

            auto v57 = Anim->GetCompAnimTrackData(v73->field_8);
            auto v51 = v75->GetCompPerAnimDataInt(v73->field_8);
            auto v45 = Skeleton->GetCompDefaultPoseData(v73->field_0);
            auto v37 = v73->field_8;
            auto v9 = v75->GetSkeleton();
            auto v38 = v9->GetCompPerSkelDataInt(v37);
            auto v29 = v76->GetCompPerSkelDataInt(v73->field_0);
            auto v19 = v69->GetName(v73->field_8);
            auto v17 = v76->GetName(v73->field_0);
            auto v66 = v72->GetRemapAlignOfPerInstData(
                    v17,
                    v19,
                    v68,
                    v29,
                    v38,
                    v45,
                    v51,
                    v57);
            if ( !dwSize && v67 ) {
                a2 = v66;
            }

            if ( v67 )
            {
                auto v10 = func(dwSize, v66);
                v77[i] = v10;
                dwSize = v67 + v10;
            }
            else
            {
                v77[i] = -1;
            }
        }
        else
        {
            v77[i] = -1;
        }
    }

    this->field_1C = tlMemAlloc(dwSize, a2, 0);
    auto *v65 = (char *)this->field_1C;
    for ( int j = 0; j < this->field_18; ++j )
    {
        if ( v77[j] == -1 )
        {
            this->field_14[j].field_C = nullptr;
        }
        else
        {
            auto *v63 = &this->field_14[j];
            auto v62 = v76->GetComponent(v63->field_0);
            this->field_14[j].field_C = &v65[v77[j]];
            if ( v63->field_10 )
            {
                auto v58 = !v63->field_10;
                auto v52 = Anim->GetCompAnimTrackData(v63->field_8);
                auto v46 = v75->GetCompPerAnimDataInt(v63->field_8);
                auto v39 = Skeleton->GetCompDefaultPoseData(v63->field_0);
                auto v30 = v63->field_8;
                auto v12 = Anim->GetSkeleton();
                auto v31 = v12->GetCompPerSkelDataInt(v30);
                auto v23 = v76->GetCompPerSkelDataInt(v63->field_0);
                auto v13 = v76->GetName(v63->field_0);
                v62->BuildPerInstData(
                    v63->field_C,
                    v13,
                    v23,
                    v31,
                    v39,
                    v46,
                    v52,
                    v58);
            }
            else
            {
                auto v61 = v75->GetSkeleton();
                auto v60 = v61->GetComponent(v63->field_8);
                auto v59 = v75->GetCompAnimTrackData(v63->field_8);
                auto v53 = v75->GetCompPerAnimDataInt(v63->field_8);
                auto v47 = Skeleton->GetCompDefaultPoseData(v63->field_0);
                auto v40 = v63->field_8;
                auto v14 = v75->GetSkeleton();
                auto v41 = v14->GetCompPerSkelDataInt(v40);
                auto v32 = v76->GetCompPerSkelDataInt(v63->field_0);
                auto v20 = v61->GetName(v63->field_8);
                auto v15 = v76->GetName(v63->field_0);
                v62->BuildRemapPerInstData(
                    v63->field_C,
                    v15,
                    v20,
                    v60,
                    v32,
                    v41,
                    v47,
                    v53,
                    v59);
            }
        }
    }

    delete[](v77);
}

nalChar::nalCharInstance * nalChar::nalCharAnim::CreateInstance(nalChar::nalCharSkeleton *a2)
{
    auto *result = new nalCharInstance(this, a2);
    return result;
}

nalComp::nalCompInstance * nalChar::nalCharAnim::VirtualCreateInstance(
        nalBaseSkeleton *a1)
{
    TRACE("nalCharAnim::VirtualCreateInstance");

    return this->CreateInstance(bit_cast<nalCharSkeleton *>(a1));
}

void * nalChar::nalCharAnim::GetPerAnimDataByName(CharComponentBase::Names a2)
{
    TRACE("nalCharAnim::GetPerAnimDataByName");

    auto *v3 = (nalCharSkeleton *) this->GetSkeleton();
    int CompIxByName = v3->GetCompIxByName(a2);
    if ( CompIxByName == -1 ) {
        return nullptr;
    }

    auto *CompPerAnimDataInt = (const void *) this->GetCompPerAnimDataInt(CompIxByName);
    auto *v9 = this->field_30->field_70[CompIxByName].m_component;
    return v9->ApplyPublicPerAnimDataOffset(a2, CompPerAnimDataInt);
}

void nalCharInstance_patch()
{
    {
        auto constexpr address_vtbl = 0x00891FF4;

        set_vfunc(address_vtbl + 0x4, func_address(&nalChar::nalCharInstance::VirtualGetPose));
        set_vfunc(address_vtbl + 0x8, func_address(&nalComp::nalCompInstance::_BuildDirectMapping));

        set_vfunc(address_vtbl + 0x10, func_address(&nalComp::nalCompInstance::_BuildEmptyPoseArray));
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharAnim::VirtualCreateInstance);
        set_vfunc(0x00891FE0, address);
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharAnim::GetPerAnimDataByName);
        SET_JUMP(0x005F0840, address);
    }
}
