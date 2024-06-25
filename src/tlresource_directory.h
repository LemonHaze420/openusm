#pragma once

#include "tlresourcedirectory.h"

struct resource_directory;

template<typename, typename>
struct tlInstanceBankResourceDirectory;

template<typename T0, typename T1>
struct tlresource_directory : tlResourceDirectory<T0, T1> {
    resource_directory *field_4;

    tlresource_directory();

    void clear() {
        this->field_4 = nullptr;
    }

    void set_resource_directory(resource_directory *dir) {
        this->field_4 = dir;
    }

    T0 *Find(const T1 &);

    T0 *Find(uint32_t a2);

    void Add(T0 *);

    static tlInstanceBankResourceDirectory<T0, T1> *& system_dir;

    static T0 *& default_tlres;

    static int tlres_type;
};

extern void tlresource_directory_patch();
