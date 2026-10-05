#include "ai_cover_marker.h"

#include "common.h"
#include "entity_mash.h"
#include "memory.h"

#include <algorithm>
#include <array>

VALIDATE_SIZE(ai_cover_marker_list, 0xC);
VALIDATE_SIZE(ai_cover_marker, 0x78);
VALIDATE_OFFSET(ai_cover_marker, next_cover_marker, 0x68);
VALIDATE_OFFSET(ai_cover_marker, previous_cover_marker, 0x6C);
VALIDATE_OFFSET(ai_cover_marker, cover_marker_list, 0x70);
VALIDATE_OFFSET(ai_cover_marker, cover_id, 0x74);

Var<ai_cover_marker_list> ai_cover_marker::all_cover_markers{0x0095ACB8};

namespace {
void *__fastcall cover_marker_delete(ai_cover_marker *self, void *, unsigned flags)
{
    self->~ai_cover_marker();
    if (flags & 1)
        mem_dealloc(self, sizeof(ai_cover_marker));
    return self;
}
int __fastcall cover_marker_size(ai_cover_marker *, void *)
{
    return sizeof(ai_cover_marker);
}
int __fastcall cover_marker_flavor(ai_cover_marker *, void *)
{
    return 27;
}
bool __fastcall cover_marker_query(ai_cover_marker *, void *)
{
    return true;
}
void __fastcall cover_marker_release(ai_cover_marker *self, void *)
{
    self->release_mem();
}
void __fastcall cover_marker_unmash(ai_cover_marker *self, void *, generic_mash_header *header, void *object,
                                    generic_mash_data_ptrs *data)
{
    self->un_mash(header, object, data);
}
}  // namespace

void *ai_cover_marker::native_vtable(void **entity_table)
{
    static std::array<void *, 0x20C / 4> table;
    std::copy_n(entity_table, table.size(), table.begin());
    table[0] = reinterpret_cast<void *>(&cover_marker_delete);
    table[0x4 / 4] = reinterpret_cast<void *>(&cover_marker_size);
    table[0x10 / 4] = reinterpret_cast<void *>(&cover_marker_release);
    table[0x54 / 4] = reinterpret_cast<void *>(&cover_marker_flavor);
    table[0x98 / 4] = reinterpret_cast<void *>(&cover_marker_query);
    table[0xC0 / 4] = reinterpret_cast<void *>(&cover_marker_query);
    table[0x164 / 4] = reinterpret_cast<void *>(&cover_marker_unmash);
    return table.data();
}

ai_cover_marker::ai_cover_marker(const string_hash &id, uint32_t flags) : marker(id, flags)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[27];
#else
    m_vtbl = 0x00884BB0;
#endif
    add_to_cover_list();
    cover_id.source_hash_code = 0;
}

ai_cover_marker::~ai_cover_marker()
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[27];
#else
    m_vtbl = 0x00884BB0;
#endif
    remove_from_cover_list();
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[16];
#else
    m_vtbl = 0x008849A0;
#endif
}

void ai_cover_marker::add_to_cover_list()
{
    auto &list = all_cover_markers();
    next_cover_marker = nullptr;
    previous_cover_marker = nullptr;
    cover_marker_list = nullptr;
    if (list.tail != nullptr) {
        list.tail->next_cover_marker = this;
        previous_cover_marker = list.tail;
    } else {
        next_cover_marker = list.head;
        if (list.head != nullptr)
            list.head->previous_cover_marker = this;
        list.head = this;
    }
    if (next_cover_marker == nullptr)
        list.tail = this;
    cover_marker_list = &list;
    ++list.size;
}

void ai_cover_marker::remove_from_cover_list()
{
    if (cover_marker_list != &all_cover_markers())
        return;
    if (previous_cover_marker != nullptr)
        previous_cover_marker->next_cover_marker = next_cover_marker;
    else
        cover_marker_list->head = next_cover_marker;
    if (next_cover_marker != nullptr)
        next_cover_marker->previous_cover_marker = previous_cover_marker;
    else
        cover_marker_list->tail = previous_cover_marker;
    --cover_marker_list->size;
    next_cover_marker = nullptr;
    previous_cover_marker = nullptr;
    cover_marker_list = nullptr;
}

void ai_cover_marker::release_mem()
{
    remove_from_cover_list();
    entity::release_mem();
}

void ai_cover_marker::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    entity::un_mash(header, object, data);
    add_to_cover_list();
    set_active(false);
}
