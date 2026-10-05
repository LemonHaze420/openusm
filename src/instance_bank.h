#pragma once


#include "mstring.h"
#include "set.hpp"

template <typename T>
struct instance_bank {
    struct entry {
        mString name;
        T *object;
    };

    _std::set<entry *> entries;
    _std::set<T *> instances;

    void purge();
};
