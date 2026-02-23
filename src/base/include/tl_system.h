#pragma once

#include "fixedstring.h"

#include "tlresourcedirectory.h"
#include "variable.h"
#include "variables.h"

struct tlHashString;
struct resource_directory;

//0x0074A5C0
extern void *tlMemAlloc(uint32_t Size, uint32_t Alignment, uint32_t Flags);

//0x0074A600
extern void tlMemFree(void *Ptr);

template<typename T0, typename T1>
struct tlInstanceBankResourceDirectory : tlResourceDirectory<T0, T1> {
    struct Node {
        T0 *field_0;
        Node *field_4[1];

        T1 *GetString() {
            return T0::get_string(this->field_0);
        }
    };

    struct Impl {
        int field_0;
        int field_4;
        Node *field_8;
        int m_size;

        Impl() {
            this->field_0 = rand();
            this->field_4 = 7;
            this->field_8 = nullptr;
        }

        int sub_770B80()
        {
            auto v2 = 0;

            int v3;
            do {
                v3 = this->field_0 & 3;
                if (v3 == 0) {
                    ++v2;
                }

                this->field_0 >>= 2;
                if (this->field_4-- == 1) {
                    this->field_0 = rand();
                    this->field_4 = 7;
                }

            } while (v3 == 0);

            auto result = 15;
            if (v2 <= 15) {
                result = v2;
            }

            return result;
        }

        void Init()
        {
            this->m_size = 0;
            this->field_8 = this->NewNodeOfLevel(15);
            for (int i = 0; i < 16; ++i) {
                this->field_8->field_4[i] = nullptr;
            }
        }

        T0 * Find(const T1 &a2)
        {
            if ( this->field_8 == nullptr ) {
                return nullptr;
            }

            auto *v7 = this->field_8;
            int size = this->m_size;

            Node *v6;
            do
            {
                while ( 1 )
                {
                    v6 = v7->field_4[size];
                    if ( v6 == nullptr ) {
                        break;
                    }

                    auto *v3 = v6->GetString();
                    auto v5 = v3->compare(a2);
                    if ( v5 == 0 ) {
                        return v6->field_0;
                    }

                    if ( v5 > 0 ) {
                        break;
                    }

                    v7 = v6;
                }

                --size;
            }
            while ( size >= 0 );

            if ( v6 != nullptr ) {
                auto v4 = *v6->GetString();
                if (v4 == a2) {
                    return v6->field_0;
                }
            }

            return nullptr;
        }

        void * Add(T0 *a1)
        {
            if (this->field_8 == nullptr) {
                this->Init();
            }

            Node *v11[16]{};

            auto *a1a = T0::get_string(a1);
            auto v9 = this->field_8;
            auto v12 = this->m_size;

            Node *v7 = nullptr;
            for (; v12 >= 0; --v12) {
                while (1) {
                    v7 = v9->field_4[v12];
                    if (v7 == nullptr) {
                        break;
                    }

                    auto *v2 = T0::get_string(v7->field_0);
                    if (v2->compare(*a1a) >= 0) {
                        break;
                    }

                    v9 = v7;
                }

                v11[v12] = v9;
            }

            if (v7 != nullptr) {
                auto *v3 = T0::get_string(v7->field_0);
                if (*v3 == *a1a) {
                    return v7->field_0;
                }
            }

            v12 = this->sub_770B80();
            if (v12 > this->m_size) {
                v12 = ++this->m_size;
                v11[v12] = this->field_8;
            }

            Node *v8 = this->NewNodeOfLevel(v12);
            v8->field_0 = a1;
            do {
                auto *v10 = v11[v12];
                v8->field_4[v12] = v10->field_4[v12];
                v10->field_4[v12--] = v8;
            } while (v12 >= 0);
            return nullptr;
        }

        bool Del(T0 *a2)
        {
            if ( this->field_8 == nullptr ) {
                return false;
            }

            auto string = T0::get_string(a2);
            auto v6 = this->field_8;
            auto size = this->m_size;
            auto i = size;

            Node *a1;
            Node *v8[16] {};

            do
            {
                while ( 1 )
                {
                    a1 = v6->field_4[i];
                    if ( a1 == nullptr ) {
                        break;
                    }

                    auto v4 = string;
                    auto v3 = T0::get_string(a1->field_0);
                    if ( v3->compare(*v4) >= 0 ) {
                        break;
                    }

                    v6 = a1;
                }

                v8[i--] = v6;
            }
            while ( i >= 0 );

            if ( !a1 || (T0 *)a1->field_0 != a2 ) {
                return false;
            }

            for ( i = 0; i <= size; ++i )
            {
                auto *v7 = v8[i];
                if ( v7->field_4[i] != a1 ) {
                    break;
                }

                v7->field_4[i] = a1->field_4[i];
            }

            tlMemFree(a1);
            while ( !this->field_8->field_4[size] && size > 0 ) {
                --size;
            }

            this->m_size = size;
            return 1;
        }

        Node *NewNodeOfLevel(int count) {
            return static_cast<Node *>(tlMemAlloc(4 * count + sizeof(Node), 8, 0x1000000u));
        }
    };

    using base_type = tlResourceDirectory<T0, T1>;
    using base_iterator_t = typename base_type::Iterator;

    struct SkipListIterator : base_iterator_t {
        Impl *field_4;
        Node *field_8;

        SkipListIterator(Impl *a2) : field_4(a2) {
            this->m_vtbl = 0x008BDDC0;
        }

        //virtual
        void finalize(bool a2) {
            this->~SkipListIterator();
            if (a2) {
                tlMemFree(this);
            }
        }

        //virtual
        void reset();

        //virtual
        bool operator()();

        //virtual
        T0 *operator*();

        void operator++();
    };

    Impl field_4;

    tlInstanceBankResourceDirectory()
    {
        T0 * (tlResourceDirectory<T0, T1>::*Find1)(unsigned int) = &tlResourceDirectory<T0, T1>::_Find;
        static void * g_vtbl[] = {
            func_address(&finalize),
            func_address(&base_type::DirectoryName),
            func_address(Find1),
            func_address(&_Find),
            func_address(&_Add),
            func_address(&_Del),
            func_address(&_Enumerate),
            func_address(&base_type::_ReleaseAll),
            func_address(&base_type::Load),
            func_address(&base_type::Load),
            func_address(&base_type::_Release)
        };

        this->m_vtbl = CAST(this->m_vtbl, &g_vtbl);
    }

    //virtual
    void finalize(bool a2) {
        this->~tlInstanceBankResourceDirectory();
        if (a2) {
            delete(this);
        }
    }

    //virtual
    void * _Add(T0 *a1) {
        return this->field_4.Add(a1);
    }

    //virtual
    bool _Del(T0 *a1) {
        return this->field_4.Del(a1);
    }

    //virtual
    SkipListIterator * _Enumerate() {
        auto *mem = tlMemAlloc(
                sizeof(SkipListIterator),
                8u,
                0x2000000u);

        auto *result = new (mem) SkipListIterator {&this->field_4};
        return result;
    }

    //virtual
    T0 * _Find(const T1 &a1) {
        return this->field_4.Find(a1);
    }

    void * operator new(std::size_t sz) {
        return tlMemAlloc(sz, 8u, 0x1000000u);
    }

    void operator delete(void *ptr) {
        tlMemFree(ptr);
    }
};

struct tlFileBuf {
    char *Buf;
    uint32_t Size;
    uint32_t UserData;
};

struct tlSystemCallbacks {
    bool (*ReadFile)(const char *, tlFileBuf *, unsigned int, unsigned int);
    void (*ReleaseFile)(tlFileBuf *);
    int field_8;
    int field_C;
    void *(*MemAlloc)(unsigned int, unsigned int, unsigned int);
    void (*MemFree)(void *);
};

struct tlInitList {
    std::intptr_t m_vtbl;
    tlInitList *field_4;

    tlInitList() {
        this->field_4 = head;
        head = this;
    }

    /* virtual */ void Register(); // = 0;

#if !STANDALONE_SYSTEM
    static inline auto & head = var<tlInitList *>(0x00970D4C);
#else
    static inline tlInitList * head = nullptr;
#endif
};


struct tlInitListFunction : tlInitList {
    void (*field_8)();

    tlInitListFunction(void (*cb)());

    //virtual
    void _Register(); // override;
};

struct tlInstanceBank;

extern tlInstanceBank & nglShaderBank;

extern int & tlScratchPadRefCount;

extern tlSystemCallbacks & tlCurSystemCallbacks;

//0x00749FD0
extern void tlInitListInit();

extern void tlStackRangeInit();

extern bool sub_101BF70(int a1);

//0x0074A520
void tlSetSystemCallbacks(const tlSystemCallbacks &a1);

//0x0074A6C0
void tlReleaseFile(tlFileBuf *File);

//0x0074A710
bool tlReadFile(const char *FileName, tlFileBuf *File, unsigned int Align, unsigned int Flags);

bool tlIsPow2(int a1);

//0x0050EAD0
void set_tl_system_directories();

extern void tl_patch();
