#include "param_list.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "slab_allocator.h"
#include "vector3d.h"

namespace als {

VALIDATE_SIZE(param_node, 0x10);

param_node::param_node(als::param a2)
{
    this->field_0 = a2;
    this->field_8 = this;
    this->field_C = this;
}

param_list::param_list()
{
    this->field_0 = nullptr;
}

void param_list::add_param(uint32_t a1, const vector3d &a2)
{
    als::param v5;

    for (uint32_t i = 0; i < 3u; ++i) {
        v5.field_4 = a2[static_cast<int>(i)];
        v5.field_0 = static_cast<int>(i + a1);
        this->add_param(v5);
    }
}

void param_list::insert_node(param_node *a2)
{
    if (this->field_0 != nullptr) {
        auto *v4 = this->field_0->field_C;
        v4->field_8 = a2;
        a2->field_C = v4;
        a2->field_8 = this->field_0;
        this->field_0->field_C = a2;
    } else {
        this->field_0 = a2;
        a2->field_C = a2;
        this->field_0->field_8 = this->field_0;
    }
}

void param_list::add_param(param a2)
{
    if constexpr (1) {
        auto *mem = mem_alloc(sizeof(param_node));

        auto *v3 = new (mem) param_node{a2};

        this->insert_node(v3);
    } else {
        THISCALL(0x00499620, this, a2);
    }
}

void param_list::clear()
{
    if constexpr (1) {
        while (this->field_0 != nullptr) {
            auto *v2 = this->field_0;
            auto *v3 = this->field_0->field_8;
            if (this->field_0 == v3) {
                this->field_0 = nullptr;
            } else {
                this->field_0 = v3;
                auto *v4 = v2->field_C;
                auto *v5 = v2->field_8;
                v4->field_8 = v5;
                v5->field_C = v4;
            }

            if (v2 != nullptr) {
                mem_dealloc(v2, sizeof(*v2));
            }
        }
    } else {
        THISCALL(0x0049E270, this);
    }
}

bool param_list::is_empty() const
{
    return (this->field_0 != nullptr);
}

void param_list::concat_list(als::param_list &a2)
{
    if constexpr (1) {
        auto &v2 = a2.field_0;
        if (!a2.is_empty()) {
            auto &v3 = this->field_0;
            if (!this->is_empty()) {
                auto &v4 = v3->field_C;
                v3->field_C = v2->field_C;
                a2.field_0->field_C->field_8 = this->field_0;
                v4->field_8 = a2.field_0;
                a2.field_0->field_C = v4;
            } else {
                this->field_0 = v2;
            }

            a2.field_0 = nullptr;
            a2.clear();
        }
    } else {
        THISCALL(0x0049FE70, this, &a2);
    }
}

void param_list::cull_duplicates_keep_last()
{
    if constexpr (1) {
        auto &v1 = this->field_0;
        if (this->field_0 != nullptr) {
            als::param_node *a1[130];

            auto &v2 = v1;
            memset(&a1[0], 0, sizeof(a1));
            auto &v3 = v1;
            als::param_node *v4;
            als::param_node **v5;
            als::param_node *v8;
            als::param_node *v9;

            while (1) {
                v4 = a1[v3->field_0.field_0];
                v5 = &a1[v3->field_0.field_0];
                if (v4 != nullptr) {
                    break;
                }

            LABEL_14:
                *v5 = v3;
                v3 = v3->field_8;
                if (v3 == v2) {
                    return;
                }
            }

            if (v4 == v2) {
                v2 = v4->field_8;
            }

            auto *v6 = this->field_0;
            if (v4 == this->field_0) {
                auto *v7 = v6->field_8;
                if (v6 == v7) {
                    this->field_0 = nullptr;
                } else {
                    this->field_0 = v7;
                }
            } else {
                v8 = v4->field_C;
                v9 = v4->field_8;
                v8->field_8 = v9;
                v9->field_C = v8;
            }

            mem_dealloc(v4, sizeof(*v4));

            goto LABEL_14;
        }
    } else {
        THISCALL(0x0049E1D0, this);
    }
}

}  // namespace als
