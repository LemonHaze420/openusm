#include "tlresourcedirectory.h"

#include "fixedstring.h"
#include "func_wrapper.h"
#include "log.h"

#include "nal_system.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "nglshader.h"
#include "return_address.h"
#include "scene_anim.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <cassert>
#include <cstdio>

template <>
nglTexture *tlResourceDirectory<nglTexture, tlFixedString>::StandardLoad(const tlFixedString &str)
{
    sp_log("StandardLoad %s", str.field_4);

    if constexpr (1) {
        auto *v19 = this;
        auto *tex = static_cast<nglTexture *>(tlMemAlloc(sizeof(nglTexture), 8, 0x1000000u));
        *tex = {};

        tex->field_4 = stru_975AC0.field_4;
        tex->field_0 = &stru_975AC0;
        stru_975AC0.field_4 = tex;
        tex->field_4->field_0 = tex;
        tex->field_8 = 1;
        tex->FileName = str;
        auto *v4 = str.field_4;
        [[maybe_unused]] auto v5 = 0;

        char Str1[256];
        strcpy(Str1, nglTexturePath);

        auto v7 = strlen(v4) + 1;
        auto *v8 = (char *)&v19 + 3;
        while (*++v8) {
            ;
        }

        std::memcpy(v8, v4, v7);
        auto *v10 = &Str1[&Str1[strlen(Str1) + 1] - &Str1[1]];

        char aIfl_1[] = ".ifl";

        *(uint32_t *)v10 = *(uint32_t *)aIfl_1;
        v10[4] = aIfl_1[4];

        bool v12;

        tlFileBuf v18;
        if (nglLoadingIFL || !tlReadFile(Str1, &v18, 4u, 0)) {
            char aDds[] = ".dds";
            *(uint32_t *)v10 = *(uint32_t *)aDds;
            v10[4] = aDds[4];
            if (!tlReadFile(Str1, &v18, 128u, 0)) {
                char aDdsmp_1[] = ".ddsmp";
                *(uint32_t *)v10 = *(uint32_t *)aDdsmp_1;
                *((uint16_t *)v10 + 2) = *(uint16_t *)&aDdsmp_1[4];
                v10[6] = aDdsmp_1[6];
                if (!tlReadFile(Str1, &v18, 128u, 0)) {
                    char aTga[] = ".tga";
                    *(uint32_t *)v10 = *(uint32_t *)aTga;
                    v10[4] = aTga[4];
                    if (!tlReadFile(Str1, &v18, 4u, 0)) {
                        tex->field_0->field_4 = tex->field_4;
                        tex->field_4->field_0 = tex->field_0;

                        tex->field_0 = tex;
                        tex->field_4 = tex;
                        tlMemFree(tex);
                        return nglDefaultTex;
                    }

                    STDCALL(0x007CA291, g_Direct3DDevice, (uint8_t *)v18.Buf, v18.Size, (int *)&tex->DXTexture);

                    tlReleaseFile(&v18);
                    goto LABEL_19;
                }
            }

            v12 = nglLoadTextureTM2(tex, (uint8_t *)v18.Buf);
            tlReleaseFile(&v18);
        } else {
            v12 = nglLoadTextureIFL(tex, (uint8_t *)v18.Buf, v18.Size);
            tlReleaseFile(&v18);
        }

        if (!v12) {
            tex->field_0->field_4 = tex->field_4;
            tex->field_4->field_0 = tex->field_0;
            tex->field_0 = tex;
            tex->field_4 = tex;
            tlMemFree(tex);
            return nglDefaultTex;
        }

    LABEL_19:

        auto *vtbl = bit_cast<fastcall_call(*)[1]>(this->m_vtbl);
        void(__fastcall * Add)(void *, void *, nglTexture *) = CAST(Add, (*vtbl)[4]);

        Add(this, nullptr, tex);
        return tex;
    } else {
        return (nglTexture *)THISCALL(0x0077A8A0, this, &str);
    }
}

template <>
nglFont *tlResourceDirectory<nglFont, tlFixedString>::StandardLoad(const tlFixedString &a1)
{
    TRACE("tlResourceDirectory<nglFont, tlFixedString>::StandardLoad", a1.to_string());

    if constexpr (0) {
        char Dest[256];
        _snprintf(Dest, 256u, "%s%s%s", nglTexturePath, a1.to_string(), ".fdf");

        tlFileBuf fileBuf;
        if (!tlReadFile(Dest, &fileBuf, 4u, 0)) {
            sp_log("Unable to open %s.\n", Dest);
            return nullptr;
        }

        auto *font = create_and_parse_fdf(a1, fileBuf.Buf);
        tlReleaseFile(&fileBuf);
        auto *vtbl = bit_cast<fastcall_call(*)[5]>(this->m_vtbl);

        auto *func = (*vtbl)[4];
        assert(bit_cast<std::intptr_t>(func) == 0x00773F60);

        if (bit_cast<tlInstanceBankResourceDirectory<nglFont, tlFixedString> *>(this)->Add(font)) {
            sp_log("Attempt to load already loaded font %s\n", a1.to_string());
        }

        return font;
    } else {
        return (nglFont *)THISCALL(0x00779220, this, &a1);
    }
}

template <>
nalAnimFile *tlResourceDirectory<nalAnimFile, tlFixedString>::StandardLoad(const tlFixedString &a1)
{
    if constexpr (0) {
    } else {
        return (nalAnimFile *)THISCALL(0x0078D610, this, &a1);
    }
}

template <>
nglMeshFile *tlResourceDirectory<nglMeshFile, tlFixedString>::StandardLoad(const tlFixedString &a1)
{
    TRACE("tlResourceDirectory<nglMeshFile, tlFixedString>::StandardLoad");

    if constexpr (0) {
        char Dest[256]{};
        _snprintf(Dest, 256u, "%s%s%s", nglMeshPath, a1.to_string(), ".pcmesh");
        auto *MeshFile = static_cast<nglMeshFile *>(tlMemAlloc(sizeof(nglMeshFile), 8, 0x1000000u));
        strcpy(MeshFile->FilePath, nglMeshPath);
        MeshFile->FileName = a1;
        MeshFile->field_120 = 1;
        MeshFile->field_130 = false;

        nglMeshFile *result = nullptr;
        if (tlReadFile(Dest, &MeshFile->FileBuf, 4u, 0)) {
            if (nglLoadMeshFileInternal(a1,
                                        MeshFile,
#ifdef TARGET_XBOX
                                        ".xbmesh"
#else
                                        ".pcmesh"
#endif
                                        )) {
                bool(__fastcall * Add)(void *, void *, nglMeshFile *) = CAST(Add, get_vfunc(this->m_vtbl, 0x10));

                if (Add(this, nullptr, MeshFile)) {
                    auto *v5 = a1.to_string();
                    sp_log("Attempt to load already loaded MeshFile %s\n", v5);
                }

                result = MeshFile;
            } else {
                tlReleaseFile(&MeshFile->FileBuf);
                tlMemFree(MeshFile);

                result = nullptr;
            }

        } else {
            auto *v3 = a1.to_string();
            sp_log("Unable to open %s%s%s.\n", nglMeshPath, v3, ".pcmesh");

            tlMemFree(MeshFile);
            result = nullptr;
        }

        return result;
    } else {
        return (nglMeshFile *)THISCALL(0x00770000, this, &a1);
    }
}

template <>
nalBaseSkeleton *tlResourceDirectory<nalBaseSkeleton, tlFixedString>::StandardLoad(const tlFixedString &a2)
{
    if constexpr (0) {
        tlFileBuf v6{};
        char Dest[256]{};
        _snprintf(Dest, 256u, "%s%s.%s", nalSkeletonPath, a2.field_4, "pcskel");
        if (!tlReadFile(Dest, &v6, 4u, 0)) {
            return nullptr;
        }

        auto *skeleton = static_cast<nalGeneric::nalGenericSkeleton *>(nalConstructSkeleton(v6.Buf));

        skeleton->field_50 = v6;
        this->Add(skeleton);
        return skeleton;
    } else {
        nalBaseSkeleton *(__fastcall * func)(void *, void *edx, const tlFixedString *) = CAST(func, 0x0078DD20);
        return func(this, nullptr, &a2);
    }
}

template <>
nglMorphSet *tlResourceDirectory<nglMorphSet, tlHashString>::StandardLoad(const tlHashString &)
{
    return nullptr;
}

template <>
nglMesh *tlResourceDirectory<nglMesh, tlHashString>::StandardLoad(const tlHashString &)
{
    return nullptr;
}

template <>
nglMorphFile *tlResourceDirectory<nglMorphFile, tlFixedString>::StandardLoad(const tlFixedString &a1)
{
    char Dest[256]{};
    _snprintf(Dest, 0x100u, "%s%s%s", nglMeshPath, a1.field_4, ".pcmorph");

    auto *v3 = static_cast<nglMeshFile *>(tlMemAlloc(0x148u, 8u, 0x1000000u));
    strcpy(v3->FilePath, nglMeshPath);
    v3->FileName = a1;
    v3->field_120 = 1;
    v3->field_130 = 0;

    if (tlReadFile(Dest, &v3->FileBuf, 4u, 0)) {
        if (nglLoadMeshFileInternal(a1, v3, ".pcmorph")) {
            auto *result = bit_cast<nglMorphFile *>(v3);
            this->Add(result);
            return result;
        } else {
            tlReleaseFile(&v3->FileBuf);
            tlMemFree(v3);
            return nullptr;
        }
    } else {
        tlMemFree(v3);
        return nullptr;
    }
}

template <>
nalSceneAnim *tlResourceDirectory<nalSceneAnim, tlFixedString>::StandardLoad(const tlFixedString &a2)
{
    char Dest[256]{};
    tlFileBuf a1{};

    _snprintf(Dest, 256u, "%s%s.%s", nalAnimPath, a2.field_4, "pcsanim");
    if (!tlReadFile(Dest, &a1, 4u, 0)) {
        return nullptr;
    }

    auto *Buf = (nalSceneAnim *)a1.Buf;
    *(tlFixedString *)(a1.Buf + 16) = a2;
    auto v5 = Buf->field_4;
    Buf->field_40 = a1;
    Buf->field_4 = v5 & 0xFFFFFFFB;
    if (nalLoadSceneAnimInternal(Buf)) {
        this->Add(Buf);
        return Buf;
    } else {
        tlReleaseFile(&a1);
        return nullptr;
    }
}

template <>
nalAnimClass<nalAnyPose> *
tlResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString>::StandardLoad(const tlFixedString &)
{
    return nullptr;
}

template <>
nglPaletteFile *tlResourceDirectory<nglPaletteFile, tlFixedString>::StandardLoad(const tlFixedString &a2)
{
    char Dest[256]{};

    _snprintf(Dest, 0x100u, "%s%s.ddsp", nglTexturePath, a2.field_4);
    auto *v3 = static_cast<nglPaletteFile *>(tlMemAlloc(0x140u, 8u, 0x1000000u));
    v3->field_124 = 1;
    v3->field_120 = 0;
    strcpy(v3->field_20, nglTexturePath);
    v3->field_0 = a2;
    if (tlReadFile(Dest, &v3->field_128, 0x40u, 0)) {
        if (sub_782B90(&a2, v3)) {
            this->Add(v3);
            return v3;
        } else {
            tlReleaseFile(&v3->field_128);
            tlMemFree(v3);
            return nullptr;
        }
    } else {
        tlMemFree(v3);
        return nullptr;
    }
}

template <>
int tlResourceDirectory<nalBaseSkeleton, tlFixedString>::StandardRelease(nalBaseSkeleton *a2, int a3, bool)
{
    if (a2 == nullptr) {
        return 0;
    }

    if (!a3 && --a2->field_48 > 0) {
        return a2->field_48;
    }

    this->Del(a2);
    a2->Release();

    if (a2->field_50.Buf != nullptr) {
        tlReleaseFile(&a2->field_50);
    }

    return 0;
}

template <>
int tlResourceDirectory<nglMorphSet, tlHashString>::StandardRelease(nglMorphSet *, int, bool)
{
    return 1;
}

template <>
int tlResourceDirectory<nglMorphFile, tlFixedString>::StandardRelease(nglMorphFile *a2, int a3, bool a4)
{
    if constexpr (0) {
#if 0
        int result;
        auto *v4 = a2;
        if ( a2 == nullptr ) {
            return 0;
        }

        if ( a3 || (result = a2->field_120 - 1, (a2->field_120 = result) == 0) )
        {
            if ( a2->field_4 ) {
                ((void (__thiscall *)(tlResourceDirectory__nglMorphFile_tlFixedString *, nglMorphFile *))this->m_vtbl->field_14)(
                    this,
                    a2);
            }

            auto *v6 = a2->field_138;
            if ( v6 != nullptr )
            {
                do
                {
                    for ( int i = 0; i < v6->NSections; ++i )
                    {
                        Section = v6->Sections[i].Section;
                        if ( Section->m_indices )
                        {
                            p_m_indexBuffer = &Section->m_indexBuffer;
                            if ( Section->m_indexBuffer )
                            {
                                sub_77B5D0((nglVertexBuffer *)&Section->m_indexBuffer, ResourceTypeTheme);
                                *p_m_indexBuffer = 0;
                            }
                        }
                        v10 = v6->Sections[i].Section;
                        v11 = v10->VertexBuffer.field_8;
                        p_VertexBuffer = &v10->VertexBuffer;
                        if ( v11 )
                        {
                            sub_77B5D0(p_VertexBuffer, ResourceTypeStatic);
                            p_VertexBuffer->field_8 = 0;
                        }
                    }

                    nglMeshDirectory->m_vtbl->Del(nglMeshDirectory, v6);
                    v6 = v6->NextMesh;
                }
                while ( v6 != nullptr );
                v4 = a2;
            }

            for ( auto j = v4->field_13C; j; j = *(int *)(j + 12) )
            {
                auto v14 = *(int *)(j + 4);
                if ( v14 ) {
                    (*(void (__stdcall **)(int))(*(int *)v14 + 16))(j);
                }
            }

            for ( auto k = *(int *)v4->field_140; k; k = *(int *)(k + 16) ) {
                nglMorphDirectory->Del(k);
            }

            if ( !v4->field_130 ) {
                tlReleaseFile(&v4->field_124);
            }

            tlMemFree(v4);
            return 0;
        }

        return result;
#endif
    } else {
        int(__fastcall * func)(void *, void *edx, nglMorphFile *, int, bool) = CAST(func, 0x00778B70);
        return func(this, nullptr, a2, a3, a4);
    }
}

template <>
int tlResourceDirectory<nglMeshFile, tlFixedString>::StandardRelease(nglMeshFile *a2, int a3, bool)
{
    if (a2 == nullptr) {
        return 0;
    }

    int result;

    if (a3 || (result = a2->field_120 - 1, (a2->field_120 = result) == 0)) {
        if (a2->FileName.field_4[0]) {
            this->Del(a2);
        }

        for (auto *i = a2->FirstMesh; i != nullptr; i = i->NextMesh) {
            for (auto j = 0u; j < i->NSections; ++j) {
                nglReleaseSection(i->Sections[j].Section);
            }

            nglGetMeshDirectory()->Del(i);
        }

        for (auto *k = a2->FirstMaterial; k != nullptr; k = k->NextMaterial) {
            auto *shader = k->m_shader;
            if (shader != nullptr) {
                shader->ReleaseMaterial(k);
            }
        }

        for (auto *m = a2->FirstMorph; m != nullptr; m = m->NextMorph) {
            nglGetMorphDirectory()->Del(m);
        }

        if (!a2->field_130) {
            tlReleaseFile(&a2->FileBuf);
        }

        tlMemFree(a2);
        return 0;
    }

    return result;
}

template <>
int tlResourceDirectory<nglMesh, tlHashString>::StandardRelease(nglMesh *, int, bool)
{
    return 1;
}

template <>
int tlResourceDirectory<nglTexture, tlFixedString>::StandardRelease(nglTexture *tex, int a3, bool a4)
{
    if constexpr (1) {
        if (tex == nullptr) {
            return 0;
        }

        auto v5 = (tex->field_34 >> 1) & 1;
        if (!a4 && v5) {
            return 1;
        }

        if (a3) {
            if (a3 == 1 && tex->m_format != NGLTEX_ANIMATED) {
                return tex->field_8;
            }

        } else {
            auto result = tex->field_8 - 1;
            auto v6 = tex->field_8 == 1;
            tex->field_8 = result;
            if (result >= 0 && !v6) {
                return result;
            }
        }

        if (tex->FileName.field_4[0]) {
            this->Del(tex);
        }

        nglDestroyTexture(tex);
        return -(v5 != 0);
    } else {
        return THISCALL(0x00773030, this, tex, a3, a4);
    }
}

template <>
int tlResourceDirectory<nalSceneAnim, tlFixedString>::StandardRelease(nalSceneAnim *a1, int a3, bool)
{
    if (a1 == nullptr) {
        return 0;
    }

    int result;
    bool v5;

    if (a3 || (result = a1->field_4C - 1, v5 = a1->field_4C == 1, a1->field_4C = result, result < 0 || v5)) {
        this->Del(a1);
        nalReleaseSceneAnimInternal(a1);
        if ((a1->field_4 & 4) == 0) {
            tlReleaseFile(&a1->field_40);
        }

        return 0;
    }

    return result;
}

template <>
int tlResourceDirectory<nglPaletteFile, tlFixedString>::StandardRelease(nglPaletteFile *a2, int a3, bool)
{
    if (a2 == nullptr) {
        return 0;
    }

    int result;
    bool v5;

    if (a3 || (result = a2->field_124 - 1, v5 = a2->field_124 == 1, a2->field_124 = result, result < 0 || v5)) {
        if (a2->field_0.field_4[0]) {
            this->Del(a2);
        }

        for (int i = a2->field_134[4] - 1; i >= 0; --i) {
            sub_7829F0(a2->field_13C[i]);
        }

        tlMemFree(a2->field_13C);
        auto v7 = a2->field_120;
        a2->field_13C = nullptr;
        if (!v7) {
            tlReleaseFile(&a2->field_128);
        }

        tlMemFree(a2);
        return 0;
    }

    return result;
}

template <>
int tlResourceDirectory<nalAnimFile, tlFixedString>::StandardRelease(nalAnimFile *a2, int a3, bool)
{
    if (a2 == nullptr) {
        return 0;
    }

    int result;
    bool v6;

    if (a3 || (result = a2->field_44 - 1, v6 = a2->field_44 == 1, a2->field_44 = result, result < 0 || v6)) {
        for (auto *i = a2->field_34; i != nullptr; i = i->field_4) {
            i->Release();
            nalGetAnimDirectory()->Del(i);
        }

        a2->field_4 &= ~8u;
        this->Del(a2);
        if ((a2->field_4 & 4) == 0) {
            tlReleaseFile(&a2->field_38);
        }

        return 0;
    }

    return result;
}

template <>
int tlResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString>::StandardRelease(nalAnimClass<nalAnyPose> *, int, bool)
{
    return 1;
}


void tlResourceDirectory_patch()
{
    {
        auto func = &tlResourceDirectory<nglMeshFile, tlFixedString>::Load;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00888F90, address);
    }

    return;
    {
        auto func = &tlResourceDirectory<nglFont, tlFixedString>::StandardLoad;
        FUNC_ADDRESS(address, func);
        SET_JUMP(0x00779220, address);
    }

    {
        FUNC_ADDRESS(address, (&tlResourceDirectory<nglMeshFile, tlFixedString>::Release));

        //set_vfunc(0x00888F94, address);
        //set_vfunc(0x00889070, address);
        //set_vfunc(0x008B81A8, address);
    }

    {
        FUNC_ADDRESS(address, (&tlResourceDirectory<nglTexture, tlFixedString>::StandardLoad));
        //REDIRECT(0x005606C0, address);
    }

    {
        FUNC_ADDRESS(address, (&tlResourceDirectory<nglFont, tlFixedString>::Load));
        //set_vfunc(0x008B9BD4, address);
        //set_vfunc(0x008B9C00, address);
    }
}
