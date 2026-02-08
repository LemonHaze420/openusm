#include "tlresource_directory.h"

#include "func_wrapper.h"
#include "nal_system.h"
#include "ngl.h"
#include "ngl_mesh.h"
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
make_default_tlres(nglMaterialFile, tlFixedString, 0x009609D0);
make_default_tlres(nglMaterialBase, tlHashString, 0x009609CC);

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
make_default_tlres(nglMaterialFile, tlFixedString);
make_default_tlres(nglMaterialBase, tlHashString);

#undef make_default_tlres

#endif

#define make_tlres_type(T0, T1, type)                               \
    template<>                                                      \
    tlresource_type tlresource_directory<T0, T1>::tlres_type = type

make_tlres_type(nglTexture, tlFixedString, TLRESOURCE_TYPE_TEXTURE);
make_tlres_type(nglMeshFile, tlFixedString, TLRESOURCE_TYPE_MESH_FILE);
make_tlres_type(nglMesh, tlHashString, TLRESOURCE_TYPE_MESH);
make_tlres_type(nglMorphFile, tlFixedString, TLRESOURCE_TYPE_MORPH_FILE);
make_tlres_type(nglMorphSet, tlHashString, TLRESOURCE_TYPE_MORPH);
make_tlres_type(nglMaterialFile, tlFixedString, TLRESOURCE_TYPE_MATERIAL_FILE);
make_tlres_type(nglMaterialBase, tlHashString, TLRESOURCE_TYPE_MATERIAL);
make_tlres_type(nalAnimFile, tlFixedString, TLRESOURCE_TYPE_ANIM_FILE);
make_tlres_type(nalAnimClass<nalAnyPose>, tlFixedString, TLRESOURCE_TYPE_ANIM);
make_tlres_type(nalSceneAnim, tlFixedString, TLRESOURCE_TYPE_SCENE_ANIM);
make_tlres_type(nalBaseSkeleton, tlFixedString, TLRESOURCE_TYPE_SKELETON);

#undef make_tlres_type

template<>
void tlresource_directory<nglMesh, tlHashString>::_Add([[maybe_unused]] nglMesh *Mesh)
{
    TRACE("tlresource_directory<nglMesh, tlHashString>::Add");
    ;
}


//0x005691B0
template<>
nglTexture *tlresource_directory<nglTexture, tlFixedString>::_Find(const tlFixedString &a1) 
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

#define constructor_tlresource_directory(T0, T1, vtbl)      \
    template<>                                              \
    tlresource_directory<T0, T1>::tlresource_directory()    \
    {                                                       \
        T0 * (tlresource_directory<T0, T1>::*Find)(unsigned int) = &_Find;  \
                                                                            \
        T0 * (tlresource_directory<T0, T1>::*Find1)(const T1 &) = &_Find;   \
                                                            \
        if constexpr (1) {                                  \
            static void * g_vtbl[] {                        \
                func_address(&finalize),                    \
                func_address(&DirectoryName),                \
                func_address(Find),                          \
                func_address(Find1),                         \
                func_address(&_Add) \
            };                                              \
                                                            \
            this->m_vtbl = CAST(vtbl, &g_vtbl);             \
        } else {                                            \
            this->m_vtbl = vtbl;                            \
        }                                                   \
                                                            \
        this->field_4 = nullptr;                            \
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


void tlresource_directory_patch()
{
    {
        nglMeshFile * (tlresource_directory<nglMeshFile, tlFixedString>::*func)(const tlFixedString &) = tlresource_directory<nglMeshFile, tlFixedString>::_Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00889680, address);
    }

    {
        nglMesh * (tlresource_directory<nglMesh, tlHashString>::*func)(const tlHashString &) = tlresource_directory<nglMesh, tlHashString>::_Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008896AC, address);
    }

    {
        void (tlresource_directory<nglMesh, tlHashString>::*func)(nglMesh *) = tlresource_directory<nglMesh, tlHashString>::_Add;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008896B0, address);
    }

    {
        nglTexture * (tlresource_directory<nglTexture, tlFixedString>::*func)(uint32_t ) = tlresource_directory<nglTexture, tlFixedString>::_Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00889650, address);
    }

    {
        nalAnimClass<nalAnyPose> * (tlresource_directory<nalAnimClass<nalAnyPose>,tlFixedString>::*func)(uint32_t ) = tlresource_directory<nalAnimClass<nalAnyPose>,tlFixedString>::_Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008897B0, address);
    }

    {
        nglTexture * (tlresource_directory<nglTexture, tlFixedString>::*func)(const tlFixedString &) = tlresource_directory<nglTexture, tlFixedString>::_Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00889654, address);
    }

    {
        nalBaseSkeleton * (tlresource_directory<nalBaseSkeleton, tlFixedString>::*func)(const tlFixedString &) = tlresource_directory<nalBaseSkeleton, tlFixedString>::_Find;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x0088980C, address);
    }
}
