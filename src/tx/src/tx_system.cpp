#include "tx_system.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "variable.h"
#include <variables.h>
#include "trace.h"

//#include "nfl_system.h"

#include <cassert>
#include <limits>
#include <time.h>

VALIDATE_SIZE(txSlotPool, 0x28);

txSlotPool::Slot *txSlotPoolGetSlot(txSlotPool *a1, int id)
{
    if (id < 0) {
        return nullptr;
    }

    int v3 = a1->max_value & id;
    if (v3 >= a1->m_count) {
        return nullptr;
    }

    auto *slot = bit_cast<txSlotPool::Slot *>(bit_cast<char *>(a1->field_0) + v3 * a1->m_size);
    if (id == slot->m_id) {
        return slot;
    }

    return nullptr;
}

int txSlotNew(txSlotPool *a1)
{
    auto *slot = a1->field_4.field_0;
    if (slot == &a1->field_4) {
        return -1;
    }

    [](auto *a1) {
        a1->field_4->field_0 = a1->field_0;
        a1->field_0->field_4 = a1->field_4;
    }(slot);

    [](auto *a1, auto *a2) -> void {
        a2->field_0 = a1;
        a2->field_4 = a1->field_4;
        a1->field_4->field_0 = a2;
        a1->field_4 = a2;
    }(&a1->field_10, slot);

    auto result = [](txSlotPool *a1, auto *slot) -> int {
        slot->m_id += a1->max_value + 1;
        slot->m_id &= 0x7FFFFFFFu;
        if ((slot->m_id & ~(a1->max_value | 0x80000000)) == 0) {
            slot->m_id += a1->max_value + 1;
        }

        return slot->m_id;
    }(a1, slot);

    return result;
}

bool txSlotPoolInit(txSlotPool *a1, void *entries, uint32_t count, uint32_t size)
{
    //sp_log("txSlotPoolInit: Start");

    //sp_log("txSlotPoolInit: count of slots = %u with size = %u", count, size);

    if constexpr (1) {
        if ((count > 0) && (count <= 0xFFFFF) && (size >= 12)) {
            int num_bits = 0;

            for (auto v5 = count; v5 != 0; v5 >>= 1) {
                ++num_bits;
            }

            if constexpr (1) {
                auto res = (num_bits == (std::numeric_limits<decltype(count)>::digits - __builtin_clz(count)));
                assert(res);
            }

            a1->field_0 = CAST(a1->field_0, entries);
            a1->m_size = size;
            a1->m_count = count;
            a1->max_value = (1 << static_cast<uint8_t>(num_bits)) - 1u;

            auto func = [](auto *a1) {
                a1->field_0 = a1;
                a1->field_4 = a1;
                a1->m_id = -1;
            };

            func(&a1->field_4);

            func(&a1->field_10);

            for (uint32_t i{0}; i < count; ++i) {
                txSlotPool::Slot *slot = CAST(slot, bit_cast<char *>(a1->field_0) + i * a1->m_size);

                slot->m_id = i | 0x80000000;

                [](auto *a1, auto *a2) -> void {
                    a2->field_0 = a1;
                    a2->field_4 = a1->field_4;
                    a1->field_4->field_0 = a2;
                    a1->field_4 = a2;
                }(&a1->field_4, slot);
            }

            return true;
        }

        return false;
    } else {
        return (bool)CDECL_CALL(0x0079F5E0, a1, entries, count, size);
    }
}

void txSlotFree(txSlotPool *a1, int a2)
{
    TRACE("txSlotFree");

    auto *slot = txSlotPoolGetSlot(a1, a2);
    if (slot != nullptr) {
        slot->m_id |= 0x80000000;

        slot->field_4->field_0 = slot->field_0;
        slot->field_0->field_4 = slot->field_4;

        [](auto *a1, auto *a2) -> void {
            a2->field_0 = a1;
            a2->field_4 = a1->field_4;
            a1->field_4->field_0 = a2;
            a1->field_4 = a2;
        }(&a1->field_4, slot);
    }
}

int txSlotNext(txSlotPool *a1, int a2)
{
    int result = -1;
    auto *slot = txSlotPoolGetSlot(a1, a2);
    if (slot != nullptr) {
        result = slot->field_0->m_id;
    }

    return result;
}

int txSlotFirst(txSlotPool *a1)
{
    return a1->field_10.field_0->m_id;
}

int txSlotIndex(txSlotPool *a1, int a2)
{
    TRACE("txSlotIndex");

    int result = -1;
    auto *slot = txSlotPoolGetSlot(a1, a2);
    if (slot != nullptr) {
        result = a1->max_value & slot->m_id;
    }

    return result;
}

#if !STANDALONE_SYSTEM
static int &dword_984538 = var<int>(0x00984538);
#else

static int &dword_984538 = []() -> auto & {
    static int g_dword_984538{};
    return g_dword_984538;
}();
#endif

int txTime()
{
    auto result = static_cast<uint64_t>(1000ll * clock()) / 1000ull;

    dword_984538 = result;
    return static_cast<int>(result);
}
