#include "variant_interface.h"

#include "common.h"
#include "conglom.h"
#include "entity_mash.h"
#include "func_wrapper.h"
#include "memory.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "parse_generic_mash.h"
#include "resource_manager.h"
#include "tl_system.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(variant_interface, 0x58);
VALIDATE_SIZE(variant_info, 0x10);
VALIDATE_SIZE(variant_speaker_id_set, 0xC);
VALIDATE_OFFSET(variant_speaker_id_set, id_count, 0x2);
VALIDATE_OFFSET(variant_speaker_id_set, ids, 0x8);

namespace
{
void unmash_variant_record(variant_info &record, generic_mash_data_ptrs *data)
{
    data->rebase(4);
    record.parts = data->get<uint8_t>(record.field_4);
    data->rebase(4);
    record.ifl_frames = data->get<char>(record.field_4);
}

void unmash_variant_record(variant_speaker_id_set &record, generic_mash_data_ptrs *data)
{
    data->rebase(4);
    record.ids = data->get<uint32_t>(record.id_count);
}

template<typename T>
void unmash_variant_vector(mashable_vector<T> &vector, generic_mash_data_ptrs *data)
{
    if (vector.is_shared()) {
        data->rebase_shared(4);

        const auto normal_bytes = *data->get_from_shared<uint32_t>();
        const auto shared_bytes = *data->get_from_shared<uint32_t>();
        auto *references = data->get_from_shared<uint32_t>();
        data->rebase_shared(4);
        data->rebase_shared(4);
        vector.m_data = data->get_from_shared<T>(vector.m_size);
        if (*references != 0) {
            data->get<char>(normal_bytes);
            data->get_from_shared<char>(shared_bytes - sizeof(T) * vector.m_size);
        } else {
            for (auto &record : vector)
                unmash_variant_record(record, data);
        }
        ++*references;
        data->rebase_shared(4);
    } else {
        data->rebase(4);
        data->rebase(4);
        vector.m_data = data->get<T>(vector.m_size);
        for (auto &record : vector)
            unmash_variant_record(record, data);
        data->rebase(4);
    }
}
}

variant_interface::variant_interface(conglomerate *a2) : conglomerate_interface(a2)
{
#if STANDALONE_SYSTEM
    construct_v_table_lookup();
    m_vtbl = ifc_v_table_lookup[10];
#else
    this->m_vtbl = 0x008835A0;
#endif
}

variant_interface::~variant_interface()
{
    if (!field_14.from_mash()) {
        for (auto &record : field_14)
            delete[] record.ids;
        delete[] field_14.m_data;
    }
    field_14.m_data = nullptr;
    field_14.m_size = 0;
    if (!variants.from_mash()) {
        for (auto &record : variants) {
            delete[] record.parts;
            delete[] record.ifl_frames;
        }
        delete[] variants.m_data;
    }
    variants.m_data = nullptr;
    variants.m_size = 0;
    my_conglomerate = nullptr;
}

void variant_interface::_un_mash(generic_mash_header *, void *owner, void *,
                                generic_mash_data_ptrs *data)
{
    my_conglomerate = static_cast<conglomerate *>(owner);
    dynamic = false;
    unmash_variant_vector(variants, data);
    unmash_variant_vector(field_14, data);
}

variant_info *variant_interface::get_random_variant()
{
    if constexpr (0) {
    } else {
        return (variant_info *)THISCALL(0x004CAD00, this);
    }
}

void variant_interface::apply_variant(string_hash a2)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (0) {
    } else {
        THISCALL(0x004E0920, this, a2);
    }
}

void variant_interface::apply_variant(variant_info *info)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (0) {
    } else {
        THISCALL(0x004DB110, this, info);
    }
}

void variant_interface::release_ifc()
{
    if (this->current_mesh != nullptr) {
        this->destroy_mesh_concatenation(this->current_mesh);
        this->destroy_ifl_frames();
        this->current_mesh = nullptr;
        this->current_variant = nullptr;
    }

    if (this->field_24 != nullptr) {
        this->destroy_morph_concatenation(this->field_24);
        this->field_24 = nullptr;
    }
}

void variant_interface::destroy_ifl_frames()
{
    auto **v1 = this->field_2C;
    int v2 = 3;
    do {
        auto *v3 = *v1;
        if (v3 != nullptr) {
            mem_dealloc(v3, 1);
        }

        *v1++ = nullptr;
        --v2;
    } while (v2 != 0);
}

void variant_interface::destroy_mesh_concatenation(nglMesh *mesh)
{
    assert(mesh != nullptr);

    for (int i = 0; i < mesh->NLODs; ++i) {
        assert(mesh->LODs != nullptr);
        this->destroy_mesh_concatenation(mesh->LODs[i].field_0);
    }

    if (mesh->LODs != nullptr) {
        assert(mesh->NLODs > 0);

        tlMemFree(mesh->LODs);
    }

    tlMemFree(mesh->Sections);
    tlMemFree(mesh);
}

void variant_interface::destroy_morph_concatenation(nglMorphSet *a1)
{
    for (int i = 0; i < a1->NFrames; ++i) {
        tlMemFree(a1->Frames[i].field_8);
    }

    tlMemFree(a1->Frames);
    tlMemFree(a1);
}

nglMorphSet *variant_interface::create_morph_concatenation(nglMorphSet **parts, int count, nglMorphSet *source)
{
    auto *result = static_cast<nglMorphSet *>(tlMemAlloc(sizeof(nglMorphSet), 8, 0));
    result->field_0 = source->field_0;
    result->NFrames = source->NFrames;
    result->field_C = source->field_C;
    result->NextMorph = nullptr;
    result->Frames = static_cast<decltype(result->Frames)>(
        tlMemAlloc(sizeof(*result->Frames) * result->NFrames, 8, 0));

    int section_count = 0;
    for (int part = 0; part < count; ++part)
        section_count += parts[part]->Frames[0].field_4;

    for (int frame = 0; frame < result->NFrames; ++frame) {
        auto &destination = result->Frames[frame];
        destination.field_0 = source->Frames[frame].field_0;
        destination.field_4 = section_count;
        destination.field_8 = static_cast<decltype(destination.field_8)>(
            tlMemAlloc(sizeof(*destination.field_8) * section_count, 8, 0));
        int output_section = 0;
        for (int part = 0; part < count; ++part) {
            const auto &input = parts[part]->Frames[frame];
            for (int section = 0; section < input.field_4; ++section)
                destination.field_8[output_section++] = input.field_8[section];
        }
    }
    return result;
}

nglMorphSet *variant_interface::get_morph(const tlFixedString &name)
{
    if (current_variant == nullptr)
        return nullptr;
    mString lookup_name{name.c_str()};
    lookup_name.append("000");
    resource_manager::push_resource_context(my_conglomerate->get_resource_context());
    auto *source = nglGetMorph(tlFixedString{lookup_name.c_str()}, true);
    resource_manager::pop_resource_context();
    if (source == nullptr)
        return nullptr;

    if (field_24 != nullptr && !(source->field_0 == field_24->field_0)) {
        destroy_morph_concatenation(field_24);
        field_24 = nullptr;
    }
    if (field_24 == nullptr) {
        nglMorphSet *parts[32];
        auto *selected = reinterpret_cast<const uint8_t *>(&field_38);
        int count = 0;
        while (*selected != 0xFF) {
            auto *part = source->field_C->FirstMorph;
            for (int index = 0; index < *selected; ++index)
                part = part->NextMorph;
            parts[count++] = part;
            ++selected;
        }
        field_24 = create_morph_concatenation(parts, current_variant->field_4, source);
    }
    return field_24;
}

void variant_interface_patch()
{
    {
        void (variant_interface::*apply_variant)(string_hash) = &variant_interface::apply_variant;
        FUNC_ADDRESS(address, apply_variant);
        REDIRECT(0x0066F402, address);
        REDIRECT(0x0066F372, address);
    }
}
