#include "variant_interface.h"

#include "common.h"
#include "base_ai_core.h"
#include "ai_voice_box_inode.h"
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
#include <cstdlib>
#include <new>

VALIDATE_SIZE(variant_interface, 0x58);
VALIDATE_SIZE(variant_info, 0x10);
VALIDATE_SIZE(variant_speaker_id_set, 0xC);
VALIDATE_OFFSET(variant_speaker_id_set, id_count, 0x2);
VALIDATE_OFFSET(variant_speaker_id_set, ids, 0x8);

namespace {
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

template <typename T>
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
}  // namespace

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

void variant_interface::_un_mash(generic_mash_header *, void *owner, void *, generic_mash_data_ptrs *data)
{
    my_conglomerate = static_cast<conglomerate *>(owner);
    dynamic = false;
    unmash_variant_vector(variants, data);
    unmash_variant_vector(field_14, data);
}

variant_info *variant_interface::get_random_variant()
{
    if constexpr (STANDALONE_SYSTEM) {
        int total = 0;
        for (const auto &info : variants)
            total += info.field_6;
        const int choice = total > 1 ? static_cast<int>(std::rand() * (total - 1) * (1.0 / 32768.0)) : 0;
        int first = 0;
        for (auto &info : variants) {
            if (choice >= first && choice < first + info.field_6)
                return &info;
            first += info.field_6;
        }
        return variants.m_data;
    } else {
        return (variant_info *)THISCALL(0x004CAD00, this);
    }
}

void variant_interface::apply_variant(string_hash a2)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (STANDALONE_SYSTEM) {
        for (auto &info : variants) {
            if (info.hash == a2) {
                apply_variant(&info);
                return;
            }
        }
    } else {
        THISCALL(0x004E0920, this, a2);
    }
}

void variant_interface::apply_variant(variant_info *info)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (STANDALONE_SYSTEM) {
        if (my_conglomerate->field_90.field_5 > 1)
            return;
        release_ifc();
        nglMesh *parts[32];
        auto *selected = reinterpret_cast<uint8_t *>(&field_38);
        int count = 0;
        for (int first = 0; first < info->field_4;) {
            int end = first + 1;
            while (end < info->field_4 && (info->parts[end] & 0x80) == 0)
                ++end;
            const int choices = end - first;
            const int choice = choices > 0 ? static_cast<int>(std::rand() * choices * (1.0 / 32768.0)) : 0;
            const auto index = info->parts[first + choice] & 0x7F;
            auto *mesh = field_28->FirstMesh;
            for (int i = 0; i < index; ++i)
                mesh = mesh->NextMesh;
            selected[count] = index;
            parts[count++] = mesh;
            first = end;
        }
        selected[count] = 0xFF;
        const char random_frame = static_cast<char>(std::rand() * (127.0 / 32768.0));
        current_variant = info;
        current_mesh = create_mesh_concatenation_and_ifl_frames(parts, count, info, random_frame, 0, nullptr);
        my_conglomerate->field_90.set_mesh(current_mesh);
        auto *core = my_conglomerate->get_ai_core();
        if (core == nullptr)
            return;
        auto *voice = static_cast<ai::voice_box_inode *>(core->get_info_node(ai::voice_box_inode::default_id, false));
        if (voice == nullptr)
            return;
        static const string_hash speaker_id{"speaker_id"};
        for (int part = 0; part < info->field_4; ++part) {
            const auto mesh_index = info->parts[part] & 0x7F;
            int maximum_frame = -1;
            for (const auto &set : field_14) {
                if (set.field_0 == mesh_index)
                    maximum_frame = std::max(maximum_frame, int(static_cast<int16_t>(set.field_4 >> 16)));
            }
            int frame = info->ifl_frames[part];
            if (frame == -1 && maximum_frame != -1)
                frame = static_cast<unsigned char>(random_frame) % (maximum_frame + 1);
            for (const auto &set : field_14) {
                const int first_frame = static_cast<int16_t>(set.field_4);
                const int last_frame = static_cast<int16_t>(set.field_4 >> 16);
                if (set.field_0 == mesh_index && ((first_frame == -1 && last_frame == -1) ||
                                                  (frame >= 0 && first_frame <= frame && last_frame >= frame))) {
                    const int index =
                        set.id_count > 0 ? static_cast<int>(std::rand() * set.id_count * (1.0 / 32768.0)) : 0;
                    voice->my_param_block.set_pb_hash(speaker_id, string_hash{static_cast<int>(set.ids[index])}, true);
                    return;
                }
            }
        }
    } else {
        THISCALL(0x004DB110, this, info);
    }
}

nglMesh *variant_interface::create_mesh_concatenation_and_ifl_frames(nglMesh **parts, int count, variant_info *info,
                                                                     char random_frame, int lod, nglMesh *source)
{
    if (source == nullptr)
        source = parts[0]->File->FirstMesh;
    auto *result = new (tlMemAlloc(sizeof(nglMesh), 16, 0)) nglMesh{};
    result->Name = source->Name;
    result->Flags = (source->Flags & 0xFF000000u) | 0x90000u;
    for (int part = 0; part < count; ++part) {
        result->NSections += parts[part]->NSections;
        result->DataSize += parts[part]->DataSize;
    }
    field_2C[lod] = static_cast<char *>(mem_alloc(result->NSections));
    result->Sections =
        static_cast<decltype(result->Sections)>(tlMemAlloc(sizeof(*result->Sections) * result->NSections, 8, 0));
    int output = 0;
    for (int part = 0; part < count; ++part) {
        for (unsigned section = 0; section < parts[part]->NSections; ++section) {
            result->Sections[output].field_0 = 0;
            result->Sections[output].Section = parts[part]->Sections[section].Section;
            field_2C[lod][output++] = info->ifl_frames[part] == -1 ? random_frame : info->ifl_frames[part];
        }
    }
    result->SphereCenter = source->SphereCenter;
    result->SphereRadius = source->SphereRadius;
    result->NBones = source->NBones;
    result->Bones = source->Bones;
    result->File = source->File;
    if (source->NLODs != 0) {
        result->LODs = static_cast<nglMesh::Lod *>(tlMemAlloc(sizeof(*result->LODs) * source->NLODs, 8, 0));
        for (int level = 0; level < source->NLODs; ++level) {
            nglMesh *lod_parts[32];
            for (int part = 0; part < count; ++part)
                lod_parts[part] = parts[part]->LODs[level].field_0;
            result->LODs[level].field_4 = source->LODs[level].field_4;
            result->LODs[level].field_0 =
                create_mesh_concatenation_and_ifl_frames(lod_parts, count, info, random_frame, level + 1, result);
        }
    }
    result->NLODs = source->NLODs;
    return result;
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
    result->Name = source->Name;
    result->NFrames = source->NFrames;
    result->field_C = source->field_C;
    result->NextMorph = nullptr;
    result->Frames = static_cast<decltype(result->Frames)>(tlMemAlloc(sizeof(*result->Frames) * result->NFrames, 8, 0));

    int section_count = 0;
    for (int part = 0; part < count; ++part)
        section_count += parts[part]->Frames[0].field_4;

    for (int frame = 0; frame < result->NFrames; ++frame) {
        auto &destination = result->Frames[frame];
        destination.field_0 = source->Frames[frame].field_0;
        destination.field_4 = section_count;
        destination.field_8 =
            static_cast<decltype(destination.field_8)>(tlMemAlloc(sizeof(*destination.field_8) * section_count, 8, 0));
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

    if (field_24 != nullptr && !(*source->Name == *field_24->Name)) {
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
