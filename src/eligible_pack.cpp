#include "eligible_pack.h"

#include "common.h"
#include "func_wrapper.h"
#include "resource_pack_location.h"
#include "resource_manager.h"
#include "eligible_pack_streamer.h"
#include "eligible_pack_category.h"
#include "trace.h"

VALIDATE_SIZE(eligible_pack, 0x74);
VALIDATE_OFFSET(eligible_pack, field_54, 0x54);

eligible_pack::eligible_pack(const char *name, const eligible_pack_token &token, eligible_pack_category *category)
    : field_0(name), field_20(name), field_40(name != nullptr ? string_hash{name} : string_hash{}),
      field_44(name != nullptr ? string_hash{name} : string_hash{}), field_48(token), field_50(category),
      field_6C(3.4028235e38f), field_70(0)
{}

void eligible_pack::set_packfile_name(const char *a2)
{
    this->field_20 = fixedstring<8>{a2};
    this->field_44 = string_hash{a2};
}

resource_pack_slot *eligible_pack::get_resource_pack_slot()
{
    auto *v1 = this->field_50->my_resource_pack_streamer;
    auto *pack_slots = v1->get_pack_slots();
    assert(pack_slots != nullptr);

    for (auto &slot : (*pack_slots)) {
        assert(slot != nullptr);

        bool v5 = false;
        if (!slot->is_empty()) {
            auto v6 = slot->get_name_key().m_hash;
            if (v6 == this->field_44) {
                v5 = true;
            }
        }

        if (v5) {
            return slot;
        }
    }

    return nullptr;
}

bool eligible_pack::is_loadable()
{
    TRACE("eligible_pack::is_loadable");

    for (auto &e_pack : this->field_54) {
        auto *v10 = e_pack->get_resource_pack_slot();
        if (v10 == nullptr || !v10->is_pack_ready()) {
            return false;
        }
    }

    resource_key key{this->field_44, RESOURCE_KEY_TYPE_PACK};
    return resource_manager::get_pack_file_stats(key, nullptr, nullptr, nullptr);
}

void eligible_pack::fixup_family(eligible_pack_streamer *a2)
{
    TRACE("eligible_pack::fixup_family");

    if constexpr (1) {
        resource_pack_location v12{};
        resource_key v11{this->field_44, resource_key_type::RESOURCE_KEY_TYPE_PACK};

        if (resource_manager::get_pack_file_stats(v11, &v12, nullptr, nullptr)) {
            for (auto i = 0; i < v12.prerequisite_count; ++i) {
                auto v11 = resource_manager::amalgapak_prerequisite_table[i + v12.prerequisite_offset].m_hash;
                auto *v8 = a2->find_eligible_pack_by_packfile_name_hash(v11);

                if (v8 != nullptr) {
                    this->add_parent(v8);
                    v8->add_child(this);
                }
            }
        }
    } else {
        THISCALL(0x00542FC0, this, a2);
    }
}

bool eligible_pack::sub_5321C0()
{
    for (auto &ep : this->field_60) {
        if (ep->get_resource_pack_slot() != nullptr) {
            return false;
        }
    }

    return true;
}

int eligible_pack::add_parent(eligible_pack *parent)
{
#if STANDALONE_SYSTEM
    if (std::find(this->field_54.begin(), this->field_54.end(), parent) == this->field_54.end()) {
        this->field_54.push_back(parent);
        return 1;
    }
    return 0;
#else
    return THISCALL(0x0053E5A0, this, parent);
#endif
}

int eligible_pack::add_child(eligible_pack *child)
{
#if STANDALONE_SYSTEM
    if (std::find(this->field_60.begin(), this->field_60.end(), child) == this->field_60.end()) {
        this->field_60.push_back(child);
        return 1;
    }
    return 0;
#else
    return THISCALL(0x0053E5F0, this, child);
#endif
}

void eligible_pack_patch()
{
    {
        FUNC_ADDRESS(address, &eligible_pack::fixup_family);
        SET_JUMP(0x00542FC0, address);
    }

    {
        FUNC_ADDRESS(address, &eligible_pack::is_loadable);
        REDIRECT(0x00551405, address);
    }
}
