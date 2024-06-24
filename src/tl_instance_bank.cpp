#include "tl_instance_bank.h"

#include "common.h"
#include "func_wrapper.h"
#include "hashstring.h"
#include "trace.h"
#include "tl_system.h"
#include "utility.h"

tlInstanceBank::Node *tlInstanceBank::NewNodeOfLevel(int a1) {
    return static_cast<tlInstanceBank::Node *>(tlMemAlloc(4 * a1 + sizeof(Node), 8, 0));
}

VALIDATE_SIZE(tlInstanceBank, 0x14);
VALIDATE_SIZE(tlInstanceBank::Node, 0x2C);

void tlInstanceBank::Init()
{
    if (this->field_0 == nullptr) {
        this->field_0 = this->NewNodeOfLevel(0);

        for (int i = 0; i < 32; ++i) {
            bit_cast<uint8_t *>(&this->field_0->field_0)[i] = 0xFF;
        }

        this->field_C = rand();
        this->field_8 = 7;
        this->field_10 = 0;
        this->field_4 = this->NewNodeOfLevel(16);

        for (int i = 0; i < 16; ++i) {
            this->field_4->field_28[i] = this->field_0;
        }
    }
}

int tlInstanceBank::sub_74A020() {
    int v2 = 0;

    int v3;
    do {
        v3 = this->field_C & 3;
        if (v3 == 0) {
            ++v2;
        }

        this->field_C >>= 2;
        if (this->field_8-- == 1) {
            this->field_C = rand();
            this->field_8 = 7;
        }
    } while (v3 == 0);

    auto result = 15;
    if (v2 <= 15) {
        result = v2;
    }

    return result;
}

tlInstanceBank::tlInstanceBank() {
    this->field_0 = nullptr;
    this->field_4 = nullptr;
}

tlInstanceBank::~tlInstanceBank() {
    auto *v2 = this->field_4;
    if (v2 != nullptr) {
        for (auto *node = v2->field_28[0]; node != this->field_0;
             node = this->field_4->field_28[0]) {
            this->Delete(node->field_0);
        }
    }

    if (this->field_0 != nullptr) {
        tlMemFree(this->field_0);
    }

    if (this->field_4 != nullptr) {
        tlMemFree(this->field_4);
    }

    this->field_0 = nullptr;
    this->field_4 = nullptr;
}

int tlInstanceBank::Delete(const tlFixedString &a2) {
    auto v5 = this->field_4;
    auto v8 = this->field_10;
    auto i = v8;

    Node *v7[16];
    Node *a1 = nullptr;
    do {
        while (1) {
            a1 = v5->field_28[i];
            if (a1->field_0.compare(a2) >= 0) {
                break;
            }

            v5 = a1;
        }

        v7[i--] = v5;
    } while (i >= 0);

    if (a1->field_0 != a2) {
        return -1;
    }

    auto v3 = --a1->field_24;
    if (v3 <= 0) {
        for (i = 0; i <= v8; ++i) {
            auto *v6 = v7[i];
            if (v6->field_28[i] != a1) {
                break;
            }

            v6->field_28[i] = a1->field_28[i];
        }

        tlMemFree(a1);
        while (this->field_4->field_28[v8] == this->field_0 && v8 > 0) {
            --v8;
        }

        this->field_10 = v8;
    }

    return v3;
}

tlInstanceBank::Node *tlInstanceBank::Insert(const tlFixedString &a1, void *a3) {
    if constexpr (1) {
        auto *v7 = this->field_4;
        auto v9 = this->field_10;

        tlInstanceBank::Node *v8[16]{};

        Node *v5 = nullptr;
        do {
            while (1) {
                v5 = v7->field_28[v9];

                if (v5->field_0.compare(a1) >= 0) {
                    break;
                }

                v7 = v5;
            }

            v8[v9--] = v7;
        } while (v9 >= 0);

        Node *result;

        if (v5->field_0 == a1) {
            ++v5->field_24;
            result = v5;
        } else {
            auto v9 = this->sub_74A020();
            if (v9 > this->field_10) {
                v9 = ++this->field_10;
                v8[v9] = this->field_4;
            }

            auto *v6 = this->NewNodeOfLevel(v9);
            v6->field_0 = a1;
            v6->field_20 = a3;
            v6->field_24 = 1;
            do {
                auto *v4 = v8[v9];
                v6->field_28[v9] = v4->field_28[v9];
                v4->field_28[v9--] = v6;
            } while (v9 >= 0);

            result = nullptr;
        }

        return result;
    } else {
        return (tlInstanceBank::Node *) THISCALL(0x0074A070, this, &a1, a3);
    }
}

tlInstanceBank::Node *tlInstanceBank::Search(const tlFixedString &a1)
{
    TRACE("tlInstanceBank::Search", a1.to_string);

    if constexpr (1)
    {
        auto v5 = this->field_4;
        auto v6 = this->field_10;

        Node *v4 = nullptr;
        do {
            while (1) {
                auto *v4 = v5->field_28[v6];
                auto v3 = v4->field_0.compare(a1);
                if (v3 == 0) {
                    return v4;
                }

                if (v3 > 0) {
                    break;
                }

                v5 = v4;
            }
            --v6;
        } while (v6 >= 0);

        if (v4->field_0 == a1) {
            return v4;
        }

        return nullptr;
    } else {
        return (Node *) THISCALL(0x0074A310, this, &a1);
    }
}

tlInstanceBank::Node *tlInstanceBank::Search(const tlHashString &a2)
{
    TRACE("tlInstanceBank::Search", a2.c_str());

    auto *v7 = this->field_4;

    Node *a1;
    for (auto i = this->field_10; i >= 0; --i) {
        while (1) {
            a1 = v7->field_28[i];
            auto v2 = a1->field_0.m_hash;

            tlHashString v4{v2};
            auto v5 = v4.compare(a2);
            if (v5 == 0) {
                return a1;
            }

            if (v5 > 0) {
                break;
            }

            v7 = a1;
        }
    }

    Node *result = nullptr;
    if (a2 == a1->field_0.m_hash) {
        result = a1;
    }

    return result;
}

template<int I>
tlInstanceBank::Node * __fastcall tlInstanceBank_Search(tlInstanceBank *self, void *, const tlHashString &a2)
{
    TRACE("tlInstanceBank_Search");

    auto *v7 = self->field_4;

    tlInstanceBank::Node *a1;
    for (auto i = self->field_10; i >= 0; --i) {
        while (1) {
            a1 = v7->field_28[i];
            auto v2 = a1->field_0.m_hash;

            tlHashString v4{v2};
            auto v5 = v4.compare(a2);
            if (v5 == 0) {
                return a1;
            }

            if (v5 > 0) {
                break;
            }

            v7 = a1;
        }
    }

    tlInstanceBank::Node *result = nullptr;
    if (a2 == a1->field_0.m_hash) {
        result = a1;
    }

    return result;
}

void tlInstanceBank_patch()
{
    tlInstanceBank::Node * (tlInstanceBank::*func)(const tlHashString &) = &tlInstanceBank::Search;

    FUNC_ADDRESS(address, func);
    //SET_JUMP(0x0074A310, address);

    if constexpr(1)
    {
        REDIRECT(0x004988E1, &tlInstanceBank_Search<0>);
        REDIRECT(0x00498BD1, &tlInstanceBank_Search<1>);
        REDIRECT(0x005EFA65, &tlInstanceBank_Search<2>);
        REDIRECT(0x005F26B2, &tlInstanceBank_Search<3>);
        REDIRECT(0x0076F68F, &tlInstanceBank_Search<4>);
        REDIRECT(0x0076FA57, &tlInstanceBank_Search<5>);
        REDIRECT(0x0078D5BE, &tlInstanceBank_Search<6>);
        REDIRECT(0x0078D96F, &tlInstanceBank_Search<7>);
        REDIRECT(0x0078DC8E, &tlInstanceBank_Search<8>);
        REDIRECT(0x0078DCE6, &tlInstanceBank_Search<9>);
        REDIRECT(0x0078DD84, &tlInstanceBank_Search<10>);
        REDIRECT(0x00793723, &tlInstanceBank_Search<11>);
        REDIRECT(0x0079375F, &tlInstanceBank_Search<12>);
    }
}
