#include "info_node_desc_list.h"

#include "func_wrapper.h"
#include "utility.h"

namespace ai {

info_node_desc_list::info_node_desc_list() {}

void info_node_desc_list::add_entry(info_node_descriptor a2)
{
    assert(0);

    if constexpr (1) {
        auto *v3 = this->field_0.m_first;

        int v5;
        for (int i = 0;; ++i) {
            if (!v3) {
                v5 = 0;
                goto LABEL_10;
            }

            if (i >= this->field_0.m_last - v3) {
                break;
            }

            if (a2.field_0.source_hash_code == v3[i].field_0.source_hash_code && a2.field_4 == v3[i].field_4)
                return;
        }

        v5 = this->field_0.m_last - v3;
    LABEL_10:

        if (v3 != nullptr && v5 < this->field_0.m_end - v3) {
            auto *v6 = this->field_0.m_last;

            void (*sub_6DBDB0)(void *, int, void *) = CAST(sub_6DBDB0, 0x006DBDB0);
            sub_6DBDB0(v6, 1, &a2);

            this->field_0.m_last = v6 + 1;
        } else {
            void(__fastcall * sub_6DCB10)(void *, void *, void *, int, void *) = CAST(sub_6DCB10, 0x006DCB10);

            sub_6DCB10(this, nullptr, this->field_0.m_last, 1, &a2);
        }
    } else {
        THISCALL(0x006D6E20, this, a2);
    }
}

}  // namespace ai

void info_node_desc_list_patch()
{
    FUNC_ADDRESS(address, &ai::info_node_desc_list::add_entry);
    SET_JUMP(0x006D6E20, address);
}
