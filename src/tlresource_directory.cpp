#include "tlresource_directory.h"

#include "debugutil.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "os_developer_options.h"
#include "resource_directory.h"
#include "trace.h"
#include "tlresource_location.h"
#include "utility.h"
#include "vtbl.h"

#include "variables.h"

#include <cassert>

#if !STANDALONE_SYSTEM

#define make_system_dir(T0, T1, address)                        \
    template<>                                                  \
    tlInstanceBankResourceDirectory<T0, T1> *&                  \
        tlresource_directory<T0, T1>::system_dir =              \
        var<tlInstanceBankResourceDirectory<T0, T1> *>(address)

make_system_dir(nglTexture, tlFixedString, 0x00960A10);
make_system_dir(nglMesh, tlHashString, 0x00960A08);
make_system_dir(nglMeshFile, tlFixedString, 0x00960A0C);
make_system_dir(nglMorphFile, tlFixedString, 0x00960A04);
make_system_dir(nglMorphSet, tlHashString, 0x00960A00);
make_system_dir(nalAnimClass<nalAnyPose>, tlFixedString, 0x009609F0);
make_system_dir(nalAnimFile, tlFixedString, 0x009609F4);
make_system_dir(nglMaterialBase, tlHashString, 0x009609F8);
make_system_dir(nglMaterialFile, tlFixedString, 0x009609FC);
make_system_dir(nalBaseSkeleton, tlFixedString, 0x009609E8);
make_system_dir(nalSceneAnim, tlFixedString, 0x009609EC);

#undef make_system_dir

#define make_default_tlres(T0, T1, address)                                 \
    template<>                                                              \
    T0 *& tlresource_directory<T0, T1>::default_tlres = var<T0 *>(address)

make_default_tlres(nglMesh, tlHashString, 0x009609DC);
make_default_tlres(nalBaseSkeleton, tlFixedString, 0x009609BC);
make_default_tlres(nglMeshFile, tlFixedString, 0x009609E0);
make_default_tlres(nglTexture, tlFixedString, 0x009609E4);
make_default_tlres(nalSceneAnim, tlFixedString, 0x009609C0);
make_default_tlres(nalAnimClass<nalAnyPose>, tlFixedString, 0x009609C4);
make_default_tlres(nalAnimFile, tlFixedString, 0x009609C8);
make_default_tlres(nglMorphFile, tlFixedString, 0x009609D4);
make_default_tlres(nglMorphSet, tlHashString, 0x009609D8);

#undef make_default_tlres

#else

#define make_system_dir(T0, T1)                                          \
template<>                                                               \
tlInstanceBankResourceDirectory<T0, T1> *&                               \
    tlresource_directory<T0, T1>::system_dir = []() -> auto & {          \
        static tlInstanceBankResourceDirectory<T0, T1> *g_system_dir {}; \
        return g_system_dir;                                             \
    }()

make_system_dir(nglTexture, tlFixedString);
make_system_dir(nglMesh, tlHashString);
make_system_dir(nglMeshFile, tlFixedString);
make_system_dir(nglMorphSet, tlHashString);
make_system_dir(nglMorphFile, tlFixedString);
make_system_dir(nglMaterialFile, tlFixedString);
make_system_dir(nglMaterialBase, tlHashString);
make_system_dir(nalAnimClass<nalAnyPose>, tlFixedString);
make_system_dir(nalAnimFile, tlFixedString);
make_system_dir(nalBaseSkeleton, tlFixedString);
make_system_dir(nalSceneAnim, tlFixedString);

#undef make_system_dir

#define make_default_tlres(T0, T1)                                       \
    template<>                                                           \
    T0 *& tlresource_directory<T0, T1>::default_tlres = []() -> T0 *& {  \
        static T0 *g_default_tlres {};                                   \
        return g_default_tlres;                                          \
    }()

make_default_tlres(nglMesh, tlHashString);
make_default_tlres(nalBaseSkeleton, tlFixedString);
make_default_tlres(nglMeshFile, tlFixedString);
make_default_tlres(nglTexture, tlFixedString);
make_default_tlres(nalSceneAnim, tlFixedString);
make_default_tlres(nalAnimClass<nalAnyPose>, tlFixedString);
make_default_tlres(nalAnimFile, tlFixedString);
make_default_tlres(nglMorphFile, tlFixedString);
make_default_tlres(nglMorphSet, tlHashString);

#undef make_default_tlres

#endif

#define constructor_tlresource_directory(T0, T1, vtbl) \
    template<> \
    tlresource_directory<T0, T1>::tlresource_directory() \
    { \
        this->m_vtbl = vtbl; \
        this->field_4 = nullptr; \
    }

constructor_tlresource_directory(nglTexture, tlFixedString, 0x00889648)
constructor_tlresource_directory(nglMeshFile, tlFixedString, 0x00889674)
constructor_tlresource_directory(nglMesh, tlHashString, 0x008896A0)
constructor_tlresource_directory(nglMorphSet, tlHashString, 0x008896CC)
constructor_tlresource_directory(nglMorphFile, tlFixedString, 0x008896F8)
constructor_tlresource_directory(nglMaterialBase, tlHashString, 0x00889724)
constructor_tlresource_directory(nglMaterialFile, tlFixedString, 0x00889750)
constructor_tlresource_directory(nalAnimFile, tlFixedString, 0x0088977C)
constructor_tlresource_directory(nalAnimClass<nalAnyPose>, tlFixedString, 0x008897A8)
constructor_tlresource_directory(nalSceneAnim, tlFixedString, 0x008897D4)
constructor_tlresource_directory(nalBaseSkeleton, tlFixedString, 0x00889800)

#undef constructor_tlresource_directory


template<>
int tlresource_directory<nglMesh, tlHashString>::tlres_type = 3;

template<>
nglMeshFile *tlresource_directory<nglMeshFile, tlFixedString>::Find(const tlFixedString &a2)
{
    TRACE("tlresource_directory<nglMeshFile,tlFixedString>::Find", a2.to_string());

    if constexpr (0)
    {
        nglMeshFile *v5 = nullptr;
        if (this->field_4 != nullptr)
        {
            v5 = CAST(v5, this->field_4->get_tlresource(a2, TLRESOURCE_TYPE_MESH_FILE));
        }

        if ( v5 == nullptr && system_dir != nullptr )
        {
            v5 = system_dir->Find(a2);
            bool SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString {"SHOW_RESOURCE_SPAM"});
            if ( v5 != nullptr )
            {
                if ( SHOW_RESOURCE_SPAM )
                {
                    auto *v2 = a2.to_string();
                    debug_print_va("found tlresource %s in system directory", v2);
                }
            }
            else if ( SHOW_RESOURCE_SPAM )
            {
                auto *v3 = a2.to_string();
                debug_print_va("didn't find tlresource %s in system directory", v3);
            }
        }

        if ( v5 == nullptr )
        {
            v5 = (nglMeshFile *) default_tlres;
        }

        return v5;
    }
    else
    {
        return (nglMeshFile *) THISCALL(0x005692B0, this, &a2);
    }
}

template<>
nglMesh *tlresource_directory<nglMesh,tlHashString>::Find(const tlHashString &a2)
{
    TRACE("tlresource_directory<nglMesh,tlHashString>::Find");

    if constexpr (0)
    {
        nglMesh *v5 = nullptr;
        if ( this->field_4 != nullptr )
        {
            v5 = (nglMesh *) this->field_4->get_tlresource(a2, TLRESOURCE_TYPE_MESH);
        }

        if ( v5 == nullptr && system_dir != nullptr )
        {
            v5 = system_dir->Find(a2);

            bool SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString {"SHOW_RESOURCE_SPAM"});
            if ( v5 != nullptr )
            {
                if ( SHOW_RESOURCE_SPAM )
                {
                    auto *v2 = a2.c_str();
                    debug_print_va("found tlresource %s in system directory", v2);
                }
            }
            else if ( SHOW_RESOURCE_SPAM )
            {
                auto *v3 = a2.c_str();
                debug_print_va("didn't find tlresource %s in system directory", v3);
            }
        }

        if ( v5 == nullptr ) {
            v5 = default_tlres;
        }

        return v5;
    }
    else
    {
        return (nglMesh *) THISCALL(0x005693B0, this, &a2);
    }
}

template<>
void tlresource_directory<nglMesh, tlHashString>::Add([[maybe_unused]] nglMesh *Mesh)
{
    TRACE("tlresource_directory<nglMesh, tlHashString>::Add");
    ;
}


//0x00569BA0
template<>
nalBaseSkeleton *tlresource_directory<nalBaseSkeleton, tlFixedString>::Find(const tlFixedString &a1)
{
    TRACE("tlresource_directory<nalBaseSkeleton, tlFixedString>::Find", a1.to_string());

    if constexpr (0)
    {
        nalBaseSkeleton *v5 = nullptr;
        if ( this->field_4 != nullptr )
        {
            v5 = (nalBaseSkeleton *) this->field_4->get_tlresource(a1, TLRESOURCE_TYPE_SKELETON);
        }

        if ( v5 == nullptr && system_dir != nullptr )
        {
            v5 = system_dir->Find(a1);

            auto SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString {"SHOW_RESOURCE_SPAM"});
            if ( v5 != nullptr )
            {
                if ( SHOW_RESOURCE_SPAM )
                {
                    auto *v2 = a1.to_string();
                    debug_print_va("found tlresource %s in system directory", v2);
                }
            }
            else if ( SHOW_RESOURCE_SPAM )
            {
                auto *v3 = a1.to_string();
                debug_print_va("didn't find tlresource %s in system directory", v3);
            }
        }

        if ( v5 == nullptr ) {
            v5 = default_tlres;
        }

        return v5;
    }
    else
    {
        return (nalBaseSkeleton *) THISCALL(0x00569BA0, this, &a1);
    }
}

//0x005691B0
template<>
nglTexture *tlresource_directory<nglTexture, tlFixedString>::Find(const tlFixedString &a1) 
{
    TRACE("tlresource_directory<nglTexture, tlFixedString>::Find(const tlFixedString &)", a1.to_string());

    if constexpr (0)
    {
        nglTexture *v5 = nullptr;
        if ( this->field_4 != nullptr )
        {
            v5 = (nglTexture *) this->field_4->get_tlresource(a1, TLRESOURCE_TYPE_TEXTURE);
        }

        if ( v5 == nullptr && system_dir != nullptr )
        {
            auto SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString {"SHOW_RESOURCE_SPAM"});
            v5 = system_dir->Find(a1);
            if ( v5 != nullptr )
            {
                if ( SHOW_RESOURCE_SPAM )
                {
                    auto *v2 = a1.to_string();
                    debug_print_va("found tlresource %s in system directory", v2);
                }
            }
            else if ( SHOW_RESOURCE_SPAM )
            {
                auto *v3 = a1.to_string();
                debug_print_va("didn't find tlresource %s in system directory", v3);
            }
        }

        if ( v5 == nullptr ) {
            v5 = default_tlres;
        }

        return v5;
    }
    else
    {
        nglTexture * (__fastcall *func)(void *, void *edx, const tlFixedString *a1) = CAST(func, 0x005691B0);
        return func(this, nullptr, &a1);
    }
}

template<>
nglTexture *tlresource_directory<nglTexture,tlFixedString>::Find(unsigned int a2)
{
    TRACE("tlresource_directory<nglTexture,tlFixedString>::Find(uint32_t )", string_hash {int(a2)}.to_string());

    nglTexture *v3 = nullptr;
    if ( this->field_4 != nullptr ) {
        v3 = (nglTexture *) this->field_4->get_tlresource(a2, TLRESOURCE_TYPE_TEXTURE);
    }

    if ( v3 == nullptr && system_dir != nullptr )
    {
        auto SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString{"SHOW_RESOURCE_SPAM"});

        nglTexture * (__fastcall *find)(void *, void *, uint32_t) = CAST(find, get_vfunc(system_dir->m_vtbl, 0x8));
        v3 = find(system_dir, nullptr, a2);
        if ( v3 != nullptr )
        {
            if ( SHOW_RESOURCE_SPAM )
            {
                debug_print_va("found tlresource %08x in system directory", a2);
            }
        }
        else if ( SHOW_RESOURCE_SPAM )
        {
            debug_print_va("didn't find tlresource %08x in system directory", a2);
        }
    }

    if ( v3 == nullptr ) {
        v3 = default_tlres;
    }

    return v3;
}

template<>
nalAnimClass<nalAnyPose> *tlresource_directory<nalAnimClass<nalAnyPose>, tlFixedString>::Find(
        unsigned int a2)
{
    TRACE("tlresource_directory<nalAnimClass<nalAnyPose>, tlFixedString>::Find");

    if constexpr (0)
    {
        nalAnimClass<nalAnyPose> *tlresource = nullptr;
        if (this->field_4 != nullptr) {
            tlresource = CAST(tlresource, this->field_4->get_tlresource(a2, TLRESOURCE_TYPE_ANIM));
        }

        if ( tlresource == nullptr && system_dir != nullptr )
        {
            nalAnimClass<nalAnyPose> * (__fastcall *find)(void *, void *, uint32_t) = CAST(find, get_vfunc(system_dir->m_vtbl, 0x8));
            tlresource  = find(system_dir, nullptr, a2);

            auto SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString{"SHOW_RESOURCE_SPAM"});

            if ( tlresource != nullptr )
            {
                if ( SHOW_RESOURCE_SPAM ) {
                    debug_print_va("found tlresource %08x in system directory", a2);
                }
            }
            else if ( SHOW_RESOURCE_SPAM )
            {
                debug_print_va("didn't find tlresource %08x in system directory", a2);
            }
        }

        if ( tlresource == nullptr ) {
            return default_tlres;
        }

        return tlresource;
    }
    else
    {
        nalAnimClass<nalAnyPose> *result = CAST(result, THISCALL(0x00566630, this, a2));
        return result;
    }
}

template<>
nglMesh *tlresource_directory<nglMesh, tlHashString>::Find(uint32_t a2)
{
    if constexpr (1)
    {
        assert(((tlresource_type) tlres_type) != TLRESOURCE_TYPE_NONE &&
               "This type must have a matching tlresource type");

        nglMesh *v3 = nullptr;
        if (this->field_4 != nullptr) {
            v3 = (nglMesh *) this->field_4->get_tlresource(a2, TLRESOURCE_TYPE_MESH);
        }

        if (v3 == nullptr && system_dir != nullptr) {
            nglMesh * (__fastcall *Find)(void *, void *, uint32_t) = CAST(Find, get_vfunc(system_dir->m_vtbl, 0x8));
            auto *v3 = Find(system_dir, nullptr, a2);
            if (v3 != nullptr) {
                //if (byte_15B2C1A)
                sp_log("found tlresource %08x in system directory", a2);
            } else
            //if (byte_15B2C1A)
            {
                sp_log("didn't find tlresource %08x in system directory", a2);
            }
        }

        if (v3 == nullptr) {
            v3 = default_tlres;
        }
        return v3;
    }
}

void tlresource_directory_patch()
{
    {
        nglMeshFile * (tlresource_directory<nglMeshFile, tlFixedString>::*func)(const tlFixedString &) = tlresource_directory<nglMeshFile, tlFixedString>::Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00889680, address);
    }

    {
        nglMesh * (tlresource_directory<nglMesh, tlHashString>::*func)(const tlHashString &) = tlresource_directory<nglMesh, tlHashString>::Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008896AC, address);
    }

    {
        void (tlresource_directory<nglMesh, tlHashString>::*func)(nglMesh *) = tlresource_directory<nglMesh, tlHashString>::Add;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008896B0, address);
    }

    {
        nglTexture * (tlresource_directory<nglTexture, tlFixedString>::*func)(uint32_t ) = tlresource_directory<nglTexture, tlFixedString>::Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00889650, address);
    }

    {
        nalAnimClass<nalAnyPose> * (tlresource_directory<nalAnimClass<nalAnyPose>,tlFixedString>::*func)(uint32_t ) = tlresource_directory<nalAnimClass<nalAnyPose>,tlFixedString>::Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008897B0, address);
    }

    {
        nglTexture * (tlresource_directory<nglTexture, tlFixedString>::*func)(const tlFixedString &) = tlresource_directory<nglTexture, tlFixedString>::Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00889654, address);
    }

    {
        nalBaseSkeleton * (tlresource_directory<nalBaseSkeleton, tlFixedString>::*func)(const tlFixedString &) = tlresource_directory<nalBaseSkeleton, tlFixedString>::Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x0088980C, address);
    }
}
