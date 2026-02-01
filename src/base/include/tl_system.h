#pragma once

#include "fixedstring.h"

#include "tlresourcedirectory.h"
#include "variable.h"

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

        tlFixedString *GetString() {
            return T0::get_string(this->field_0);
        }
    };

    struct Impl {
        int field_0;
        int field_4;
        Node *field_8;
        int m_size;

        int sub_770B80();

        void Init();

        T0 *Find(const T1 &);

        void *Add(T0 *a1);

        Node *NewNodeOfLevel(int count) {
            return static_cast<Node *>(tlMemAlloc(4 * count + sizeof(Node), 8, 0x1000000u));
        }
    };

    using base_iterator_t = typename tlResourceDirectory<T0, T1>::Iterator;

    struct SkipListIterator : base_iterator_t {
        Impl *field_4;
        Node *field_8;

        void reset();

        bool operator()();

        T0 *operator*();

        void operator++();
    };

    Impl field_4;

    tlInstanceBankResourceDirectory();

    void *Add(T0 *a1);
    bool Del(T0 *a1);

    SkipListIterator *Enumerate();

    //virtual
    T0 * _Find(const T1 &);

    void finalize(bool a2) {
        this->~tlInstanceBankResourceDirectory();
        if (a2) {
            delete(this);
        }
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
