#include "variant_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "tl_system.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(variant_interface, 0x58);

variant_interface::variant_interface(conglomerate *a2) : conglomerate_interface(a2)
{
    this->m_vtbl = 0x008835A0;
}

variant_info *variant_interface::get_random_variant()
{
    if constexpr (0)
    {
    }
    else
    {
        return (variant_info *) THISCALL(0x004CAD00, this);
    }
}

void variant_interface::apply_variant(string_hash a2)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (0)
    {}
    else
    {
        THISCALL(0x004E0920, this, a2);
    }
}

void variant_interface::apply_variant(variant_info *info)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (0)
    {}
    else
    {
        THISCALL(0x004DB110, this, info);
    }
}

void variant_interface::release_ifc()
{
    if ( this->current_mesh != nullptr )
    {
        this->destroy_mesh_concatenation(this->current_mesh);
        this->destroy_ifl_frames();
        this->current_mesh = nullptr;
        this->current_variant = nullptr;
    }

    if ( this->field_24 != nullptr )
    {
        this->destroy_morph_concatenation(this->field_24);
        this->field_24 = nullptr;
    }
}

void variant_interface::destroy_ifl_frames()
{
    auto **v1 = this->field_2C;
    int v2 = 3;
    do
    {
        auto *v3 = *v1;
        if ( v3 != nullptr )
        {
            mem_dealloc(v3, 1);

        }

        *v1++ = nullptr;
        --v2;
    }
    while ( v2 != 0 );
}

void variant_interface::destroy_mesh_concatenation(nglMesh *mesh)
{
    assert(mesh != nullptr);

    for ( int i = 0; i < mesh->NLODs; ++i ) {
        assert(mesh->LODs != nullptr);
        this->destroy_mesh_concatenation(mesh->LODs[i].field_0);
    }

    if ( mesh->LODs != nullptr )
    {
        assert(mesh->NLODs > 0);

        tlMemFree(mesh->LODs);
    }

    tlMemFree(mesh->Sections);
    tlMemFree(mesh);
}

void variant_interface::destroy_morph_concatenation(nglMorphSet *a1)
{
    for ( int i = 0; i < a1->NFrames; ++i ) {
        tlMemFree(a1->Frames[i].field_8);
    }

    tlMemFree(a1->Frames);
    tlMemFree(a1);
}

void variant_interface_patch()
{
    {
        void (variant_interface::* apply_variant)(string_hash) = &variant_interface::apply_variant;
        FUNC_ADDRESS(address, apply_variant);
        REDIRECT(0x0066F402, address);
        REDIRECT(0x0066F372, address);
    }
}
