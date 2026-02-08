#include "tlresourcedirectory.h"

#include "fixedstring.h"
#include "func_wrapper.h"
#include "log.h"

#include "nal_system.h"
#include "ngl.h"
#include "return_address.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <cassert>
#include <cstdio>

template<>
nglTexture *tlResourceDirectory<nglTexture, tlFixedString>::StandardLoad(const tlFixedString &str) {
    sp_log("StandardLoad %s", str.field_4);

    if constexpr (1) {
        auto *v19 = this;
        auto *tex = static_cast<nglTexture *>(tlMemAlloc(sizeof(nglTexture), 8, 0x1000000u));
        *tex = {};

        static Var<nglTexture> stru_975AC0{0x00975AC0};

        tex->field_4 = stru_975AC0().field_4;
        tex->field_0 = &stru_975AC0();
        stru_975AC0().field_4 = tex;
        tex->field_4->field_0 = tex;
        tex->field_8 = 1;
        tex->field_60 = str;
        auto *v4 = str.field_4;
        [[maybe_unused]] auto v5 = 0;

        char Str1[256];
        strcpy(Str1, nglTexturePath);

        auto v7 = strlen(v4) + 1;
        auto *v8 = (char *) &v19 + 3;
        while (*++v8) {
            ;
        }

        std::memcpy(v8, v4, v7);
        auto *v10 = &Str1[&Str1[strlen(Str1) + 1] - &Str1[1]];

        char aIfl_1[] = ".ifl";

        *(uint32_t *) v10 = *(uint32_t *) aIfl_1;
        v10[4] = aIfl_1[4];

        bool v12;

        sp_log("StandardLoad: %s", Str1);

        tlFileBuf v18;
        if (nglLoadingIFL() || !tlReadFile(Str1, &v18, 4u, 0)) {
            char aDds[] = ".dds";
            *(uint32_t *) v10 = *(uint32_t *) aDds;
            v10[4] = aDds[4];
            if (!tlReadFile(Str1, &v18, 128u, 0)) {
                char aDdsmp_1[] = ".ddsmp";
                *(uint32_t *) v10 = *(uint32_t *) aDdsmp_1;
                *((uint16_t *) v10 + 2) = *(uint16_t *) &aDdsmp_1[4];
                v10[6] = aDdsmp_1[6];
                if (!tlReadFile(Str1, &v18, 128u, 0)) {
                    char aTga[] = ".tga";
                    *(uint32_t *) v10 = *(uint32_t *) aTga;
                    v10[4] = aTga[4];
                    if (!tlReadFile(Str1, &v18, 4u, 0)) {
                        tex->field_0->field_4 = tex->field_4;
                        tex->field_4->field_0 = tex->field_0;

                        tex->field_0 = tex;
                        tex->field_4 = tex;
                        tlMemFree(tex);
                        return nglDefaultTex;
                    }

                    STDCALL(0x007CA291,
                            g_Direct3DDevice,
                            (uint8_t *) v18.Buf,
                            v18.Size,
                            (int *) &tex->DXTexture);

                    tlReleaseFile(&v18);
                    goto LABEL_19;
                }
            }

            v12 = nglLoadTextureTM2(tex, (uint8_t *) v18.Buf);
            tlReleaseFile(&v18);
        } else {
            v12 = nglLoadTextureIFL(tex, (uint8_t *) v18.Buf, v18.Size);
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
        void (__fastcall *Add)(void *, void *, nglTexture *) = CAST(Add, (*vtbl)[4]);

        Add(this, nullptr, tex);
        return tex;
    } else {
        return (nglTexture *) THISCALL(0x0077A8A0, this, &str);
    }
}

template<>
nglTexture *tlResourceDirectory<nglTexture, tlFixedString>::Load(const tlFixedString &a1) {
    sp_log("Load %s", a1.field_4);

    if constexpr (0) {
        return this->StandardLoad(a1);
    } else {
        return (nglTexture *) THISCALL(0x005606C0, this, &a1);
    }
}

template<>
nglFont *tlResourceDirectory<nglFont, tlFixedString>::StandardLoad(const tlFixedString &a1)
{
    TRACE("tlResourceDirectory<nglFont, tlFixedString>::StandardLoad", a1.to_string());

    if constexpr (0)
    {
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

        if (bit_cast<tlInstanceBankResourceDirectory<nglFont, tlFixedString> *>(this)->Add(font))
        {
            sp_log("Attempt to load already loaded font %s\n", a1.to_string());
        }

        return font;
    } else {
        return (nglFont *) THISCALL(0x00779220, this, &a1);
    }
}

template<>
nglTexture * tlResourceDirectory<nglTexture, tlFixedString>::_Find(unsigned int) {
    return nullptr;
}

template<>
nalBaseSkeleton * tlResourceDirectory<nalBaseSkeleton, tlFixedString>::_Find(unsigned int) {
    return nullptr;
}

template<>
nglFont *tlResourceDirectory<nglFont, tlFixedString>::Load(const tlFixedString &a1) {
    return this->StandardLoad(a1);
}

template<>
nalAnimFile *tlResourceDirectory<nalAnimFile, tlFixedString>::StandardLoad(const tlFixedString &a1) {
    return (nalAnimFile *) THISCALL(0x0078D610, this, &a1);
}

template<>
int tlResourceDirectory<nglMeshFile, tlFixedString>::Release(nglMeshFile *a2, int a3, bool a4) {
    sp_log("return to 0x%08X", getReturnAddress());

    //sp_log("Release %s", a2->field_124.Buf);

    return THISCALL(0x005606F0, this, a2, a3, a4);
}

template<>
nglMeshFile *tlResourceDirectory<nglMeshFile, tlFixedString>::StandardLoad(const tlFixedString &a1)
{
    TRACE("tlResourceDirectory<nglMeshFile, tlFixedString>::StandardLoad");

    if constexpr (0)
    {
        char Dest[256] {};
        _snprintf(Dest, 256u, "%s%s%s", nglMeshPath(), a1.to_string(), ".pcmesh");
        auto *MeshFile = static_cast<nglMeshFile *>(tlMemAlloc(sizeof(nglMeshFile), 8, 0x1000000u));
        strcpy(MeshFile->FilePath, nglMeshPath());
        MeshFile->FileName = a1;
        MeshFile->field_120 = 1;
        MeshFile->field_130 = false;

        nglMeshFile *result = nullptr;
        if (tlReadFile(Dest, &MeshFile->FileBuf, 4u, 0))
        {
            if (nglLoadMeshFileInternal(a1, MeshFile,
#ifdef TARGET_XBOX
                        ".xbmesh"
#else
                        ".pcmesh"
#endif
                        ))
            {
                bool (__fastcall *Add)(void *, void *, nglMeshFile *) = CAST(Add, get_vfunc(this->m_vtbl, 0x10));

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
            sp_log("Unable to open %s%s%s.\n", nglMeshPath(), v3, ".pcmesh");

            tlMemFree(MeshFile);
            result = nullptr;
        }

        return result;
    }
    else
    {
        return (nglMeshFile *) THISCALL(0x00770000, this, &a1);
    }
}

template<>
nglMeshFile *tlResourceDirectory<nglMeshFile, tlFixedString>::Load(const tlFixedString &a1)
{
    return this->StandardLoad(a1);
}

template<>
nalBaseSkeleton * tlResourceDirectory<nalBaseSkeleton, tlFixedString>::StandardLoad(
        const tlFixedString &a2)
{
    if constexpr (0) {
        tlFileBuf v6 {};
        char Dest[256] {};
        _snprintf(Dest, 256u, "%s%s.%s", nalSkeletonPath, a2.field_4, "pcskel");
        if ( !tlReadFile(Dest, &v6, 4u, 0) ) {
            return nullptr;
        }

        auto *skeleton = static_cast<nalGeneric::nalGenericSkeleton *>(nalConstructSkeleton(v6.Buf));

        skeleton->field_50 = v6;
        this->Add(skeleton);
        return skeleton;
    } else {
        nalBaseSkeleton * (__fastcall * func)(void *, void *edx, const tlFixedString *) = CAST(func, 0x0078DD20);
        return func(this, nullptr, &a2);
    }
}

template<>
int tlResourceDirectory<nalBaseSkeleton, tlFixedString>::StandardRelease(nalBaseSkeleton *a2, int a3, bool) {
    if ( a2 == nullptr ) {
        return 0;
    }

    if ( !a3 && --a2->field_48 > 0) {
        return a2->field_48;
    }

    this->Del(a2);
    a2->Release();

    if ( a2->field_50.Buf != nullptr ) {
        tlReleaseFile(&a2->field_50);
    }

    return 0;
}

template<>
nalBaseSkeleton * tlResourceDirectory<nalBaseSkeleton, tlFixedString>::Load(const tlFixedString &a2)
{
    return this->StandardLoad(a2);
}

void tlResourceDirectory_patch() {

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
