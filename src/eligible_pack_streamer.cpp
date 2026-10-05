#include "eligible_pack_streamer.h"

#include "binary_search_array_cmp.h"
#include "common.h"
#include "eligible_pack.h"
#include "eligible_pack_category.h"
#include "eligible_pack_token.h"
#include "ideal_pack_info.h"
#include "func_wrapper.h"
#include "memory.h"
#include "resource_pack_streamer.h"
#include "resource_partition.h"
#include "terrain.h"
#include "utility.h"

#include <cassert>
#include <algorithm>

VALIDATE_SIZE(eligible_pack_streamer, 0x38);

eligible_pack_streamer::~eligible_pack_streamer()
{
    clear();
}

void eligible_pack_streamer::init(int a2, int num_streamers, resource_pack_streamer **streamers,
                                  bool(__cdecl **callbacks)(resource_pack_slot::callback_enum, resource_pack_streamer *,
                                                            resource_pack_slot *, limited_timer *),
                                  void(__cdecl *get_ideal_pack_info)(_std::vector<ideal_pack_info> *))
{
    //
    if constexpr (1) {
        this->clear();

        assert(get_ideal_pack_info != nullptr);

        this->get_ideal_pack_info_callback = get_ideal_pack_info;

        assert(num_streamers > 0);
        assert(streamers != nullptr);

        this->eligible_packs.reserve(a2);
        this->field_14.reserve(num_streamers);
        {
            for (int i = 0; i < num_streamers; ++i) {
                eligible_pack_category *the_category = new eligible_pack_category{this, streamers[i], callbacks[i]};

                if (this->field_14.size() < this->field_14.capacity()) {
                    auto v13 = this->field_14.m_last;
                    *v13 = the_category;
                    this->field_14.m_last = v13 + 1;
                } else {
                    this->field_14.push_back(the_category);
                }
            };
        }

    } else {
        THISCALL(0x00547C50, this, a2, num_streamers, streamers, callbacks, get_ideal_pack_info);
    }
}

void eligible_pack_streamer::clear()
{
    this->field_0 = false;
    for (auto *pack : this->eligible_packs) {
        if (pack != nullptr) {
            pack->~eligible_pack();
            mem_dealloc(pack, sizeof(eligible_pack));
        }
    }
    _std::vector<eligible_pack *>{}.swap(this->eligible_packs);

    for (auto *category : this->field_14) {
        delete category;
    }
    _std::vector<eligible_pack_category *>{}.swap(this->field_14);
}

int compare_eligible_pack_names(const void *a1, const void *a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *lhs = *static_cast<eligible_pack *const *>(a1);
        auto *rhs = *static_cast<eligible_pack *const *>(a2);
        const auto lhs_name = lhs->field_40.source_hash_code;
        const auto rhs_name = rhs->field_40.source_hash_code;

        if (lhs_name > rhs_name) {
            return 1;
        }

        return -(lhs_name < rhs_name);
    } else {
        return CDECL_CALL(0x0050EC30, a1, a2);
    }
}

void eligible_pack_streamer::unlock_pack_slot(resource_pack_slot *slot)
{
    for (auto &v6 : this->field_14) {
        auto *v3 = v6->get_streamer();
        auto *partition = slot->get_partition();
        if (v3 == partition->get_streamer()) {
            v6->unlock_pack_slot(slot);
        }
    }
}

eligible_pack *eligible_pack_streamer::find_eligible_pack_by_packfile_name_hash(string_hash a2)
{
    if constexpr (1) {
        for (auto &ep : this->eligible_packs) {
            if (a2 == ep->field_44) {
                return ep;
            }
        }

        return nullptr;
    } else {
        assert(0);
    }
}

int compare_name_to_eligible_pack_name(string_hash &a1, eligible_pack *&a2)
{
    string_hash v13 = a1;
    auto *v12 = a2;
    auto name_hash = v12->get_name_hash();
    auto v5 = (v13 > name_hash);
    if (v5) {
        return 1;
    } else {
        auto v4 = v12->get_name_hash();
        auto v8 = (v13 < v4);
        if (v8) {
            return -1;
        } else {
            return 0;
        }
    }
}

eligible_pack *eligible_pack_streamer::find_eligible_pack_by_name(string_hash a2)
{
    int index = -1;
    auto size = this->eligible_packs.size();
    auto **v2 = &this->eligible_packs[0];
    if (binary_search_array_cmp<string_hash, eligible_pack *>(
            &a2, v2, 0, size, &index, compare_name_to_eligible_pack_name)) {
        assert(index >= 0 && index < (int)eligible_packs.size());

        return this->eligible_packs[index];
    } else {
        return nullptr;
    }
}

void eligible_pack_streamer::fixup_eligible_pack_parent_child_relationships()
{
    for (auto &pack : this->eligible_packs) {
        pack->fixup_family(this);
    }
}

eligible_pack *eligible_pack_streamer::add_eligible_pack(const char *a2, const eligible_pack_token &a3,
                                                         resource_pack_streamer *a4)
{
    if constexpr (STANDALONE_SYSTEM) {
        eligible_pack_category *the_category = nullptr;
        for (auto *pack_category : this->field_14) {
            if (pack_category->my_resource_pack_streamer == a4) {
                the_category = pack_category;
                break;
            }
        }
        assert(the_category != nullptr);

        auto *mem = mem_alloc(sizeof(eligible_pack));
        auto *pack = new (mem) eligible_pack{a2, a3, the_category};
        this->eligible_packs.push_back(pack);
        qsort(this->eligible_packs.m_first,
              this->eligible_packs.size(),
              sizeof(eligible_pack *),
              compare_eligible_pack_names);
        return pack;
    } else {
        return (eligible_pack *)THISCALL(0x00551290, this, a2, &a3, a4);
    }
}

void eligible_pack_streamer::prioritize()
{
    if (!field_0) {
        return;
    }

    _std::vector<ideal_pack_info> ideal_packs;
    get_ideal_pack_info_callback(&ideal_packs);

    _std::vector<ideal_pack_info *> sorted_packs;
    sorted_packs.reserve(ideal_packs.size());
    for (auto &pack : ideal_packs) {
        sorted_packs.push_back(&pack);
    }
    std::sort(sorted_packs.begin(), sorted_packs.end(), [](const ideal_pack_info *lhs, const ideal_pack_info *rhs) {
        return lhs->field_4 < rhs->field_4;
    });

    for (auto *category : field_14) {
        category->prioritize(sorted_packs);
    }
}

void eligible_pack_streamer::frame_advance(Float a2)
{
    if (this->field_0) {
        for (auto &ep_cat : this->field_14) {
            ep_cat->frame_advance(a2);
        }
    }
}

bool eligible_pack_streamer::is_idle() const
{
    if (!this->field_0) {
        return true;
    }

    for (auto &v2 : this->field_14) {
        if (!v2->my_resource_pack_streamer->is_idle()) {
            return false;
        }
    }

    return true;
}

bool eligible_pack_streamer::is_pack_slot_locked(resource_pack_slot *slot) const
{
    for (auto &v7 : this->field_14) {
        auto *streamer = v7->get_streamer();
        auto *partition = slot->get_partition();
        if (streamer == partition->get_streamer()) {
            return v7->is_pack_slot_locked(slot);
        }
    }

    assert(0);
    return false;
}

eligible_pack *eligible_pack_streamer::find_eligible_pack_by_token(resource_pack_token &a2) const
{
    auto *ep = bit_cast<eligible_pack *>(a2.field_0);
    assert(!this->eligible_packs.empty());

    return ep;
}

pack_switch_info_t *eligible_pack_streamer::get_pack_switch_info(string_hash a2)
{
    if (!this->field_28.empty()) {
        auto *ep = this->find_eligible_pack_by_name(a2);
        if (ep != nullptr) {
            for (auto &v1 : this->field_28) {
                if (v1.field_0 == ep) {
                    return (&v1);
                }
            }
        }
    }

    return nullptr;
}

resource_pack_slot *eligible_pack_streamer::get_eligible_pack_slot(eligible_pack *e_pack)
{
    assert(e_pack != nullptr);
    return e_pack->get_resource_pack_slot();
}

void eligible_pack_streamer_patch()
{
    {
        FUNC_ADDRESS(address, &eligible_pack_streamer::init);
        REDIRECT(0x0055C4A5, address);
    }
}
