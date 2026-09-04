#include "eligible_pack_category.h"

#include "common.h"
#include "eligible_pack.h"
#include "eligible_pack_streamer.h"
#include "func_wrapper.h"
#include "resource_pack_streamer.h"

VALIDATE_SIZE(eligible_pack_category, 0x28);

eligible_pack_category::eligible_pack_category(eligible_pack_streamer *eligible_streamer,
                                               resource_pack_streamer *resource_streamer,
                                               bool(__cdecl *a4)(resource_pack_slot::callback_enum,
                                                                 resource_pack_streamer *, resource_pack_slot *,
                                                                 limited_timer *))
{
    if constexpr (0) {
        assert(eligible_streamer != nullptr && resource_streamer != nullptr);

        this->my_eligible_pack_streamer = eligible_streamer;
        this->my_resource_pack_streamer = resource_streamer;
        this->field_8 = a4;

        auto &pack_slots = *this->my_resource_pack_streamer->get_pack_slots();
        auto v5 = pack_slots.size();
        this->field_C.reserve(v5);
    } else {
        THISCALL(0x00543100, this, eligible_streamer, resource_streamer, a4);
    }
}

eligible_pack_category::~eligible_pack_category()
{
    if constexpr (0) {
        this->clear();
    } else {
        THISCALL(0x0053E640, this);
    }
}

void eligible_pack_category::clear()
{
    this->my_eligible_pack_streamer = nullptr;
    this->my_resource_pack_streamer = nullptr;
    this->field_C.clear();
}

bool eligible_pack_category::is_pack_slot_locked(resource_pack_slot *a2) const
{
    auto it = std::find_if(this->field_1C.begin(), this->field_1C.end(), [a2](auto &v1) { return (v1 == a2); });

    return (it != this->field_1C.end());
}

void eligible_pack_category::lock_pack_slot(resource_pack_slot *s)
{
    if (s->is_pack_ready()) {
        this->my_resource_pack_streamer->unload(s);
    }

    assert(!this->is_pack_slot_locked(s));

    this->field_1C.push_back(s);
}

void eligible_pack_category::unlock_pack_slot(resource_pack_slot *s)
{
    assert(this->is_pack_slot_locked(s));

    assert(s->is_empty() || s->is_pack_unloading());

    {
        auto end = this->field_1C.end();
        auto it = this->field_1C.begin();
        while (it != end) {
            if ((*it) == s) {
                it = this->field_1C.erase(it);
            } else {
                ++it;
            }
        }
    }
}

int eligible_pack_category::find_empty_pack_slot() const
{
    assert(this->my_resource_pack_streamer != nullptr);

    assert(this->my_resource_pack_streamer->get_pack_slots() != nullptr);

    auto &pack_slots = *this->my_resource_pack_streamer->get_pack_slots();
    auto it = std::find_if(pack_slots.begin(), pack_slots.end(), [this](auto &slot) {
        return (slot->is_empty() && !this->is_pack_slot_locked(slot));
    });

    return (it != pack_slots.end() ? std::distance(pack_slots.begin(), it) : -1);
}

void eligible_pack_category::frame_advance(Float a2)
{
    if constexpr (0) {
        assert(this->my_resource_pack_streamer != nullptr);

        assert(this->my_resource_pack_streamer->get_pack_slots() != nullptr);

        auto &pack_slots = *this->my_resource_pack_streamer->get_pack_slots();
        int empty_slot_idx = this->find_empty_pack_slot();
        if (empty_slot_idx == -1 || this->field_C.empty()) {
            int slot_idx = this->find_lowest_priority_unloadable_loaded_pack_slot();
            if (slot_idx == -1) {
                slot_idx = this->find_lowest_priority_unloadable_loading_pack_slot();
                if (slot_idx != -1) {
                    assert(slot_idx >= 0 && slot_idx < pack_slots.size());

                    assert(this->my_resource_pack_streamer->can_cancel_load(slot_idx));

                    this->my_resource_pack_streamer->cancel_load(slot_idx);
                }
            } else {
                assert(slot_idx >= 0 && slot_idx < pack_slots.size());

                assert(pack_slots[slot_idx]->is_pack_ready());

                this->my_resource_pack_streamer->unload(slot_idx);
            }
        } else if (this->my_resource_pack_streamer->is_disk_idle()) {
            assert(empty_slot_idx >= 0 && empty_slot_idx < pack_slots.size());

            assert(pack_slots[empty_slot_idx]->is_empty());

            for (auto &ep : this->field_C) {
                assert(ep != nullptr);

                if (ep->get_resource_pack_slot() == nullptr && ep->is_loadable()) {
                    auto *v11 = this->field_8;
                    resource_pack_token token{int(ep), 0};
                    this->my_resource_pack_streamer->load(ep->field_20.to_string(), empty_slot_idx, v11, &token);
                    return;
                }
            }
        }
    } else {
        THISCALL(0x005513B0, this, a2);
    }
}

int eligible_pack_category::find_lowest_priority_unloadable_loaded_pack_slot()
{
    assert(this->my_resource_pack_streamer != nullptr);

    assert(this->my_resource_pack_streamer->get_pack_slots() != nullptr);

    auto &pack_slots = *this->my_resource_pack_streamer->get_pack_slots();
    int v12 = -1;

    for (uint32_t i{0}; i < pack_slots.size(); ++i) {
        auto &slot = pack_slots[i];
        if (slot->is_pack_ready() && !this->is_pack_slot_locked(slot)) {
            auto &pack_token = slot->get_pack_token();
            auto *ep = this->my_eligible_pack_streamer->find_eligible_pack_by_token(pack_token);
            assert(ep != nullptr);

            if (!ep->is_ideal() && ep->sub_5321C0()) {
                if (v12 == -1) {
                    v12 = i;
                } else {
                    auto &v3 = pack_slots[v12];
                    auto &v4 = v3->get_pack_token();
                    auto *lowest_so_far = this->my_eligible_pack_streamer->find_eligible_pack_by_token(v4);
                    assert(lowest_so_far != nullptr);

                    auto priority = ep->get_priority();
                    if (priority > lowest_so_far->get_priority()) {
                        v12 = i;
                    }
                }
            }
        }
    }

    return v12;
}

int eligible_pack_category::find_lowest_priority_unloadable_loading_pack_slot()
{
    assert(this->my_resource_pack_streamer != nullptr);

    assert(this->my_resource_pack_streamer->get_pack_slots() != nullptr);

    auto &pack_slots = *this->my_resource_pack_streamer->get_pack_slots();
    int v12 = -1;
    for (uint32_t i{0}; i < pack_slots.size(); ++i) {
        if (this->my_resource_pack_streamer->can_cancel_load(i)) {
            auto &slot = pack_slots[i];
            if (!this->is_pack_slot_locked(slot)) {
                auto &pack_token = slot->get_pack_token();
                auto *ep = this->my_eligible_pack_streamer->find_eligible_pack_by_token(pack_token);
                assert(ep != nullptr);

                if (!ep->is_ideal() && ep->sub_5321C0()) {
                    if (v12 == -1) {
                        v12 = i;
                    } else {
                        auto &v4 = pack_slots[v12];
                        auto &v5 = v4->get_pack_token();
                        auto *lowest_so_far = this->my_eligible_pack_streamer->find_eligible_pack_by_token(v5);
                        assert(lowest_so_far != nullptr);

                        auto priority = ep->get_priority();
                        if (priority > lowest_so_far->get_priority()) {
                            v12 = i;
                        }
                    }
                }
            }
        }
    }

    return v12;
}
