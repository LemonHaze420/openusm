#include "tl_system.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "nal_system.h"
#include "ngl.h"
#include "ngl_font.h"
#include "ngl_mesh.h"
#include "nglshader.h"
#include "osassert.h"
#include "resource_directory.h"
#include "tl_instance_bank.h"
#include "tlresourcedirectory.h"
#include "tlresource_directory.h"
#include "trace.h"
#include "us_frontend.h"
#include "utility.h"
#include "variable.h"
#include "vtbl.h"

#include <cassert>
#include <malloc.h>
#include <windows.h>

using tldir_t = tlInstanceBankResourceDirectory<nglTexture, tlFixedString>;
VALIDATE_SIZE(tldir_t, 0x14);
VALIDATE_SIZE(tldir_t::SkipListIterator, 0xC);
VALIDATE_SIZE(tldir_t::Impl, 0x10);
VALIDATE_SIZE(tldir_t::Node, 0x8);

auto & tlHostPrefix = var<char[256]>(0x00970D88);

#ifndef STANDALONE_SYSTEM
#error "Not defined macro STANDALONE_SYSTEM"
#endif

#if !STANDALONE_SYSTEM

int & tlMemAllocCounter = var<int>(0x00970D58);

tlSystemCallbacks & tlCurSystemCallbacks = var<tlSystemCallbacks>(0x00970D6C);

static int & tlStackBegin = var<int>(0x00970E88);
static int & tlStackEnd = var<int>(0x00970E8C);

int & tlScratchPadRefCount = var<int>(0x00970D5C);

tlInstanceBank & nglShaderBank = var<tlInstanceBank>(0x00972840);

#else

#define make_var(type, name) \
    static type g_##name {}; \
    type & name {g_##name}

make_var(int, tlMemAllocCounter);

make_var(tlSystemCallbacks, tlCurSystemCallbacks);

#undef make_var

static int & tlStackBegin = []() -> auto & {
    static int tlStackBegin {};
    return tlStackBegin;
}();

static int & tlStackEnd = []() -> auto & {
    static int g_tlStackEnd {};
    return g_tlStackEnd;
}();

int & tlScratchPadRefCount = []() -> auto & {
    static int g_tlScratchPadRefCount {};
    return g_tlScratchPadRefCount;
}();

tlInstanceBank & nglShaderBank = []() -> auto & {
    static tlInstanceBank g_nglShaderBank {};
    return g_nglShaderBank;
}();

FrontEnd_Shader & gFrontEnd_Shader = []() -> auto & {
    static FrontEnd_Shader g_shader {};
    return g_shader;
}();

tlInitListFunction InitList_Init_nglVertexDef_FrontEnd_builder {
    &Init_nglVertexDef_FrontEnd_builder
};

#endif

template<>
nglFont *tlInstanceBankResourceDirectory<nglFont, tlFixedString>::Impl::Find(
    const tlFixedString &a1) {
    if (this->field_8 == nullptr) {
        return nullptr;
    }

    auto *v7 = this->field_8;
    auto v8 = this->m_size;

    Node *v6 = nullptr;
    do {
        while (1) {
            v6 = v7->field_4[v8];
            if (v6 == nullptr) {
                break;
            }

            auto *v3 = &v6->field_0->field_0;
            auto v5 = v3->compare(a1);
            if (v5 == 0) {
                return v6->field_0;
            }

            if (v5 > 0) {
                break;
            }

            v7 = v6;
        }

        --v8;
    } while (v8 >= 0);

    if (v6 != nullptr) {
        if (auto &v4 = v6->field_0->field_0; v4 == a1) {
            return v6->field_0;
        }
    }

    return nullptr;
}

void tlReleaseFile(tlFileBuf *File)
{
    if (tlCurSystemCallbacks.ReleaseFile != nullptr) {
        tlCurSystemCallbacks.ReleaseFile(File);
    } else {
        char *fileBuf = File->Buf;

        --tlMemAllocCounter;
        if (tlCurSystemCallbacks.MemFree != nullptr) {
            tlCurSystemCallbacks.MemFree(fileBuf);
        } else {
            _aligned_free(fileBuf);
        }

        File->Buf = nullptr;
        File->Size = 0;
        File->UserData = 0;
    }
}

void tlStackRangeInit() {
    tlStackBegin = 0;
    tlStackEnd = 0;
}

bool sub_101BF70(int a1)
{
    return a1 >= tlStackBegin && a1 < tlStackEnd;
}

bool tlIsPow2(int a1) {
    return a1 && (a1 & (a1 - 1)) == 0;
}

void tlMemFree(void *Ptr) {
    --tlMemAllocCounter;

    if (tlCurSystemCallbacks.MemFree != nullptr) {
        tlCurSystemCallbacks.MemFree(Ptr);
    } else {
        _aligned_free(Ptr);
    }
}

void *tlMemAlloc(uint32_t Size, uint32_t Alignment, uint32_t Flags) {
    TRACE("tlMemAlloc", std::to_string(Size).c_str());

    void *memPtr;

    auto alignChk = Alignment;

    assert((Size & 15) == (Size % 16));

    if constexpr (0) {
        if ((Alignment == 0) && !(Size & 15)) {
            alignChk = 0;
        }
    } else {
        if (!(Size & 15)) {
            alignChk = (Alignment + 15) & 0xFFFFFFF0;
        }
    }

    ++tlMemAllocCounter;

    if (tlCurSystemCallbacks.MemAlloc != nullptr) {
        memPtr = tlCurSystemCallbacks.MemAlloc(Size, alignChk, Flags);
    } else {
        memPtr = _aligned_malloc(Size, alignChk);
    }

    if (!(Flags & 2) && (memPtr == nullptr) && Size) {
        error("Memory allocation failed. %d bytes, %d align", Size, alignChk);
    }

    return memPtr;
}

bool tlReadFile(const char *FileName, tlFileBuf *File, unsigned int Align, unsigned int Flags)
{
    TRACE("tlReadFile", FileName);

    if constexpr (1)
    {
        char *allocLoc;
        unsigned int bytesRead = 0;
        char Work[512] = {0};

        unsigned int alignment = Align;
        if (tlCurSystemCallbacks.ReadFile != nullptr) {
            return tlCurSystemCallbacks.ReadFile(FileName, File, Align, Flags);
        }

        if (strncmp(FileName, tlHostPrefix, strlen(tlHostPrefix)) == 0) {
            Work[0] = 0;
        } else {
            int fileLoc = 0;
            char fileRead = '\0';
            do {
                fileRead = tlHostPrefix[fileLoc];
                Work[fileLoc++] = fileRead;
            } while (fileRead);
        }

        size_t fileLen = strlen(FileName) + 1;
        auto *filePtr = (char *) &alignment + 3;
        while (*++filePtr) {
            ;
        }

        std::memcpy(filePtr, FileName, fileLen);
        for (auto *i = Work; *i; ++i) {
            if (*i == '/') {
                *i = '\\';
            }
        }

        HANDLE openedFile = CreateFileA(Work, 0x80000000, 1u, nullptr, 3u, 0x8000080u, nullptr);
        if (openedFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        uint32_t fileSize = GetFileSize(openedFile, nullptr);
        File->Size = fileSize;
        if (!alignment && !(fileSize & 0xF)) {
            alignment = 0;
        }

        ++tlMemAllocCounter;
        if (tlCurSystemCallbacks.MemAlloc != nullptr) {
            allocLoc = static_cast<char *>(tlCurSystemCallbacks.MemAlloc(fileSize, alignment, Flags));
        } else {
            allocLoc = static_cast<char *>(_aligned_malloc(fileSize, alignment));
        }

        if (!(Flags & 2) && allocLoc == nullptr) {
            if (fileSize) {
                error("Memory allocation failed. %d bytes, %d align", fileSize, alignment);
            }
        }

        fileSize = File->Size;
        File->Buf = allocLoc;
        ReadFile(openedFile, allocLoc, fileSize, (LPDWORD) &bytesRead, nullptr);
        CloseHandle(openedFile);
        return true;
    }
    else
    {
        return (bool) CDECL_CALL(0x0074A710, FileName, File, Align, Flags);
    }
}

void tlSetSystemCallbacks(const tlSystemCallbacks &a1) {
    tlCurSystemCallbacks = a1;
}

void tlGetSystemCallbacks(tlSystemCallbacks *a1) {
    a1->ReadFile = tlCurSystemCallbacks.ReadFile;
    a1->ReleaseFile = tlCurSystemCallbacks.ReleaseFile;
    a1->field_8 = tlCurSystemCallbacks.field_8;
    a1->field_C = tlCurSystemCallbacks.field_C;
    a1->MemAlloc = tlCurSystemCallbacks.MemAlloc;
    a1->MemFree = tlCurSystemCallbacks.MemFree;
}

void set_tl_system_directories()
{
    if constexpr (1) {
        tlresource_directory<nglTexture,tlFixedString>::system_dir = nglGetTextureDirectory();
        tlresource_directory<nglMeshFile,tlFixedString>::system_dir = nglGetMeshFileDirectory();
        tlresource_directory<nglMesh,tlHashString>::system_dir = nglGetMeshDirectory();
        tlresource_directory<nglMorphFile,tlFixedString>::system_dir = nglGetMorphFileDirectory();
        tlresource_directory<nglMorphSet,tlHashString>::system_dir = nglGetMorphDirectory();
        tlresource_directory<nalAnimFile,tlFixedString>::system_dir = nalGetAnimFileDirectory();
        tlresource_directory<nalAnimClass<nalAnyPose>,tlFixedString>::system_dir = nalGetAnimDirectory();
        tlresource_directory<nalBaseSkeleton,tlFixedString>::system_dir = nalGetSkeletonDirectory();
        tlresource_directory<nalSceneAnim,tlFixedString>::system_dir = nalGetSceneAnimDirectory();
        tlresource_directory<nglTexture,tlFixedString>::default_tlres = nglDefaultTex;
        tlresource_directory<nglMeshFile,tlFixedString>::default_tlres = nullptr;
        tlresource_directory<nglMesh,tlHashString>::default_tlres = nullptr;
        tlresource_directory<nglMorphSet,tlHashString>::default_tlres = nullptr;
        tlresource_directory<nglMorphFile,tlFixedString>::default_tlres = nullptr;
        tlresource_directory<nalAnimFile,tlFixedString>::default_tlres = nullptr;
        tlresource_directory<nalAnimClass<nalAnyPose>,tlFixedString>::default_tlres = nullptr;
        tlresource_directory<nalSceneAnim,tlFixedString>::default_tlres = nullptr;
        tlresource_directory<nalBaseSkeleton,tlFixedString>::default_tlres = nullptr;
    } else {
        CDECL_CALL(0x0050EAD0);
    }
}

//0x0078A160
template<>
void tlInstanceBankResourceDirectory<nglTexture, tlFixedString>::SkipListIterator::reset() {
    auto *v1 = this->field_4->field_8;
    if (v1 != nullptr) {
        this->field_8 = v1->field_4[0];
    } else {
        this->field_8 = nullptr;
    }
}

//0x00773CB0
template<>
nglTexture *tlInstanceBankResourceDirectory<nglTexture, tlFixedString>::SkipListIterator::operator*() {
    nglTexture *result = nullptr;
    auto v1 = this->field_8;
    if (v1 != nullptr) {
        result = v1->field_0;
    }

    return result;
}

//0x00778DF0
template<>
bool tlInstanceBankResourceDirectory<nglTexture, tlFixedString>::SkipListIterator::operator()() {
    return this->field_8 != nullptr;
}

//0x0077A160
template<>
void tlInstanceBankResourceDirectory<nglTexture, tlFixedString>::SkipListIterator::operator++() {
    auto *v1 = this->field_8;
    if (v1 != nullptr) {
        this->field_8 = v1->field_4[0];
    }
}

void tlInitList::Register() {
    void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x0));
    func(this);
}

tlInitListFunction::tlInitListFunction(void (*cb)()) : field_8(cb) {
    if constexpr (1) {
        static void * g_vtbl[] {
            func_address(&_Register)
        };
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0086F85C;
    }
}

void tlInitListFunction::_Register()
{
    this->field_8();
}

void tlInitListInit()
{
    TRACE("tlInitListInit");

    if constexpr (1)
    {
        for (auto *item = tlInitList::head;
                item != nullptr;
                item = item->field_4)
        {
            item->Register();
        }

    } else {
        CDECL_CALL(0x00749FD0);
    }
}

void tl_patch() {

    SET_JUMP(0x00749FD0, tlInitListInit);

    SET_JUMP(0x0074A710, tlReadFile);

    SET_JUMP(0x0074A5C0, tlMemAlloc);

    SET_JUMP(0x0074A600, tlMemFree);

    SET_JUMP(0x0074A6C0, tlReleaseFile);

    SET_JUMP(0x0074A520, tlSetSystemCallbacks);

    {
        auto func = &tlInstanceBankResourceDirectory<nglTexture, tlFixedString>::Add;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008B8D24, address);
    }

    {
        auto func = &tlInstanceBankResourceDirectory<nglMesh, tlHashString>::_Find;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008B81E0, address);
    }

    {
        auto func = &tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString>::Add;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008BDDA4, address);
    }

    return;

    {
        auto func = &tlInstanceBankResourceDirectory<nglTexture, tlFixedString>::_Find;

        FUNC_ADDRESS(address, func);
        //set_vfunc(0x008B8D20, address);
    }

    {
        auto func = &tlInstanceBankResourceDirectory<nglTexture, tlFixedString>::Del;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008B8D28, address);
    }

    {
        auto func = &tlInstanceBankResourceDirectory<nglMeshFile, tlFixedString>::_Find;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008B818C, address);
    }

    {
        auto func = &tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString>::_Find;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008BDDA0, address);
    }

    {
        auto func = &tlInstanceBankResourceDirectory<nglMeshFile, tlFixedString>::Add;

        FUNC_ADDRESS(address, func);
        set_vfunc(0x008B8190, address);
    }

    {
        auto func = &tlResourceDirectory<nglTexture, tlFixedString>::ReleaseAll;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008B8D30, address);
    }

    {
        tlInstanceBank::Node *(tlInstanceBank::*func)(
            const tlHashString &a2) = &tlInstanceBank::Search;
        FUNC_ADDRESS(address, func);
        //SET_JUMP(0x0074A310, address);
    }

    {
        FUNC_ADDRESS(address, &tlInstanceBank::Insert);
        //REDIRECT(0x007837C8, address);
        //REDIRECT(0x00402118, address);

        //REDIRECT(0x00402BF8, address);
        //REDIRECT(0x00403B08, address);

        REDIRECT(0x004115B9, address);
    }
}
