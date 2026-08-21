#include "eligible_pack_streamer.h"

#include "binary_search_array_cmp.h"
#include "common.h"
#include "eligible_pack.h"
#include "eligible_pack_category.h"
#include "eligible_pack_token.h"
#include "func_wrapper.h"
#include "memory.h"
#include "resource_pack_streamer.h"
#include "resource_partition.h"
#include "terrain.h"
#include "utility.h"

#include <cassert>

VALIDATE_SIZE(eligible_pack_streamer, 0x38);

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

        void(__fastcall * reserve)(void *, void *, int) = CAST(reserve, 0x0056E660);

        reserve(&this->field_14, nullptr, num_streamers);
        {
            for (int i = 0; i < num_streamers; ++i) {
                eligible_pack_category *the_category = new eligible_pack_category{this, streamers[i], callbacks[i]};

                if (this->field_14.size() < this->field_14.capacity()) {
                    auto v13 = this->field_14.m_last;
                    *v13 = the_category;
                    this->field_14.m_last = v13 + 1;
                } else {
                    void(__fastcall * _Insert_n)(void *, void *, void *, int, void *) = CAST(_Insert_n, 0x0056A260);

                    _Insert_n(&this->field_14, nullptr, this->field_14.m_last, 1, &the_category);
                }
            };
        }

    } else {
        THISCALL(0x00547C50, this, a2, num_streamers, streamers, callbacks, get_ideal_pack_info);
    }
}

void eligible_pack_streamer::clear()
{
    THISCALL(0x00547BA0, this);
}

int compare_eligible_pack_names(const void *a1, const void *a2)
{
    if constexpr (0) {
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
    if constexpr (0) {
        eligible_pack_category *the_category = nullptr;

        for (auto &pack_cat : this->field_14) {
            if (pack_cat->my_resource_pack_streamer == a4) {
                the_category = pack_cat;
                break;
            }
        }

        assert(the_category != nullptr);

        auto *mem = mem_alloc(sizeof(eligible_pack));
        eligible_pack *v9 = new (mem) eligible_pack{a2, a3, the_category};

        if (this->eligible_packs.size() < this->eligible_packs.capacity()) {
            auto *v12 = this->eligible_packs.m_last;
            *v12 = v9;
            this->eligible_packs.m_last = v12 + 1;
        } else {
            void(__fastcall * _Insert_n)(void *, void *, void *, int, void *) = CAST(_Insert_n, 0x0056A260);

            _Insert_n(&this->eligible_packs, nullptr, this->eligible_packs.m_last, 1, &v9);
        }

        auto v14 = this->eligible_packs.size();
        qsort(this->eligible_packs.m_first, v14, 4u, compare_eligible_pack_names);
        return v9;
    } else {
        return (eligible_pack *)THISCALL(0x00551290, this, a2, &a3, a4);
    }
}

void eligible_pack_streamer::prioritize()
{
    THISCALL(0x00547D80, this);
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
